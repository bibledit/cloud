/*
** Copyright (©) 2003-2026 Teus Benschop.
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**  
*/


#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wredundant-tags"
#include <config.h>
#include <executable/client.h>
#include <libgen.h>
#include <iostream>
#include <thread>
#include "library/bibledit.h"
#pragma GCC diagnostic push
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#pragma GCC diagnostic pop

gint bibledit_window_root_x = 0;
gint bibledit_window_root_y = 0;
gint bibledit_window_width = 0;
gint bibledit_window_height = 0;
gboolean bibledit_window_maximized = false;
gboolean bibledit_window_fullscreen = false;
const char * bibledit_window_state_ini = "state.ini";
const char * bibledit_window_state = "WindowState";
const char * bibledit_window_root_x_key = "RootX";
const char * bibledit_window_root_y_key = "RootY";
const char * bibledit_window_width_key = "Width";
const char * bibledit_window_height_key = "Height";
const char * bibledit_window_maximized_key = "Maximized";
const char * bibledit_window_fullscreen_key = "Fullscreen";
bool bibledit_window_configured = false;
std::string port;


// Function declarations.
static gboolean on_decide_policy (WebKitWebView *web_view, WebKitPolicyDecision *decision, WebKitPolicyDecisionType decision_type, gpointer user_data);
static void on_download_started (WebKitWebContext *context, WebKitDownload *download, gpointer user_data);
static gboolean on_decide_destination (WebKitDownload * download, gchar * suggested_filename, gpointer user_data);
static void on_download_finished (WebKitDownload *download, gpointer user_data);


int main (int argc, char *argv[])
{
    application = gtk_application_new("org.bibledit.linux", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(application, "activate", G_CALLBACK(activate), nullptr);

    port = bibledit_get_network_port();

    // Derive the webroot from the $HOME environment variable.
    std::string webroot;
    const char* home = g_getenv("HOME");
    if (home) webroot = home;
    if (!webroot.empty()) webroot.append("/");
    webroot.append("bibledit");

    // Read the package directory from config.h.

    // The $make install will copy the relevant files to /usr/share/bibledit.
    // That is the package data directory.
    // Bibledit will then copy this to the webroot upon first run for a certain version number.
    bibledit_initialize_library(PACKAGE_DATA_DIR, webroot.c_str());
    // In case the app is installed from source, through the "install.sh" script,
    // on first run it will try to copy the package data from /usr/share/bibledit,
    // find nothing there, and continue normally.
    // This will work when the app is unpacked in folder ~/bibledit.

    bibledit_start_library();

    new std::thread(timer_thread);

    status = g_application_run(G_APPLICATION(application), argc, argv);

    g_object_unref(application);

    bibledit_stop_library();

    while (bibledit_is_running())
    {
    };

    bibledit_shutdown_library();

    return status;
}


void activate (GtkApplication *app)
{
    GList* list = gtk_application_get_windows(app);

    if (list)
    {
        // Activate existing live app.
        gtk_window_present(GTK_WINDOW(list->data));
        return;
    }

    // The top-level window.
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Bibledit");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_application(GTK_WINDOW(window), application);

    // The icon will be loaded from folder "pix" in the current directory,
    // or if it's not there, from /usr/share/bibledit.
    {
        const char* pix {"pix"};
        const char* xpm {"bbe48x48.xpm"};
        gchar* icon_file = g_build_filename(pix, xpm, nullptr);
        if (!g_file_test(icon_file, G_FILE_TEST_EXISTS))
            icon_file = g_build_filename(PACKAGE_DATA_DIR, pix, xpm, nullptr);
        gtk_window_set_default_icon_from_file(icon_file, nullptr);
        g_free(icon_file);
    }

    // Create a browser instance.
    WebKitWebView* webview = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Put the browser area into the main window.
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(webview));

    // Start with a cleared web cache.
    WebKitWebContext* context = webkit_web_context_get_default();
    webkit_web_context_clear_cache(context);

    // Load a web page into the browser instance.
    // The server's port number of this client is intentionally different from the Cloud's server port.
    // This way both can run on one system simultaneously.
    {
        const std::string url = "http://localhost:" + port;
        webkit_web_view_load_uri(webview, url.c_str());
    }

    // Ensure it will get mouse and keyboard events.
    gtk_widget_grab_focus(GTK_WIDGET(webview));

    // Get window state and size.
    const char* appid = g_application_get_application_id(g_application_get_default());
    char* file = g_build_filename(g_get_user_cache_dir(), appid, bibledit_window_state_ini, nullptr);
    GKeyFile* keyfile = g_key_file_new();
    bool geometry_loaded = g_key_file_load_from_file(keyfile, file, G_KEY_FILE_NONE, nullptr);
    if (geometry_loaded)
    {
        bibledit_window_root_x = g_key_file_get_integer(keyfile, bibledit_window_state, bibledit_window_root_x_key,
                                                        nullptr);
        bibledit_window_root_y = g_key_file_get_integer(keyfile, bibledit_window_state, bibledit_window_root_y_key,
                                                        nullptr);
        bibledit_window_width = g_key_file_get_integer(keyfile, bibledit_window_state, bibledit_window_width_key, nullptr);
        bibledit_window_height = g_key_file_get_integer(keyfile, bibledit_window_state, bibledit_window_height_key,
                                                        nullptr);
        bibledit_window_maximized = g_key_file_get_boolean(keyfile, bibledit_window_state,
                                                           bibledit_window_maximized_key, nullptr);
        bibledit_window_fullscreen = g_key_file_get_boolean(keyfile, bibledit_window_state,
                                                            bibledit_window_fullscreen_key, nullptr);
    }
    g_key_file_unref(keyfile);
    g_free(file);

    // Signal handlers.
    g_signal_connect(window, "configure-event", G_CALLBACK(on_configure), nullptr);
    g_signal_connect(window, "size-allocate", G_CALLBACK(on_window_size_allocate), nullptr);
    g_signal_connect(window, "window-state-event", G_CALLBACK(on_window_state_event), nullptr);
    g_signal_connect(window, "destroy", G_CALLBACK(on_signal_destroy), nullptr);
    g_signal_connect(webview, "key-press-event", G_CALLBACK(on_key_press), nullptr);
    g_signal_connect(context, "download-started", G_CALLBACK(on_download_started), nullptr);
    g_signal_connect(webview, "decide-policy", G_CALLBACK(on_decide_policy), nullptr);

    // Set window size and state before it shows.
    if (geometry_loaded)
    {
        gtk_window_set_default_size(GTK_WINDOW(window), bibledit_window_width, bibledit_window_height);
        if (bibledit_window_maximized) gtk_window_maximize(GTK_WINDOW(window));
        if (bibledit_window_fullscreen) gtk_window_fullscreen(GTK_WINDOW(window));
    }

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(window);

    // Move window to desired position after it shows.
    // Do that after a delay so the window gets the chance to settle.
    if (geometry_loaded)
    {
        g_timeout_add(300, GSourceFunc(on_timeout), nullptr);
    }
    else
    {
        bibledit_window_configured = true;
    }

    // Run the main GTK+ event loop.
    gtk_main();
}


void on_signal_destroy ([[maybe_unused]] gpointer user_data)
{
    // Save the window states.

    GKeyFile* keyfile = g_key_file_new();

    g_key_file_set_integer(keyfile, bibledit_window_state, bibledit_window_root_x_key, bibledit_window_root_x);
    g_key_file_set_integer(keyfile, bibledit_window_state, bibledit_window_root_y_key, bibledit_window_root_y);
    g_key_file_set_integer(keyfile, bibledit_window_state, bibledit_window_width_key, bibledit_window_width);
    g_key_file_set_integer(keyfile, bibledit_window_state, bibledit_window_height_key, bibledit_window_height);
    g_key_file_set_boolean(keyfile, bibledit_window_state, bibledit_window_maximized_key, bibledit_window_maximized);
    g_key_file_set_boolean(keyfile, bibledit_window_state, bibledit_window_fullscreen_key, bibledit_window_fullscreen);

    const char* appid = g_application_get_application_id(g_application_get_default());
    char* path = g_build_filename(g_get_user_cache_dir(), appid, nullptr);
    g_mkdir_with_parents(path, 0700);
    char* file = g_build_filename(path, bibledit_window_state_ini, nullptr);
    g_key_file_save_to_file(keyfile, file, nullptr);

    g_free(file);
    g_key_file_unref(keyfile);
    g_free(path);

    // Quit program.
    gtk_main_quit();
}


// Handle key presses.
gboolean on_key_press (GtkWidget *widget, GdkEvent *event, [[maybe_unused]] gpointer data)
{
    if (event->type == GDK_KEY_PRESS)
    {
        GdkEventKey* event_key = (GdkEventKey*)event;
        // The web view does not handle the keys for undo and redo of its own.
        // Handle them here.
        if ((event_key->keyval == GDK_KEY_z) || (event_key->keyval == GDK_KEY_Z))
        {
            if (event_key->state & GDK_CONTROL_MASK)
            {
                const gchar* command;
                if (event_key->state & GDK_SHIFT_MASK)
                    command = WEBKIT_EDITING_COMMAND_REDO;
                else
                    command = WEBKIT_EDITING_COMMAND_UNDO;
                WebKitWebView* web_view = WEBKIT_WEB_VIEW(widget);
                webkit_web_view_execute_editing_command(web_view, command);
                // Key press handled.
                return true;
            }
        }
        // Handle Ctrl-P for printing.
        if ((event_key->keyval == GDK_KEY_p) || (event_key->keyval == GDK_KEY_P))
        {
            if (event_key->state & GDK_CONTROL_MASK)
            {
                WebKitWebView* web_view = WEBKIT_WEB_VIEW(widget);
                WebKitPrintOperation* print_operation = webkit_print_operation_new(web_view);
                webkit_print_operation_run_dialog(print_operation, GTK_WINDOW(window));
                return true;
            }
        }
        // Handle Ctrl-F for searching.
        if ((event_key->keyval == GDK_KEY_f) || (event_key->keyval == GDK_KEY_F))
        {
            if (event_key->state & GDK_CONTROL_MASK)
            {
                webkit_search(widget);
                return true;
            }
        }
    }
    // Key press not handled.
    return false;
}


void on_window_size_allocate (GtkWidget *widget, [[maybe_unused]] GtkAllocation *allocation)
{
    // Save the window geometry only if the window is not maximized or fullscreen.
    if (not(bibledit_window_maximized or bibledit_window_fullscreen))
        gtk_window_get_size(GTK_WINDOW(widget), &bibledit_window_width, &bibledit_window_height);
}


gboolean on_window_state_event ([[maybe_unused]] GtkWidget *widget, GdkEventWindowState *event)
{
    bibledit_window_maximized = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
    bibledit_window_fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;
    return true;
}


static gboolean on_timeout ([[maybe_unused]] gpointer data)
{
    if (bibledit_window_root_x and bibledit_window_root_y)
    {
        gtk_window_move(GTK_WINDOW(window), bibledit_window_root_x, bibledit_window_root_y);
        bibledit_window_configured = true;
    }
    return false;
}


static gboolean on_configure (GtkWidget *widget, [[maybe_unused]] GdkEvent *event, [[maybe_unused]] gpointer user_data)
{
    // Save the window position only if the window is not maximized or fullscreen.
    if (bibledit_window_configured and not(bibledit_window_maximized or bibledit_window_fullscreen))
        gtk_window_get_position(GTK_WINDOW(widget), &bibledit_window_root_x, &bibledit_window_root_y);
    return false;
}


static gboolean on_decide_policy (
    [[maybe_unused]] WebKitWebView *web_view,
    WebKitPolicyDecision *decision,
    WebKitPolicyDecisionType decision_type,
    [[maybe_unused]] gpointer user_data)
{
    // Handle the type of decision that may lead to file download.
    if (decision_type == WEBKIT_POLICY_DECISION_TYPE_RESPONSE)
    {
        WebKitResponsePolicyDecision* response_decision = WEBKIT_RESPONSE_POLICY_DECISION(decision);
        //WebKitURIRequest * uri_request = webkit_response_policy_decision_get_request (response_decision);
        //const gchar * uri = webkit_uri_request_get_uri (uri_request);
        // Check whether the webkit can display the mime type.
        gboolean mime_supported = webkit_response_policy_decision_is_mime_type_supported(response_decision);
        if (!mime_supported)
        {
            // Cannot display the mime type: Download the file.
            webkit_policy_decision_download(decision);
            return TRUE;
        }
    }
    // Making no decision results in webkit_policy_decision_use().
    return FALSE;
}


static void on_download_started ([[maybe_unused]] WebKitWebContext *context, WebKitDownload *download, [[maybe_unused]] gpointer user_data)
{
    // Listen for decide destination
    g_signal_connect(download, "decide-destination", G_CALLBACK(on_decide_destination), nullptr);
    // Listen for download finished.
    g_signal_connect(download, "finished", G_CALLBACK(on_download_finished), nullptr);
}


static gboolean on_decide_destination (
    [[maybe_unused]] WebKitDownload * download,
    gchar * suggested_filename,
    [[maybe_unused]] gpointer user_data)
{
    // There is something weird going on when downloading a file.
    // When downloading for example the notes.tar, it works well for the first time.
    // The downloaded notes.tar will be saved in the Download directory of the user.
    // While this notes.tar is there, then downloading the same file again, something weird happens:
    // It will delete the previously downloaded notes.tar, and leave it at that.
    // The result is that the user finds nothing downloaded.
    // To work around this, it here first removes any notes.tar that might have been there.
    // Then downloading it, it works well.
    gchar* path = g_build_filename(g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD), suggested_filename, nullptr);
    unlink(path);
    g_free(path);
    return false;
}


static void on_download_finished (WebKitDownload *download, [[maybe_unused]] gpointer user_data)
{
    // After download complete, open the folder.
    // Before it used to open the saved file.
    // But that did not always work well, depending on the OS version.
    // There have been times that the OS failed to open the tarball.
    // So now: Open the folder.
    const gchar* destination = webkit_download_get_destination(download);
    gchar* folder = g_path_get_dirname(destination);
    std::string command = "xdg-open \"";
    command.append(folder);
    command.append("\"");
    [[maybe_unused]] const int result = system(command.c_str());
    g_free(folder);
}


void webkit_search (GtkWidget *widget)
{
    GtkDialogFlags flags = GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
    GtkWidget* dialog = gtk_dialog_new_with_buttons("Search", GTK_WINDOW(window), flags,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                                    nullptr);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    GtkWidget* dialog_vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget* entry = gtk_entry_new();
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(dialog_vbox), entry, TRUE, TRUE, 4);
    gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
    gtk_widget_grab_focus(entry);

    GtkWidget* okbutton = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_widget_grab_default(okbutton);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK)
    {
        WebKitWebView* web_view = WEBKIT_WEB_VIEW(widget);
        WebKitFindController* find_controller = webkit_web_view_get_find_controller(web_view);
        // Finish possible previous search.
        webkit_find_controller_search_finish(find_controller);
        // The text to search for.
        std::string search_text = gtk_entry_get_text(GTK_ENTRY(entry));
        if (search_text.empty())
        {
            // Remove all highlights and unselect text by doing this:
            // Search for a word that does not normally occur.
            search_text = "un_search";
        }
        // Search for the text and highlight all hits.
        WebKitFindOptions find_options = WebKitFindOptions(
            WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE | WEBKIT_FIND_OPTIONS_WRAP_AROUND);
        webkit_find_controller_search(find_controller, search_text.c_str(), find_options, G_MAXUINT);
    }

    gtk_widget_destroy(dialog);
}


// Start an external URL through the default system browser.
[[noreturn]] void timer_thread ()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        const std::string url = bibledit_get_external_url();
        if (not url.empty())
        {
            g_app_info_launch_default_for_uri(url.c_str(), nullptr, nullptr);
        }
    }
}
