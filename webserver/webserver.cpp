/*
Copyright (©) 2003-2025 Teus Benschop.

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


#include <webserver/webserver.h>
#include <webserver/http.h>
#include <bootstrap/bootstrap.h>
#include <webserver/request.h>
#include <config/globals.h>
#include <database/logs.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wc99-extensions"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wswitch-enum"
#include <mbedtls/version.h>
#include <mbedtls/platform.h>
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl_cache.h"
#pragma GCC diagnostic pop
#ifdef HAVE_WINDOWS
#include <io.h>
#endif


// Static check on required definitions, taken from the ssl_server.c example.
#ifndef MBEDTLS_BIGNUM_C
static_assert (false, "MBEDTLS_BIGNUM_C should be defined");
#endif
#ifndef MBEDTLS_PEM_PARSE_C
static_assert (false, "MBEDTLS_PEM_PARSE_C should be defined");
#endif
#ifndef MBEDTLS_ENTROPY_C
static_assert (false, "MBEDTLS_ENTROPY_C should be defined");
#endif
#ifndef MBEDTLS_SSL_TLS_C
static_assert (false, "MBEDTLS_SSL_TLS_C should be defined");
#endif
#ifndef MBEDTLS_SSL_CLI_C
static_assert (false, "MBEDTLS_SSL_CLI_C should be defined");
#endif
#ifndef MBEDTLS_SSL_SRV_C
static_assert (false, "MBEDTLS_SSL_SRV_C should be defined");
#endif
#ifndef MBEDTLS_NET_C
static_assert (false, "MBEDTLS_NET_C should be defined");
#endif
#ifndef MBEDTLS_RSA_C
static_assert (false, "MBEDTLS_RSA_C should be defined");
#endif
#ifndef MBEDTLS_PEM_PARSE_C
static_assert (false, "MBEDTLS_PEM_PARSE_C should be defined");
#endif
#ifndef MBEDTLS_CTR_DRBG_C
static_assert (false, "MBEDTLS_CTR_DRBG_C should be defined");
#endif
#ifndef MBEDTLS_X509_CRT_PARSE_C
static_assert (false, "MBEDTLS_X509_CRT_PARSE_C should be defined");
#endif
#ifndef MBEDTLS_FS_IO
static_assert (false, "MBEDTLS_FS_IO should be defined");
#endif
//#ifndef MBEDTLS_USE_PSA_CRYPTO
//static_assert (false, "MBEDTLS_USE_PSA_CRYPTO should be defined");
//#endif
#ifdef MBEDTLS_X509_REMOVE_INFO
static_assert (false, "MBEDTLS_X509_REMOVE_INFO should not be defined");
#endif


#if MBEDTLS_VERSION_MAJOR == 2
#elif MBEDTLS_VERSION_MAJOR == 3
#else
static_assert (false, "MbedTLS version other than 2 or 3");
#endif


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
static int get_line (const int sock, char *buf, const int size)
{
  int i {0};
  char character {'\0'};
  int n {0};
  while ((i < size - 1) && (character != '\n')) {
    n = static_cast<int> (::recv (sock, &character, 1, 0));
    if (n > 0) {
      if (character == '\r') {
        n = static_cast<int> (recv (sock, &character, 1, MSG_PEEK));
        if ((n > 0) && (character == '\n')) {
          recv (sock, &character, 1, 0);
        } else {
          character = '\n';
        }
      }
      buf[i] = character;
      i++;
    } else {
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
  if (size_t pos = address.find(".");
      pos != std::string::npos) {
    pos = address.find_last_of(":");
    address.erase (0, ++pos);
  }
}


// Processes a single request from a web client.
static void webserver_process_request (const int connfd, const std::string& clientaddress)
{
  // The environment for this request.
  // It reference to this object gets passed around from function to function during the entire request.
  // This provides thread-safety to the request.
  Webserver_Request request {};
  
  // This is the plain http server.
  request.secure = false;
  
  // Store remote client address in the request.
  request.remote_address = clientaddress;
  
  try {
    if (config_globals_webserver_running) {
      
      // Connection health flag.
      bool connection_healthy {true};
      
      // Read the client's request.
      // With the HTTP protocol it is not possible to read the request till EOF,
      // because EOF does never come, because the browser keeps the connection open
      // for receiving the response.
      // The HTTP protocol works per line.
      // Read one line of data from the client.
      // An empty line marks the end of the headers.
#define BUFFERSIZE 2048
      int bytes_read {};
      bool header_parsed {true};
      char buffer [BUFFERSIZE];
      // Fix valgrind unitialized value message.
      memset (&buffer, 0, BUFFERSIZE);
      do {
        bytes_read = get_line (connfd, buffer, BUFFERSIZE);
        if (bytes_read <= 0) connection_healthy = false;
        // Parse the browser's request's headers.
        header_parsed = http_parse_header (buffer, request);
      } while (header_parsed);

      if (connection_healthy) {
        
        // In the case of a POST request, more data follows: The POST request itself.
        // The length of that data is indicated in the header's Content-Length line.
        // Read that data, and parse it.
        std::string postdata {};
        if (request.is_post) {
          bool done_reading {false};
          int total_bytes_read {0};
          do {
            bytes_read = static_cast<int> (recv(connfd, buffer, BUFFERSIZE, 0));
            for (int i = 0; i < bytes_read; i++) {
              postdata += buffer [i];
            }
            // EOF indicates reading is ready.
            // An error also indicates that reading is ready.
            if (bytes_read <= 0) done_reading = true;
            if (bytes_read < 0) connection_healthy = false;
            // "Content-Length" bytes read: Done.
            total_bytes_read += bytes_read;
            if (total_bytes_read >= request.content_length) done_reading = true;
          } while (!done_reading);
          if (total_bytes_read < request.content_length) connection_healthy = false;
        }
        
        if (connection_healthy) {
          
          http_parse_post (postdata, request);
          
          // Assemble response.
          bootstrap_index (request);
          http_assemble_response (request);
          
          // Send response to browser.
          const char * output = request.reply.c_str();
          // The C function strlen () fails on null characters in the reply, so use string::size() instead.
          size_t length = request.reply.size ();
          send (connfd, output, length, 0);
          
          // When streaming a file, copy the file's contents straight from disk to the network file descriptor.
          // Do not load the entire file into memory.
          // This enables large file transfers on low-memory devices.
          // Also handle cases that the requested file does not exist.
          // So the number of bytes read should be larger than zero, not unequal to zero.
          // In the case of != 0, it falls in an endless loop, because -1 indicates failure.
          if (!request.stream_file.empty ()) {
            int filefd =
#ifdef HAVE_WINDOWS
            _open
#else
            open
#endif
            (request.stream_file.c_str(), O_RDONLY);
            unsigned char streambuffer [1024];
            int bytecount {};
            do {
              bytecount = static_cast<int> (
#ifdef HAVE_WINDOWS
              _read
#else
              read
#endif
              (filefd, streambuffer, 1024));
              if (bytecount > 0) {
                [[maybe_unused]] auto sendbytes = send (connfd, reinterpret_cast<const char *> (streambuffer), static_cast<size_t>(bytecount), 0);
              }
            }
            while (bytecount > 0);
#ifdef HAVE_WINDOWS
            _close
#else
            close
#endif
            (filefd);
          }
        }
      }
    }
  } 
  catch (const std::exception& e) {
    std::string message ("Internal error: ");
    message.append (e.what ());
    Database_Logs::log (message);
  } 
  catch (const std::exception* e) {
    std::string message ("Internal error: ");
    message.append (e->what ());
    Database_Logs::log (message);
  } 
  catch (...) {
    Database_Logs::log ("A general internal error occurred");
  }
  
  // Done: Close.
#ifdef HAVE_WINDOWS
  shutdown (connfd, SD_BOTH);
  closesocket (connfd);
#else
  shutdown (connfd, SHUT_RDWR);
  close (connfd);
#endif
}


#ifndef HAVE_WINDOWS
// This http server uses BSD sockets.
void http_server ()
{
  bool listener_healthy {true};

  // Create a listening socket.
  // This represents an endpoint.
  // This prepares to accept incoming connections on.
#ifdef HAVE_CLIENT
  // A client listens on IPv4, see also below.
  const int listenfd = socket (AF_INET, SOCK_STREAM, 0);
#endif
#ifdef HAVE_CLOUD
  // The Cloud listens on address family AF_INET6 for both IPv4 and IPv6.
  const int listenfd = socket (AF_INET6, SOCK_STREAM, 0);
#endif
  if (listenfd < 0) {
    std::string error = "Error opening socket: ";
    error.append (strerror (errno));
    std::cerr << error << std::endl;
    Database_Logs::log (error);
    listener_healthy = false;
  }

  // Eliminate "Address already in use" error from bind.
  // The function is used to allow the local address to  be reused
  // when the server is restarted before the required wait time expires.
  int optval {1};
  int result = setsockopt (listenfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *> ( &optval), sizeof (int));
  if (result != 0) {
    std::string error = "Error setting socket option: ";
    error.append (strerror (errno));
    std::cerr << error << std::endl;
    Database_Logs::log (error);
  }

  // The listening socket will be an endpoint for all requests to a port on this host.
#ifdef HAVE_CLIENT
  // When configured as a client, it listens on the IPv4 loopback device.
  // It has been seen on Ubuntu 16.04 that a Bibledit Client would not listen on a IPv6 loopback device.
  struct sockaddr_in serveraddr;
  memset (&serveraddr, 0, sizeof (serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  serveraddr.sin_port = htons (filter::strings::convert_to_int (config::logic::http_network_port ()));
#endif
#ifdef HAVE_CLOUD
  // When configured as a server it listens on any IPv6 address.
  sockaddr_in6 serveraddr;
  memset (&serveraddr, 0, sizeof (serveraddr));
  serveraddr.sin6_flowinfo = 0;
  serveraddr.sin6_family = AF_INET6;
  serveraddr.sin6_addr = in6addr_any;
  serveraddr.sin6_port = htons (static_cast<uint16_t>(filter::strings::convert_to_int (config::logic::http_network_port ())));
#endif
  result = ::bind (listenfd, reinterpret_cast<sockaddr *>(&serveraddr), sizeof (serveraddr));
  if (result != 0) {
    std::string error = "Error binding server to socket: ";
    error.append (strerror (errno));
    std::cerr << error << std::endl;
    Database_Logs::log (error);
    listener_healthy = false;
  }

  // Make it a listening socket ready to queue and accept many connection requests
  // before the system starts rejecting the incoming requests.
  result = listen (listenfd, 100);
  if (result != 0) {
    std::string error = "Error listening on socket: ";
    error.append (strerror (errno));
    std::cerr << error << std::endl;
    Database_Logs::log (error);
    listener_healthy = false;
  }

  // Keep waiting for, accepting, and processing connections.
  while (listener_healthy && config_globals_webserver_running) {

    // Socket and file descriptor for the client connection.
    sockaddr_in6 clientaddr6;
    socklen_t clientlen = sizeof (clientaddr6);
    int connfd = accept (listenfd, reinterpret_cast<sockaddr *>(&clientaddr6), &clientlen);
    if (connfd > 0) {

      // Socket receive timeout, plain http.
      timeval tv;
      tv.tv_sec = 60;
      tv.tv_usec = 0;
      setsockopt (connfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
      
      // The client's remote IPv6 address in hexadecimal digits separated by colons.
      // IPv4 addresses are mapped to IPv6 addresses.
      // Example IPv4 address: ::ffff:127.0.0.1
      // Example IPv6 address: ::1
      // Clean the IP address up so it's a clear IPv4 or IPv6 notation.
      char remote_address[256];
      inet_ntop (AF_INET6, &clientaddr6.sin6_addr, remote_address, sizeof (remote_address));
      std::string clientaddress = remote_address;
      convert_ipv6_notation_to_pure_ipv4_notation (clientaddress);

      // Handle this request in a thread, enabling parallel requests.
      std::thread request_thread = std::thread (webserver_process_request, connfd, clientaddress);
      // Detach and delete thread object.
      request_thread.detach ();
      
    } else {
      std::string error = "Error accepting connection on socket: ";
      error.append (strerror (errno));
      std::cerr << error << std::endl;
      Database_Logs::log (error);
    }
  }
  
  // Close listening socket, freeing it for any next server process.
  close (listenfd);
}
#endif


#ifdef HAVE_WINDOWS
bool server_accepting_flag {false};
std::mutex server_accepting_mutex;


void http_server_acceptor_processor (SOCKET listen_socket)
{
  // Accept a client socket.
  server_accepting_mutex.lock ();
  server_accepting_flag = true;
  server_accepting_mutex.unlock ();
  struct sockaddr_in clientaddr;
  socklen_t clientlen = sizeof (clientaddr);
  // The SOCKET in Windows will be closed when it goes out of scope.
  // This is different from a Unix file descriptor.
  // Therefore there's a difference in web server architecture between them.
  SOCKET client_socket = accept (listen_socket, (struct sockaddr *)&clientaddr, &clientlen);
  server_accepting_mutex.lock ();
  server_accepting_flag = false;
  server_accepting_mutex.unlock ();
  if (client_socket != INVALID_SOCKET) {

    // Set timeout on receive, in milliseconds.
    const char * tv = "600000";
    setsockopt (client_socket, SOL_SOCKET, SO_RCVTIMEO, tv, sizeof (tv));

    // The client's remote IPv4 address in dotted notation.
    std::string clientaddress;
    char remote_address[256];
    inet_ntop (AF_INET, &clientaddr.sin_addr.s_addr, remote_address, sizeof (remote_address));
    clientaddress = remote_address;

    webserver_process_request (client_socket, clientaddress);
  }

  // Shutdown and close the connection.
  shutdown (client_socket, SD_BOTH);
  closesocket (client_socket);
}
#endif


#ifdef HAVE_WINDOWS
// This http server uses Windows sockets.
void http_server ()
{
  int result;
  bool listener_healthy = true;
  
  // Initialize the interface to Windows Sockets.
  WSADATA wsa_data;
  result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
  if (result != 0) {
    std::string error = "Could not initialize Windows Sockets with error " + std::to_string (result);
    std::cerr << error << std::endl;
    Database_Logs::log (error);
    listener_healthy = false;
  }
  // Check for the correct requested Windows Sockets interface version.
  if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2) {
    std::string error = "Incorrect Windows Sockets version";
    std::cerr << error << std::endl;
    Database_Logs::log (error);
    listener_healthy = false;
  }
  
  // Create a socket for listening for incoming connections.
  SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket == INVALID_SOCKET) {
    std::string error = "Socket failed with error " + std::to_string (WSAGetLastError());
    std::cerr << error << std::endl;
    Database_Logs::log (error);
    listener_healthy = false;
  }

  // The listening socket will be an endpoint for all requests to a port on this host.
  // As Windows is a client, it listens on the loopback device only, for improved security.
  typedef struct sockaddr SA;
  struct sockaddr_in serveraddr;
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  serveraddr.sin_port = htons(filter::strings::convert_to_int(config::logic::http_network_port()));
  result = ::bind(listen_socket, (SA *)&serveraddr, sizeof(serveraddr));
  if (result == SOCKET_ERROR) {
	  std::string error = "Error binding server to socket";
    std::cerr << error << std::endl;
    Database_Logs::log (error);
	  listener_healthy = false;
  }

  // Listen for multiple connections.
  result = listen(listen_socket, SOMAXCONN);
  if (result == SOCKET_ERROR) {
    std::string error = "Listen failed with error " + std::to_string (WSAGetLastError());
    std::cerr << error << std::endl;
    Database_Logs::log (error);
    listener_healthy = false;
  }
  
  // Keep waiting for, accepting, and processing connections.
  config_globals_webserver_running = true;
  while (listener_healthy && config_globals_webserver_running) {

    // Poll the system whether to wait for a client.
    bool start_acceptor = false;
    server_accepting_mutex.lock ();
    if (!server_accepting_flag) start_acceptor = true;
    server_accepting_mutex.unlock ();
    if (start_acceptor) {
      // Handle waiting for and processing the request in a thread, enabling parallel requests.
      std::thread request_thread = std::thread (http_server_acceptor_processor, listen_socket);
      // Detach and delete thread object.
      request_thread.detach ();
    }
    // Wait shortly before next poll iteration.
    std::this_thread::sleep_for (std::chrono::milliseconds (10));
  }
  
  // No longer need server socket
  closesocket(listen_socket);
  
  // Clean up the interface to Windows Sockets.
  WSACleanup();
}
#endif


// Processes a single request from a web client.
static void secure_webserver_process_request (mbedtls_ssl_config * conf, mbedtls_net_context client_fd)
{
  // Socket receive timeout, secure https.
#ifndef HAVE_WINDOWS
  timeval tv;
  tv.tv_sec = 60;
  tv.tv_usec = 0;
  setsockopt (client_fd.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
  
  // The environment for this request.
  // It gets passed around from function to function during the entire request.
  // This provides thread-safety to the request.
  Webserver_Request request {};
  
  // This is the secure http server.
  request.secure = true;
  
  // SSL/TSL data.
  mbedtls_ssl_context ssl;
  mbedtls_ssl_init (&ssl);
  
  try {

    if (config_globals_webserver_running) {

      // Get the client's remote IPv4 address in dotted notation,
      // or the IPv6 address in the proper notation,
      // and put it in the webserver request object.
      {
        sockaddr_storage client_addr;
        socklen_t socklen = sizeof(client_addr);
        getpeername(client_fd.fd, reinterpret_cast<sockaddr *>(&client_addr), &socklen);
        char remote_address [256];
        if (client_addr.ss_family == AF_INET) {
          struct sockaddr_in *s = reinterpret_cast<sockaddr_in *>(&client_addr);
          inet_ntop(AF_INET, &s->sin_addr, remote_address, sizeof remote_address);
        } else if(client_addr.ss_family == AF_INET6) {
          struct sockaddr_in6 *s = reinterpret_cast<sockaddr_in6 *>(&client_addr);
          inet_ntop(AF_INET6, &s->sin6_addr, remote_address, sizeof remote_address);
        }
        request.remote_address = remote_address;
        convert_ipv6_notation_to_pure_ipv4_notation (request.remote_address);
      }
      
      // This flag indicates a healthy connection: One that can proceed.
      bool connection_healthy = true;
      // Function results.
      int ret;
      
      if (connection_healthy) {
        ret = mbedtls_ssl_setup (&ssl, conf);
        if (ret != 0) {
          filter_url_display_mbed_tls_error (ret, nullptr, true, request.remote_address);
          connection_healthy = false;
        }
      }
      
      if (connection_healthy) {
        mbedtls_ssl_set_bio (&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, nullptr);
      }
      
      // SSL / TLS handshake.
      while (connection_healthy && (ret = mbedtls_ssl_handshake (&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
          if (config_globals_webserver_running) {
            // In case the secure server runs, display the error.
            // And in case the server is interrupted by e.g. Ctrl-C, don't display this error.
            filter_url_display_mbed_tls_error (ret, nullptr, true, request.remote_address);
          }
          connection_healthy = false;
        }
      }
      
      // Read the HTTP headers.
      bool header_parsed = true;
      std::string header_line {};
      while (connection_healthy && header_parsed) {
        // Read the client's request.
        // With the HTTP protocol it is not possible to read the request till EOF,
        // because EOF does not always come,
        // since the browser may keep the connection open for the response.
        // The HTTP protocol works per line.
        // Read and parse one line of data from the client.
        // An empty line marks the end of the headers.
        unsigned char buffer [1];
        memset (&buffer, 0, 1);
        ret = mbedtls_ssl_read (&ssl, buffer, 1);
        if (ret == MBEDTLS_ERR_SSL_WANT_READ) continue;
        if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        if (ret == 0) header_parsed = false; // 0: EOF
        if (ret < 0) connection_healthy = false;
        if (connection_healthy && header_parsed) {
          char c = static_cast <char> (buffer [0]);
          // The request contains a carriage return (\r) and a new line feed (\n).
          // The traditional order of this is \r\n.
          // Therefore when a \r is encountered, just disregard it.
          // A \n will follow to mark the end of the header line.
          if (c == '\r') continue;
          // At a new line, parse the received header line.
          if (c == '\n') {
            header_parsed = http_parse_header (header_line, request);
            header_line.clear ();
          } else {
            header_line += c;
          }
        }
      }
      header_line.clear ();
      
      if (request.is_post) {
        // In the case of a POST request, more data follows:
        // The POST request itself.
        // The length of that data is indicated in the header's Content-Length line.
        // Read that data.
        std::string postdata{};
        bool done_reading = false;
        int total_bytes_read = 0;
        while (connection_healthy && !done_reading) {
          unsigned char buffer [1];
          memset (&buffer, 0, 1);
          ret = mbedtls_ssl_read (&ssl, buffer, 1);
          if (ret == MBEDTLS_ERR_SSL_WANT_READ) continue;
          if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
          if (ret == 0) done_reading = true; // 0: EOF
          if (ret < 0) connection_healthy = false;
          if (connection_healthy && !done_reading) {
            char c = static_cast <char> (buffer [0]);
            postdata += c;
            total_bytes_read ++;
          }
          // "Content-Length" bytes read: Done.
          if (total_bytes_read >= request.content_length) done_reading = true;
        }
        if (total_bytes_read < request.content_length) connection_healthy = false;
        // Parse the POSTed data.
        if (connection_healthy) {
          http_parse_post (postdata, request);
        }
      }
      
      // Assemble response.
      if (connection_healthy) {
        bootstrap_index (request);
        http_assemble_response (request);
      }
      
      // Write the response to the browser.
      const char * output = request.reply.c_str();
      const unsigned char * buf = reinterpret_cast<const unsigned char *>(output);
      // The C function strlen () fails on null characters in the reply, so take string::size()
      size_t len = request.reply.size ();
      while (connection_healthy && (len > 0)) {
        // Function
        // int ret = mbedtls_ssl_write (&ssl, buf, len)
        // will do partial writes in some cases.
        // If the return value is non-negative but less than length,
        // the function must be called again with updated arguments:
        // buf + ret, len - ret
        // until it returns a value equal to the last 'len' argument.
        ret = mbedtls_ssl_write (&ssl, buf, len);
        if (ret > 0) {
          buf += ret;
          len -= static_cast <size_t> (ret);
        } else {
          // When it returns MBEDTLS_ERR_SSL_WANT_WRITE/READ,
          // it must be called later with the *same* arguments,
          // until it returns a positive value.
          if (ret == MBEDTLS_ERR_SSL_WANT_READ) continue;
          if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
          filter_url_display_mbed_tls_error (ret, nullptr, true, request.remote_address);
          connection_healthy = false;
        }
      }

      // When streaming a file, copy file contents straight from disk to the network file descriptor.
      // Do not load the entire file into memory.
      // This enables large file transfers on low-memory devices.
      if (!request.stream_file.empty ()) {
        int filefd =
#ifdef HAVE_WINDOWS
        _open
#else
        open
#endif
        (request.stream_file.c_str(), O_RDONLY);
        unsigned char buffer [1024];
        int bytecount;
        do {
          bytecount = static_cast<int>(
#ifdef HAVE_WINDOWS
          _read
#else
          read
#endif
          (filefd, buffer, 1024));
          int remaining_length = bytecount;
          const unsigned char * buffer_ptr = reinterpret_cast<const unsigned char *>(&buffer);
          while (connection_healthy && (remaining_length > 0)) {
            // Function
            // int ret = mbedtls_ssl_write (&ssl, buf, len)
            // will do partial writes in some cases.
            // If the return value is non-negative but less than length,
            // the function must be called again with updated arguments:
            // buf + ret, len - ret
            // until it returns a value equal to the last 'len' argument.
            ret = mbedtls_ssl_write (&ssl, buffer_ptr, static_cast<size_t>(remaining_length));
            if (ret > 0) {
              buffer_ptr += ret;
              remaining_length -= ret;
            } else {
              // When it returns MBEDTLS_ERR_SSL_WANT_WRITE/READ,
              // it must be called later with the *same* arguments,
              // until it returns a positive value.
              if (ret == MBEDTLS_ERR_SSL_WANT_READ) continue;
              if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
              filter_url_display_mbed_tls_error (ret, nullptr, true, request.remote_address);
              connection_healthy = false;
            }
          }
        }
        while (bytecount > 0);
#ifdef HAVE_WINDOWS
        _close
#else
        close
#endif
        (filefd);
      }
      
      // Close SSL/TLS connection.
      if (connection_healthy) {
        while ((ret = mbedtls_ssl_close_notify (&ssl)) < 0) {
          if (ret == MBEDTLS_ERR_SSL_WANT_READ) continue;
          if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
          filter_url_display_mbed_tls_error (ret, nullptr, true, request.remote_address);
          connection_healthy = false;
          if (connection_healthy) {}; // Suppress static analyzer warning about unused code.
          break;
        }
      }
      
    }
  } 
  catch (const std::exception& e) {
    std::string message ("Internal error: ");
    message.append (e.what ());
    Database_Logs::log (message);
  } 
  catch (const std::exception * e) {
    std::string message ("Internal error: ");
    message.append (e->what ());
    Database_Logs::log (message);
  } 
  catch (...) {
    Database_Logs::log ("A general internal error occurred");
  }
  
  // Close client network connection.
  mbedtls_net_free (&client_fd);
  
  // Done with the SSL context.
  mbedtls_ssl_free (&ssl);
}


void https_server ()
{
  // On clients, don't run the secure web server.
  // It is not possible to get a https certificate for https://localhost anyway.
  // Not running this secure server saves valuable system resources on low power devices.
#ifdef RUN_SECURE_SERVER

  // The https network port to listen on.
  // Port 0..9 means: Don't run the secure web server.
  const std::string network_port = config::logic::https_network_port ();
  if (network_port.length() <= 1) 
    return;
  
  // Check whether all the certificates are there and can be read.
  // If not, log some feedback and don't run the secure web server.
  const std::string server_key_path {config::logic::server_key_path (false)};
  const std::string server_certificate_path {config::logic::server_certificate_path (false)};
  const std::string authorities_certificates_path {config::logic::authorities_certificates_path (false)};
  if (!server_key_path.empty()) {
    const std::string contents {filter_url_file_get_contents (server_key_path)};
    if (contents.empty()) {
      Database_Logs::log("Cannot read " + server_key_path + " so not running secure server");
      return;
    }
  } else {
    Database_Logs::log("Cannot find server private key in " + config::logic::server_key_path (true) + " so not running secure server");
    return;
  }
  if (!server_certificate_path.empty()) {
    const std::string contents {filter_url_file_get_contents (server_certificate_path)};
    if (contents.empty()) {
      Database_Logs::log("Cannot read " + server_certificate_path + " so not running secure server");
      return;
    }
  } else {
    Database_Logs::log("Cannot find server certificate in " + config::logic::server_certificate_path (true) + " so not running secure server");
    return;
  }
  if (!authorities_certificates_path.empty()) {
    std::string contents {filter_url_file_get_contents (authorities_certificates_path)};
    if (contents.empty()) {
      Database_Logs::log("Cannot read " + authorities_certificates_path + " so not running secure server");
      return;
    }
  } else {
    Database_Logs::log("Cannot find certificate authorities chain in " + config::logic::authorities_certificates_path (true) + " so not running secure server");
    return;
  }
  
  // File descriptor for the listener.
  mbedtls_net_context listen_fd;
  mbedtls_net_init (&listen_fd);
  
  // The SSL configuration for the lifetime of the server.
  // This is done during initialisation of mbed TLS.
  mbedtls_ssl_config conf;
  mbedtls_ssl_config_init (&conf);
  
  mbedtls_ssl_cache_context cache;
  mbedtls_ssl_cache_init (&cache);

  // A single entropy source that is used in all the threads.
  mbedtls_entropy_context entropy;
  mbedtls_entropy_init (&entropy);

  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_ctr_drbg_init (&ctr_drbg);

#if MBEDTLS_VERSION_MAJOR == 3
  const psa_status_t psa_status = psa_crypto_init();
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
  if (psa_status != PSA_SUCCESS) {
#pragma GCC diagnostic pop
    Database_Logs::log("Failure to run PSA crypto initialization: Not running the secure server");
    return;
  }
#endif

  // Load the private RSA server key.
  mbedtls_pk_context pkey;
  mbedtls_pk_init (&pkey);
  int ret =
#if MBEDTLS_VERSION_MAJOR == 2
  mbedtls_pk_parse_keyfile (&pkey, server_key_path.c_str (), nullptr);
#endif
#if MBEDTLS_VERSION_MAJOR == 3
  mbedtls_pk_parse_keyfile (&pkey, server_key_path.c_str (), nullptr, mbedtls_ctr_drbg_random, &ctr_drbg);
#endif
  if (ret != 0) {
    filter_url_display_mbed_tls_error (ret, nullptr, true, std::string());
    Database_Logs::log("Invalid " + server_key_path + " so not running secure server");
    return;
  }
  
  // Server certificates store.
  mbedtls_x509_crt srvcert;
  mbedtls_x509_crt_init (&srvcert);
  
  // Load the server certificate.
  ret = mbedtls_x509_crt_parse_file (&srvcert, server_certificate_path.c_str ());
  if (ret != 0) {
    filter_url_display_mbed_tls_error (ret, nullptr, true, std::string());
    Database_Logs::log("Invalid " + server_certificate_path + " so not running secure server");
    return;
  }

  // Load the chain of certificates of the certificate authorities.
  ret = mbedtls_x509_crt_parse_file (&srvcert, authorities_certificates_path.c_str ());
  if (ret != 0) {
    filter_url_display_mbed_tls_error (ret, nullptr, true, std::string());
    Database_Logs::log("Invalid " + authorities_certificates_path + " so not running secure server");
    return;
  }

  // Seed the random number generator.
  const char *pers = "Cloud";
  ret = mbedtls_ctr_drbg_seed (&ctr_drbg, mbedtls_entropy_func, &entropy, reinterpret_cast<const unsigned char *> (pers), strlen (pers));
  if (ret != 0) {
    filter_url_display_mbed_tls_error (ret, nullptr, true, std::string());
    return;
  }
  
  // Setup the listening TCP socket.
  ret = mbedtls_net_bind (&listen_fd, nullptr, network_port.c_str (), MBEDTLS_NET_PROTO_TCP);
  if (ret != 0) {
    filter_url_display_mbed_tls_error (ret, nullptr, true, std::string());
    return;
  }
  
  // Setup SSL/TLS default values for the lifetime of the https server.
  ret = mbedtls_ssl_config_defaults (&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0) {
    filter_url_display_mbed_tls_error (ret, nullptr, true, std::string());
    return;
  }
  mbedtls_ssl_conf_rng (&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
  mbedtls_ssl_conf_session_cache (&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
  mbedtls_ssl_conf_ca_chain (&conf, srvcert.next, nullptr);
  ret = mbedtls_ssl_conf_own_cert (&conf, &srvcert, &pkey);
  if (ret != 0) {
    filter_url_display_mbed_tls_error (ret, nullptr, true, std::string());
    return;
  }
  
  // Whether the plain http server redirects to secure http.
  // Possible automatic forwarding from http to https is postponed till here
  // because only here it is known whether the https server runs at all.
  // https://github.com/bibledit/cloud/issues/866
  config_globals_enforce_https_browser = config::logic::enforce_https_browser ();
  config_globals_enforce_https_client = config::logic::enforce_https_client ();

  std::cout << "Listening on https://localhost:" << network_port << std::endl;
  
  // Keep preparing for, accepting, and processing client connections.
  while (config_globals_webserver_running) {
    
    // Client connection file descriptor.
    mbedtls_net_context client_fd;
    mbedtls_net_init (&client_fd);
    
    // Wait until a client connects.
    ret = mbedtls_net_accept (&listen_fd, &client_fd, nullptr, 0, nullptr);
    if (ret != 0 ) {
      filter_url_display_mbed_tls_error (ret, nullptr, true, std::string());
      continue;
    }

    // Handle this request in a thread, enabling parallel requests.
    std::thread request_thread = std::thread (secure_webserver_process_request, &conf, client_fd);
    // Detach and delete thread object.
    request_thread.detach ();
  }
  
  // Wait shortly to give sufficient time to let the connection fail,
  // before the local SSL/TLS variables get out of scope,
  // which would lead to a segmentation fault if those variables were still in use.
  std::this_thread::sleep_for (std::chrono::milliseconds (5));

  // Close listening socket, freeing it for a possible subsequent server process.
  mbedtls_net_free (&listen_fd);
  
  // Shutting down mbed TLS.
  mbedtls_x509_crt_free (&srvcert);
  mbedtls_pk_free (&pkey);
  mbedtls_ssl_config_free (&conf);
  mbedtls_ssl_cache_free (&cache);
  mbedtls_ctr_drbg_free (&ctr_drbg);
  mbedtls_entropy_free (&entropy);
#if MBEDTLS_VERSION_MAJOR == 3
  mbedtls_psa_crypto_free();
#endif
  
#endif // ifdef RUN_SECURE_SERVER
}


/*

 Notes about the network port and a proxy.
 
 In case a client can only connect through port 80, 
 then this may proxy a certain folder to another port:
 http://serverfault.com/questions/472482/proxypass-redirect-directory-url-to-non-standard-port
 
 Or to write our own server in C acting as a proxy 
 to forward incoming requests to the bibledit instances on localhost.
 
 But since there are URLs requested that start with a /, 
 that may not work with the proxy.
 That needs a fix first.
 
*/
