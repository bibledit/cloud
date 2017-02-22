/*
 Copyright (©) 2003-2016 Teus Benschop.
 
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


#ifndef INCLUDED_CLIENT_LOGIC_H
#define INCLUDED_CLIENT_LOGIC_H


#include <config/libraries.h>


bool client_logic_client_enabled ();
void client_logic_enable_client (bool enable);
string client_logic_url (string address, int port, string path);
string client_logic_connection_setup (string user, string hash);
string client_logic_create_note_encode (string bible, int book, int chapter, int verse,
                                        string summary, string contents, bool raw);
void client_logic_create_note_decode (string data,
                                      string& bible, int& book, int& chapter, int& verse,
                                      string& summary, string& contents, bool& raw);
string client_logic_link_to_cloud (string path, string linktext);
string client_logic_usfm_resources_path ();
void client_logic_usfm_resources_update ();
vector <string> client_logic_usfm_resources_get ();


#endif
