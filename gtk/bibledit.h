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
** Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**  
*/


#ifndef INCLUDED_BIBLEDIT_H
#define INCLUDED_BIBLEDIT_H


#include <cstdlib>
#include <gtk/gtk.h>


using namespace std;


int main (int argc, char *argv[]);
static void activate (GtkApplication *app);
GtkApplication *application;
GtkWidget * window;
static void on_signal_destroy (gpointer user_data);
gint status;
static gboolean on_key_press (GtkWidget *widget, GdkEvent *event, gpointer data);
static void on_window_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gboolean on_window_state_event (GtkWidget *widget, GdkEventWindowState *event);
static gboolean on_timeout (gpointer data);
static gboolean on_configure (GtkWidget *widget, GdkEvent *event, gpointer user_data);
void webkit_search (GtkWidget *widget);
void timer_thread ();


#endif
