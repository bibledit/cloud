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


#ifndef INCLUDED_TASKS_LOGIC
#define INCLUDED_TASKS_LOGIC


#include <config/libraries.h>


#define ROTATEJOURNAL "rotatejournal"
#define RECEIVEEMAIL "receiveemail"
#define SENDEMAIL "sendemail"
#define REINDEXBIBLES "reindexbibles"
#define REINDEXNOTES "reindexnotes"
#define CREATECSS "createcss"
#define IMPORTBIBLE "importusfm"
#define IMPORTRESOURCE "importresource"
#define COMPAREUSFM "compareusfm"
#define MAINTAINDATABASE "maintaindatabase"
#define CLEANTMPFILES "cleantmpfiles"
#define LINKGITREPOSITORY "linkgitrepository"
#define SENDRECEIVEBIBLES "sendreceivebibles"
#define SYNCBIBLES "syncbibles"
#define SYNCNOTES "syncnotes"
#define SYNCSETTINGS "syncsettings"
#define SYNCCHANGES "syncchanges"
#define SYNCFILES "syncfiles"
#define SYNCRESOURCES "syncresources"
#define CLEANDEMO "cleandemo"
#define CONVERTBIBLE2RESOURCE "convertbible2resource"
#define CONVERTRESOURCE2BIBLE "convertresource2bible"
#define PRINTRESOURCES "printresources"
#define NOTESSTATISTICS "notesstatistics"
#define GENERATECHANGES "generatechanges"
#define SPRINTBURNDOWN "sprintburndown"
#define CHECKBIBLE "checkbible"
#define EXPORTALL "exportall"
#define EXPORTTEXTUSFM "exporttextusfm"
#define EXPORTUSFM "exportusfm"
#define EXPORTODT "exportodt"
#define EXPORTINFO "exportinfo"
#define EXPORTHTML "exporthtml"
#define EXPORTWEBMAIN "exportwebmain"
#define EXPORTWEBINDEX "exportwebindex"
#define EXPORTONLINEBIBLE "exportonlinebible"
#define EXPORTESWORD "exportesword"
#define EXPORTQUICKBIBLE "exportquickbible"
#define HYPHENATE "hyphenate"
#define SETUPPARATEXT "setupparatext"
#define SYNCPARATEXT "syncparatext"
#define SUBMITBIBLEDROPBOX "submitbibledropbox"
#define IMPORTIMAGES "importimages"
#define REFRESHSWORDMODULES "refreshswordmodules"
#define INSTALLSWORDMODULE "installswordmodule"
#define UPDATESWORDMODULES "updateallswordmodules"
#define LISTUSFMRESOURCES "listusfmresources"
#define CREATESAMPLEBIBLE "createsamplebible"
#define CACHERESOURCES "cacheresources"
#define NOTIFYSOFTWAREUPDATES "notifysoftwareupdates"
#define REFRESHWEBRESOURCES "refreshwebresources"
#define RSSFEEDUPDATECHAPTER "rssfeedupdatechapter"


string tasks_logic_folder ();
void tasks_logic_queue (string command, vector <string> parameters = { });
bool tasks_logic_queued (string command, vector <string> parameters = { });


#endif
