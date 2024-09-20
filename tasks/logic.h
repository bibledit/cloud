/*
Copyright (©) 2003-2024 Teus Benschop.

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


#pragma once

#include <config/libraries.h>


namespace task {
constexpr const auto rotatejournal {"rotatejournal"};
constexpr const auto receiveemail {"receiveemail"};
constexpr const auto sendemail {"sendemail"};
constexpr const auto reindexbibles {"reindexbibles"};
constexpr const auto reindexnotes {"reindexnotes"};
constexpr const auto createcss {"createcss"};
constexpr const auto importbible {"importusfm"};
constexpr const auto importresource {"importresource"};
constexpr const auto compareusfm {"compareusfm"};
constexpr const auto maintaindatabase {"maintaindatabase"};
constexpr const auto cleantmpfiles {"cleantmpfiles"};
constexpr const auto linkgitrepository {"linkgitrepository"};
constexpr const auto sendreceivebibles {"sendreceivebibles"};
constexpr const auto syncbibles {"syncbibles"};
constexpr const auto syncnotes {"syncnotes"};
constexpr const auto syncsettings {"syncsettings"};
constexpr const auto syncchanges {"syncchanges"};
constexpr const auto syncfiles {"syncfiles"};
constexpr const auto syncresources {"syncresources"};
constexpr const auto cleandemo {"cleandemo"};
constexpr const auto convertbible2resource {"convertbible2resource"};
constexpr const auto convertresource2bible {"convertresource2bible"};
constexpr const auto printresources {"printresources"};
constexpr const auto notesstatistics {"notesstatistics"};
constexpr const auto generatechanges {"generatechanges"};
constexpr const auto sprintburndown {"sprintburndown"};
constexpr const auto checkbible {"checkbible"};
constexpr const auto exportall {"exportall"};
constexpr const auto exporttextusfm {"exporttextusfm"};
constexpr const auto exportusfm {"exportusfm"};
constexpr const auto exportodt {"exportodt"};
constexpr const auto exportinfo {"exportinfo"};
constexpr const auto exporthtml {"exporthtml"};
constexpr const auto exportwebmain {"exportwebmain"};
constexpr const auto exportwebindex {"exportwebindex"};
constexpr const auto exportonlinebible {"exportonlinebible"};
constexpr const auto exportesword {"exportesword"};
constexpr const auto hyphenate {"hyphenate"};
constexpr const auto setupparatext {"setupparatext"};
constexpr const auto syncparatext {"syncparatext"};
constexpr const auto importimages {"importimages"};
constexpr const auto refreshswordmodules {"refreshswordmodules"};
constexpr const auto installswordmodule {"installswordmodule"};
constexpr const auto updateswordmodules {"updateallswordmodules"};
constexpr const auto listusfmresources {"listusfmresources"};
constexpr const auto createsamplebible {"createsamplebible"};
constexpr const auto cacheresources {"cacheresources"};
constexpr const auto refreshwebresources {"refreshwebresources"};
constexpr const auto rssfeedupdatechapter {"rssfeedupdatechapter"};
constexpr const auto producebiblestransferfile {"producebiblestransferfile"};
constexpr const auto importbiblestransferfile {"importbiblestransferfile"};
constexpr const auto producenotestransferfile {"producenotestransferfile"};
constexpr const auto importnotestransferfile {"importnotestransferfile"};
constexpr const auto produceresourcestransferfile {"produceresourcestransferfile"};
constexpr const auto importresourcestransferfile {"importresourcestransferfile"};
constexpr const auto deletechanges {"deletechanges"};
constexpr const auto clearcaches {"clearcaches"};
constexpr const auto trimcaches {"trimcaches"};
constexpr const auto export2nmt {"export2nmt"};
constexpr const auto createemptybible {"createemptybible"};
constexpr const auto importbibleimages {"importbibleimages"};
constexpr const auto getgoogleaccesstoken {"getgoogleaccesstoken"};
}


std::string tasks_logic_folder ();
void tasks_logic_queue (std::string command, std::vector <std::string> parameters = { });
bool tasks_logic_queued (std::string command, std::vector <std::string> parameters = { });

