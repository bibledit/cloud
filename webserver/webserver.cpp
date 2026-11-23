/*
Copyright (©) 2003-2026 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include <bootstrap/bootstrap.h>
#include <config/globals.h>
#include <database/logs.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/http.h>
#include <webserver/request.h>
#include <webserver/webserver.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#ifdef HAVE_WINDOWS
#include <io.h>
#endif


static void enqueue_task(std::function<void()> task);

// Gets a line from a socket.
// The line may end with a newline, a carriage return, or a CR-LF combination.
// It terminates the string read with a null character.
// If no newline indicator is found before the end of the buffer the string is terminated with a null.
// If any of the above three line terminators is read,
// the last character of the string will be a linefeed
// and the string will be terminated with a null character.
// Parameters: the socket file descriptor
//             the buffer to save the data to
//             the size of the buffer
// Returns: the number of bytes stored (excluding null).
static int get_line(const int sock, char* buf, const int size)
{
    int i{0};
    char character{'\0'};
    int n{0};
    while ((i < size - 1) && (character != '\n'))
    {
        n = static_cast<int>(recv(sock, &character, 1, 0));
        if (n > 0)
        {
            if (character == '\r')
            {
                n = static_cast<int>(recv(sock, &character, 1, MSG_PEEK));
                if (n > 0 and character == '\n')
                {
                    recv(sock, &character, 1, 0);
                }
                else
                {
                    character = '\n';
                }
            }
            buf[i] = character;
            i++;
        }
        else
        {
            character = '\n';
        }
    }
    buf[i] = '\0';
    return i;
}


// This converts an IPv4 address in IPv6 notation to a pure IPv4 notation.
static void convert_ipv6_notation_to_pure_ipv4_notation (std::string& address)
{
    // The client's remote IPv6 address in hexadecimal digits separated by colons.
    // IPv4 addresses are mapped to IPv6 addresses.
    // Example IPv4 address: ::ffff:127.0.0.1
    // Example IPv6 address: ::1
    // Clean the IP address up so it's a clear IPv4 or IPv6 notation.
    if (size_t pos = address.find('.'); pos != std::string::npos)
    {
        pos = address.find_last_of(':');
        address.erase(0, ++pos);
    }
}


// Processes a single request from a web client.
// ReSharper disable once CppPassValueParameterByConstReference
static void webserver_process_request (const int conn_fd, const std::string client_address)
{
    // The environment for this request.
    // A reference to this object gets passed around from function to function during the entire request.
    // This provides thread-safety to the request.
    Webserver_Request request{};

    // This is the plain http server.
    request.secure = false;

    // Store remote client address in the request.
    request.remote_address = client_address;

    try
    {
        if (config_globals_webserver_running)
        {
            // Connection health flag.
            bool connection_healthy{true};

            // Read the client's request.
            // With the HTTP protocol it is not possible to read the request till EOF,
            // because EOF does never come, because the browser keeps the connection open
            // for receiving the response.
            // The HTTP protocol works per line.
            // Read one line of data from the client.
            // An empty line marks the end of the headers.
            constexpr int buffer_size{2048};
            int bytes_read{};
            bool header_parsed{true};
            char buffer[buffer_size] = {};
            do
            {
                bytes_read = get_line(conn_fd, buffer, buffer_size);
                if (bytes_read <= 0) connection_healthy = false;
                // Parse the browser's request's headers.
                header_parsed = http_parse_header(buffer, request);
            }
            while (header_parsed);

            if (connection_healthy)
            {
                // In the case of a POST request, more data follows: The POST request itself.
                // The length of that data is indicated in the header's Content-Length line.
                // Read that data, and parse it.
                std::string post_data{};
                if (request.is_post)
                {
                    bool done_reading{false};
                    int total_bytes_read{0};
                    do
                    {
                        bytes_read = static_cast<int>(recv(conn_fd, buffer, buffer_size, 0));
                        for (int i = 0; i < bytes_read; i++)
                        {
                            post_data += buffer[i];
                        }
                        // EOF indicates reading is ready.
                        // An error also indicates that reading is ready.
                        if (bytes_read <= 0) done_reading = true;
                        if (bytes_read < 0) connection_healthy = false;
                        // "Content-Length" bytes read: Done.
                        total_bytes_read += bytes_read;
                        if (total_bytes_read >= request.content_length) done_reading = true;
                    }
                    while (!done_reading);
                    if (total_bytes_read < request.content_length) connection_healthy = false;
                }

                if (connection_healthy)
                {
                    http_parse_post(post_data, request);

                    // Assemble response.
                    bootstrap_index(request);
                    http_assemble_response(request);

                    // Send response to browser.
                    const char* output = request.reply.c_str();
                    // The C function strlen () fails on null characters in the reply, so use string::size() instead.
                    const size_t length = request.reply.size();
                    send(conn_fd, output, length, 0);

                    // When streaming a file, copy the file's contents straight from disk to the network file descriptor.
                    // Do not load the entire file into memory.
                    // This enables large file transfers on low-memory devices.
                    // Also handle cases that the requested file does not exist.
                    // So the number of bytes read should be larger than zero, not unequal to zero.
                    // In the case of != 0, it falls in an endless loop, because -1 indicates failure.
                    if (!request.stream_file.empty())
                    {
                        const int file_fd =
#ifdef HAVE_WINDOWS
                            _open
#else
                            open
#endif
                            (request.stream_file.c_str(), O_RDONLY);
                        unsigned char stream_buffer[1024];
                        int byte_count{};
                        do
                        {
                            byte_count = static_cast<int>(
#ifdef HAVE_WINDOWS
                                _read
#else
                                read
#endif
                                (file_fd, stream_buffer, 1024));
                            if (byte_count > 0)
                            {
                                // ReSharper disable once CppRedundantCastExpression
                                send (conn_fd, reinterpret_cast<const char *> (stream_buffer), static_cast<size_t>(byte_count), 0);
                            }
                        }
                        while (byte_count > 0);
#ifdef HAVE_WINDOWS
                        _close
#else
                        close
#endif
                            (file_fd);
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        std::string message("Internal error: ");
        message.append(e.what());
        database::logs::log(message);
    }
    catch (const std::exception* e) // NOLINT(*-throw-by-value-catch-by-reference)
    {
        std::string message("Internal error: ");
        message.append(e->what());
        database::logs::log(message);
    }
    catch (...)
    {
        database::logs::log("A general internal error occurred");
    }

    // Done: Close.
#ifdef HAVE_WINDOWS
    shutdown(conn_fd, SD_BOTH);
    closesocket(conn_fd);
#else
    shutdown(conn_fd, SHUT_RDWR);
    close(conn_fd);
#endif
}


#ifndef HAVE_WINDOWS
// This http server uses BSD sockets.
void http_server()
{
    bool listener_healthy{true};

    // Create a listening socket.
    // This represents an endpoint.
    // This prepares to accept incoming connections on.
#ifndef HAVE_CLOUD
    // A client listens on IPv4, see also below.
    const int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
#endif
#ifdef HAVE_CLOUD
    // The Cloud listens on address family AF_INET6 for both IPv4 and IPv6.
    const int listen_fd = socket(AF_INET6, SOCK_STREAM, 0);
#endif
    if (listen_fd < 0)
    {
        std::string error = "Error opening socket: ";
        error.append(strerror(errno));
        std::cerr << error << std::endl;
        database::logs::log(error);
        listener_healthy = false;
    }

    // Eliminate "Address already in use" error from bind.
    // The function is used to allow the local address to  be reused
    // when the server is restarted before the required wait time expires.
    int opt_val{1};
    int result = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(int));
    if (result != 0)
    {
        std::string error = "Error setting socket option: ";
        error.append(strerror(errno));
        std::cerr << error << std::endl;
        database::logs::log(error);
    }

    // The listening socket will be an endpoint for all requests to a port on this host.
#ifndef HAVE_CLOUD
    // When configured as a client, it listens on the IPv4 loopback device.
    // It has been seen on Ubuntu 16.04 that a Bibledit Client would not listen on a IPv6 loopback device.
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(static_cast<decltype(server_addr.sin_port)>(filter::string::convert_to_int (config::logic::http_network_port())));
#endif
#ifdef HAVE_CLOUD
    // When configured as a server it listens on any IPv6 address.
    sockaddr_in6 server_addr = {};
    server_addr.sin6_flowinfo = 0;
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(
        static_cast<uint16_t>(filter::string::convert_to_int (config::logic::http_network_port ())));
#endif
    result = bind(listen_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof (server_addr));
    if (result != 0)
    {
        std::string error = "Error binding server to socket: ";
        error.append(strerror(errno));
        std::cerr << error << std::endl;
        database::logs::log(error);
        listener_healthy = false;
    }

    // Make it a listening socket ready to queue and accept many connection requests
    // before the system starts rejecting the incoming requests.
    result = listen(listen_fd, 100);
    if (result != 0)
    {
        std::string error = "Error listening on socket: ";
        error.append(strerror(errno));
        std::cerr << error << std::endl;
        database::logs::log(error);
        listener_healthy = false;
    }

    // Keep waiting for, accepting, and processing connections.
    while (listener_healthy and config_globals_webserver_running)
    {
        // Socket and file descriptor for the client connection.
        sockaddr_in6 client_addr6{};
        socklen_t client_len = sizeof (client_addr6);
        if (const int conn_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr6), &client_len);
            conn_fd > 0)
        {
            // Socket receive timeout, plain http.
            timeval tv{};
            tv.tv_sec = 60;
            tv.tv_usec = 0;
            setsockopt(conn_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            // The client's remote IPv6 address in hexadecimal digits separated by colons.
            // IPv4 addresses are mapped to IPv6 addresses.
            // Example IPv4 address: ::ffff:127.0.0.1
            // Example IPv6 address: ::1
            // Clean the IP address up so it's a clear IPv4 or IPv6 notation.
            char remote_address[256];
            inet_ntop(AF_INET6, &client_addr6.sin6_addr, remote_address, sizeof (remote_address));
            std::string client_address = remote_address;
            convert_ipv6_notation_to_pure_ipv4_notation(client_address);

            // Handle this request in a thread, enabling parallel requests.
            enqueue_task([conn_fd, client_address] { webserver_process_request(conn_fd, client_address); });
        }
        else
        {
            std::string error = "Error accepting connection on socket: ";
            error.append(strerror(errno));
            std::cerr << error << std::endl;
            database::logs::log(error);
        }
    }

    // Close listening socket, freeing it for any next server process.
    close(listen_fd);
}
#endif


#ifdef HAVE_WINDOWS
bool server_accepting_flag {false};
std::mutex server_accepting_mutex;


void http_server_acceptor_processor(SOCKET listen_socket)
{
    // Accept a client socket.
    server_accepting_mutex.lock();
    server_accepting_flag = true;
    server_accepting_mutex.unlock();
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof (clientaddr);
    // The SOCKET in Windows will be closed when it goes out of scope.
    // This is different from a Unix file descriptor.
    // Therefore, there's a difference in web server architecture between them.
    SOCKET client_socket = accept(listen_socket, (struct sockaddr*)&clientaddr, &clientlen);
    server_accepting_mutex.lock();
    server_accepting_flag = false;
    server_accepting_mutex.unlock();
    if (client_socket != INVALID_SOCKET)
    {
        // Set timeout on receive, in milliseconds.
        const char* tv = "600000";
        setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, tv, sizeof (tv));

        // The client's remote IPv4 address in dotted notation.
        std::string clientaddress;
        char remote_address[256];
        inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, remote_address, sizeof (remote_address));
        clientaddress = remote_address;

        webserver_process_request(client_socket, clientaddress);
    }

    // Shutdown and close the connection.
    shutdown(client_socket, SD_BOTH);
    closesocket(client_socket);
}
#endif


#ifdef HAVE_WINDOWS
// This http server uses Windows sockets.
void http_server()
{
    int result;
    bool listener_healthy = true;

    // Initialize the interface to Windows Sockets.
    WSADATA wsa_data;
    result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0)
    {
        std::string error = "Could not initialize Windows Sockets with error " + std::to_string(result);
        std::cerr << error << std::endl;
        database::logs::log(error);
        listener_healthy = false;
    }
    // Check for the correct requested Windows Sockets interface version.
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        std::string error = "Incorrect Windows Sockets version";
        std::cerr << error << std::endl;
        database::logs::log(error);
        listener_healthy = false;
    }

    // Create a socket for listening for incoming connections.
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET)
    {
        std::string error = "Socket failed with error " + std::to_string(WSAGetLastError());
        std::cerr << error << std::endl;
        database::logs::log(error);
        listener_healthy = false;
    }

    // The listening socket will be an endpoint for all requests to a port on this host.
    // As Windows is a client, it listens on the loopback device only, for improved security.
    typedef struct sockaddr SA;
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serveraddr.sin_port = htons(filter::string::convert_to_int(config::logic::http_network_port()));
    result = ::bind(listen_socket, (SA*)&serveraddr, sizeof(serveraddr));
    if (result == SOCKET_ERROR)
    {
        std::string error = "Error binding server to socket";
        std::cerr << error << std::endl;
        database::logs::log(error);
        listener_healthy = false;
    }

    // Listen for multiple connections.
    result = listen(listen_socket, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        std::string error = "Listen failed with error " + std::to_string(WSAGetLastError());
        std::cerr << error << std::endl;
        database::logs::log(error);
        listener_healthy = false;
    }

    // Keep waiting for, accepting, and processing connections.
    config_globals_webserver_running = true;
    while (listener_healthy && config_globals_webserver_running)
    {
        // Poll the system whether to wait for a client.
        bool start_acceptor = false;
        server_accepting_mutex.lock();
        if (!server_accepting_flag) start_acceptor = true;
        server_accepting_mutex.unlock();
        if (start_acceptor)
        {
            // Handle this request via the thread pool, enabling parallel requests.
            enqueue_task(std::bind(http_server_acceptor_processor, listen_socket));
        }
        // Wait shortly before next poll iteration.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // No longer need server socket
    closesocket(listen_socket);

    // Clean up the interface to Windows Sockets.
    WSACleanup();
}
#endif


static void webserver_log_ssl_errors(const std::string& prefix, const std::string& remote_address = {})
{
    while (const unsigned long error = ERR_get_error())
    {
        char text[256];
        ERR_error_string_n(error, text, sizeof(text));
        std::string message = prefix + ": " + text;
        if (!remote_address.empty()) message.append(" (IP address " + remote_address + ")");
        database::logs::log(message);
    }
}


// Adds the certificate authorities chain to the SSL context.
static bool webserver_add_certificate_chain(SSL_CTX* context, const std::string& path)
{
    FILE* file = fopen(path.c_str(), "r");
    if (!file) return false;
    bool result{true};
    while (X509* certificate = PEM_read_X509(file, nullptr, nullptr, nullptr))
    {
        if (SSL_CTX_add_extra_chain_cert(context, certificate) != 1)
        {
            X509_free(certificate);
            result = false;
            break;
        }
        // The context owns the certificate after it has been added successfully.
    }
    // Reaching the end of the file is not an error.
    ERR_clear_error();
    fclose(file);
    return result;
}


// Reads one byte from an SSL connection.
// Returns 1 on success, 0 on EOF, and -1 on error.
static int webserver_ssl_read_byte(SSL* ssl, char& character)
{
    int result{0};
    do
    {
        result = SSL_read(ssl, &character, 1);
    } while (result < 0 && SSL_get_error(ssl, result) == SSL_ERROR_WANT_READ);
    if (result == 1) return 1;
    if (result == 0) return 0;
    webserver_log_ssl_errors("Error reading from secure connection");
    return -1;
}


// Writes all bytes to an SSL connection.
static bool webserver_ssl_write_all(SSL* ssl, const char* data, size_t length)
{
    while (length > 0)
    {
        const int written = SSL_write(ssl, data, static_cast<int>(length));
        if (written > 0)
        {
            data += written;
            length -= static_cast<size_t>(written);
            continue;
        }
        const int error = SSL_get_error(ssl, written);
        if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) continue;
        webserver_log_ssl_errors("Error writing to secure connection");
        return false;
    }
    return true;
}


// Processes a single request from a web client.
[[maybe_unused]] static void secure_webserver_process_request(SSL_CTX* context, int client_fd)
{
    // Socket receive timeout, secure https.
#ifndef HAVE_WINDOWS
    timeval tv{};
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

    // The environment for this request.
    // It gets passed around from function to function during the entire request.
    // This provides thread-safety to the request.
    Webserver_Request request{};

    // This is the secure http server.
    request.secure = true;

    SSL* ssl = SSL_new(context);
    if (!ssl)
    {
        webserver_log_ssl_errors("Failure to create a secure connection");
#ifdef HAVE_WINDOWS
        closesocket(client_fd);
#else
        close(client_fd);
#endif
        return;
    }
    SSL_set_fd(ssl, client_fd);

    try
    {
        if (config_globals_webserver_running)
        {
            // Get the client's remote IPv4 address in dotted notation,
            // or the IPv6 address in the proper notation,
            // and put it in the webserver request object.
            {
                sockaddr_storage client_addr{};
                socklen_t socklen = sizeof(client_addr);
                getpeername(client_fd, reinterpret_cast<sockaddr*>(&client_addr), &socklen);
                char remote_address[256];
                if (client_addr.ss_family == AF_INET)
                {
                    auto* s = reinterpret_cast<sockaddr_in*>(&client_addr);
                    inet_ntop(AF_INET, &s->sin_addr, remote_address, sizeof remote_address);
                }
                else if (client_addr.ss_family == AF_INET6)
                {
                    auto* s = reinterpret_cast<sockaddr_in6*>(&client_addr);
                    inet_ntop(AF_INET6, &s->sin6_addr, remote_address, sizeof remote_address);
                }
                else remote_address[0] = '\0';
                request.remote_address = remote_address;
                convert_ipv6_notation_to_pure_ipv4_notation(request.remote_address);
            }

            // This flag indicates a healthy connection: One that can proceed.
            bool connection_healthy = true;

            // SSL / TLS handshake.
            if (SSL_accept(ssl) != 1)
            {
                webserver_log_ssl_errors("Secure connection handshake failed", request.remote_address);
                connection_healthy = false;
            }

            // Read the HTTP headers.
            bool header_parsed = true;
            std::string header_line{};
            while (connection_healthy && header_parsed)
            {
                // Read and parse one line of data from the client.
                char c{0};
                const int result = webserver_ssl_read_byte(ssl, c);
                if (result == 0) header_parsed = false; // EOF
                if (result < 0) connection_healthy = false;
                if (connection_healthy && header_parsed)
                {
                    if (c == '\r') continue;
                    if (c == '\n')
                    {
                        header_parsed = http_parse_header(header_line, request);
                        header_line.clear();
                    }
                    else
                    {
                        header_line += c;
                    }
                }
            }
            header_line.clear();

            if (request.is_post)
            {
                std::string post_data{};
                bool done_reading = false;
                int total_bytes_read = 0;
                while (connection_healthy && !done_reading)
                {
                    char c{0};
                    const int result = webserver_ssl_read_byte(ssl, c);
                    if (result == 0) done_reading = true; // EOF
                    if (result < 0) connection_healthy = false;
                    if (connection_healthy && !done_reading)
                    {
                        post_data += c;
                        total_bytes_read++;
                    }
                    if (total_bytes_read >= request.content_length) done_reading = true;
                }
                if (total_bytes_read < request.content_length) connection_healthy = false;
                if (connection_healthy)
                {
                    http_parse_post(post_data, request);
                }
            }

            // Assemble response.
            if (connection_healthy)
            {
                bootstrap_index(request);
                http_assemble_response(request);
            }

            // Write the response to the browser.
            if (connection_healthy && !request.reply.empty())
            {
                connection_healthy = webserver_ssl_write_all(ssl, request.reply.data(), request.reply.size());
            }

            // When streaming a file, copy file contents straight from disk to the network connection.
            if (connection_healthy && !request.stream_file.empty())
            {
                int filefd =
#ifdef HAVE_WINDOWS
                    _open
#else
                    open
#endif
                    (request.stream_file.c_str(), O_RDONLY);
                unsigned char buffer[1024];
                int bytecount;
                do
                {
                    bytecount = static_cast<int>(
#ifdef HAVE_WINDOWS
                        _read
#else
                        read
#endif
                        (filefd, buffer, 1024));
                    if (bytecount > 0)
                    {
                        connection_healthy = webserver_ssl_write_all(
                            ssl, reinterpret_cast<const char*>(buffer), static_cast<size_t>(bytecount));
                    }
                }
                while (connection_healthy && bytecount > 0);
#ifdef HAVE_WINDOWS
                _close
#else
                close
#endif
                    (filefd);
            }

            // Close SSL/TLS connection.
            if (connection_healthy) SSL_shutdown(ssl);
        }
    }
    catch (const std::exception& e)
    {
        std::string message("Internal error: ");
        message.append(e.what());
        database::logs::log(message);
    }
    catch (const std::exception* e) // NOLINT(*-throw-by-value-catch-by-reference)
    {
        std::string message("Internal error: ");
        message.append(e->what());
        database::logs::log(message);
    }
    catch (...)
    {
        database::logs::log("A general internal error occurred");
    }

    // Done with the SSL object, then close the client network connection.
    SSL_free(ssl);
#ifdef HAVE_WINDOWS
    closesocket(client_fd);
#else
    close(client_fd);
#endif
}


void https_server()
{
    // On clients, don't run the secure web server.
    // It is not possible to get a https certificate for https://localhost anyway.
    // Not running this secure server saves valuable system resources on low power devices.
    if constexpr (not config::logic::run_secure_server())
        return;

    // The https network port to listen on.
    // Port 0..9 means: Don't run the secure web server.
    const std::string network_port = config::logic::https_network_port();
    if (network_port.length() <= 1)
        return;

    // Check whether all the certificates are there and can be read.
    // If not, log some feedback and don't run the secure web server.
    const std::string server_key_path{config::logic::server_key_path(false)};
    const std::string server_certificate_path{config::logic::server_certificate_path(false)};
    const std::string authorities_certificates_path{config::logic::authorities_certificates_path(false)};
    if (!server_key_path.empty())
    {
        if (const std::string contents{filter_url_file_get_contents(server_key_path)};
            contents.empty())
        {
            database::logs::log("Cannot read", server_key_path, "so not running secure server");
            return;
        }
    }
    else
    {
        database::logs::log("Cannot find server private key in", config::logic::server_key_path(true), "so not running secure server");
        return;
    }
    if (!server_certificate_path.empty())
    {
        if (filter_url_file_get_contents(server_certificate_path).empty())
        {
            database::logs::log("Cannot read", server_certificate_path, "so not running secure server");
            return;
        }
    }
    else
    {
        database::logs::log("Cannot find server certificate in", config::logic::server_certificate_path(true), "so not running secure server");
        return;
    }
    if (!authorities_certificates_path.empty())
    {
        if (const std::string contents{filter_url_file_get_contents(authorities_certificates_path)};
            contents.empty())
        {
            database::logs::log("Cannot read", authorities_certificates_path, "so not running secure server");
            return;
        }
    }
    else
    {
        database::logs::log("Cannot find certificate authorities chain in", config::logic::authorities_certificates_path(true), "so not running secure server");
        return;
    }

    // The SSL context for the lifetime of the server.
    std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> context{SSL_CTX_new(TLS_server_method()), SSL_CTX_free};
    if (!context)
    {
        webserver_log_ssl_errors("Failure to create the secure server context: Not running the secure server");
        return;
    }
    SSL_CTX_set_min_proto_version(context.get(), TLS1_2_VERSION);
    if (SSL_CTX_use_certificate_file(context.get(), server_certificate_path.c_str(), SSL_FILETYPE_PEM) != 1)
    {
        webserver_log_ssl_errors("Cannot load server certificate: Not running the secure server");
        return;
    }
    if (!webserver_add_certificate_chain(context.get(), authorities_certificates_path))
    {
        webserver_log_ssl_errors("Cannot load certificate authorities chain: Not running the secure server");
        return;
    }
    if (SSL_CTX_use_PrivateKey_file(context.get(), server_key_path.c_str(), SSL_FILETYPE_PEM) != 1)
    {
        webserver_log_ssl_errors("Cannot load server private key: Not running the secure server");
        return;
    }
    if (SSL_CTX_check_private_key(context.get()) != 1)
    {
        webserver_log_ssl_errors("Server private key does not match the certificate: Not running the secure server");
        return;
    }
    SSL_CTX_load_verify_locations(context.get(), authorities_certificates_path.c_str(), nullptr);
    SSL_CTX_set_session_cache_mode(context.get(), SSL_SESS_CACHE_SERVER);

    // Setup the listening TCP socket.
    const int listen_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        database::logs::log("Error opening secure socket:", strerror(errno));
        return;
    }
    int opt_val{1};
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
#ifdef IPV6_V6ONLY
    opt_val = 0;
    setsockopt(listen_fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt_val, sizeof(opt_val));
#endif
    sockaddr_in6 server_addr{};
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(static_cast<uint16_t>(filter::string::convert_to_int(network_port)));
    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) != 0)
    {
        database::logs::log("Error binding secure server to socket:", strerror(errno));
        close(listen_fd);
        return;
    }
    if (listen(listen_fd, 100) != 0)
    {
        database::logs::log("Error listening on secure socket:", strerror(errno));
        close(listen_fd);
        return;
    }

    // Whether the plain http server redirects to secure http.
    // Possible automatic forwarding from http to https is postponed till here
    // because only here it is known whether the https server runs at all.
    // https://github.com/bibledit/cloud/issues/866
    config_globals_enforce_https_browser = config::logic::enforce_https_browser();
    config_globals_enforce_https_client = config::logic::enforce_https_client();

    std::cout << "Listening on https://localhost:" << network_port << std::endl;

    // Keep accepting and processing client connections.
    while (config_globals_webserver_running)
    {
        sockaddr_in6 client_addr{};
        socklen_t client_len = sizeof(client_addr);
        const int client_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client_fd < 0)
        {
            if (config_globals_webserver_running)
                database::logs::log("Error accepting secure connection:", strerror(errno));
            continue;
        }
        enqueue_task([ctx = context.get(), client_fd] { secure_webserver_process_request(ctx, client_fd); });
    }

    // Wait shortly to give sufficient time to let the connection fail,
    // before the SSL context gets out of scope.
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // Close listening socket, freeing it for a possible subsequent server process.
    close(listen_fd);
}


// Storage for the worker threads
static std::vector<std::thread> thread_pool;

// Queue of tasks.
//   The underlying std::deque is unlikely to give severe heap fragmentation
//   because it allocates a block on the heap for many elements at once,
//   dozens or hundreds per chunk.
static std::queue<std::function<void()>> tasks;

// Mutex to synchronize access to shared data.
static std::mutex mutex;

// Condition variable to signal changes in the state of the tasks queue.
static std::condition_variable cv;

// Flag to indicate whether the thread pool should run.
static std::atomic run_pool {false};

void start_thread_pool(const std::size_t num_threads)
{
    // Guard against double starting.
    if (run_pool)
        return;
    // Flag to run.
    run_pool = true;
    // Creating worker threads.
    for (size_t i = 0; i < num_threads; ++i) {
        thread_pool.emplace_back([] {
            while (true) {
                std::function<void()> task;
                // The reason for putting the below code here is to unlock the queue
                // before executing the task so that other threads can perform enqueue tasks.
                {
                    // Locking the queue so that data can be shared safely.
                    std::unique_lock lock(mutex);

                    // Waiting until there is a task to execute or the pool is stopped.
                    // While in .wait it unlocks the mutex on the queue.
                    cv.wait(lock, [] {
                        return not tasks.empty() or not run_pool;
                    });

                    // Exit the thread in case the pool is stopped, disregarding pending http requests.
                    if (not run_pool)
                        return;

                    // Get the next task from the queue.
                    task = std::move(tasks.front());
                    tasks.pop();
                }

                // Run the task in this thread.
                task();
            }
        });
    }
}


void stop_thread_pool()
{
    // Guard against double stopping.
    if (not run_pool)
        return;

    // Indicate stop.
    run_pool = false;

    // Notify all threads
    cv.notify_all();

    // Join all worker threads, if possible, to ensure they have completed their tasks.
    std::ranges::for_each(thread_pool, [](std::thread &t)
    {
        if (t.joinable())
            t.join();
    });
    // Clear them so they are no longer available on a possible subsequent shutdown.
    thread_pool.clear();
}


void enqueue_task(std::function<void()> task)
{
    bool large_queue {false};
    {
        std::unique_lock lock(mutex);
        tasks.emplace(std::move(task));
        large_queue = tasks.size() > 50;
    }
    cv.notify_one();
    // If the queue gets too large, it waits shortly.
    // That should reduce the request rate.
    if (large_queue)
    {
        using namespace std::literals;
        std::this_thread::sleep_for(100ms);
    }
}
