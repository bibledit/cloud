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


#pragma once

#include <config/libraries.h>


namespace task {
constexpr const auto rotate_journal {"rotatejournal"};
constexpr const auto receive_email {"receiveemail"};
constexpr const auto send_email {"sendemail"};
constexpr const auto reindex_bibles {"reindexbibles"};
constexpr const auto reindex_notes {"reindexnotes"};
constexpr const auto create_css {"createcss"};
constexpr const auto import_bible {"importusfm"};
constexpr const auto import_resource {"importresource"};
constexpr const auto compare_usfm {"compareusfm"};
constexpr const auto maintain_database {"maintaindatabase"};
constexpr const auto clean_tmp_files {"cleantmpfiles"};
constexpr const auto link_git_repository {"linkgitrepository"};
constexpr const auto send_receive_bibles {"sendreceivebibles"};
constexpr const auto sync_bibles {"syncbibles"};
constexpr const auto sync_notes {"syncnotes"};
constexpr const auto sync_settings {"syncsettings"};
constexpr const auto sync_changes {"syncchanges"};
constexpr const auto sync_files {"syncfiles"};
constexpr const auto sync_resources {"syncresources"};
constexpr const auto clean_demo {"cleandemo"};
constexpr const auto convert_bible_to_resource {"convertbible2resource"};
constexpr const auto convert_resource_to_bible {"convertresource2bible"};
constexpr const auto notes_statistics {"notesstatistics"};
constexpr const auto generate_changes {"generatechanges"};
constexpr const auto sprint_burndown {"sprintburndown"};
constexpr const auto check_bible {"checkbible"};
constexpr const auto export_all {"exportall"};
constexpr const auto export_text_usfm {"exporttextusfm"};
constexpr const auto export_usfm {"exportusfm"};
constexpr const auto export_odt {"exportodt"};
constexpr const auto export_info {"exportinfo"};
constexpr const auto export_html {"exporthtml"};
constexpr const auto export_web_main {"exportwebmain"};
constexpr const auto export_web_index {"exportwebindex"};
constexpr const auto export_online_bible {"exportonlinebible"};
constexpr const auto export_esword {"exportesword"};
constexpr const auto setup_paratext {"setupparatext"};
constexpr const auto sync_paratext {"syncparatext"};
constexpr const auto refresh_sword_modules {"refreshswordmodules"};
constexpr const auto install_sword_module {"installswordmodule"};
constexpr const auto update_sword_modules {"updateallswordmodules"};
constexpr const auto list_usfm_resources {"listusfmresources"};
constexpr const auto create_sample_bible {"createsamplebible"};
constexpr const auto cache_resources {"cacheresources"};
constexpr const auto refresh_web_resources {"refreshwebresources"};
constexpr const auto rss_feed_update_chapter {"rssfeedupdatechapter"};
constexpr const auto produce_bibles_transferfile {"producebiblestransferfile"};
constexpr const auto import_bibles_transferfile {"importbiblestransferfile"};
constexpr const auto produce_notes_transferfile {"producenotestransferfile"};
constexpr const auto import_notes_transferfile {"importnotestransferfile"};
constexpr const auto produce_resources_transferfile {"produceresourcestransferfile"};
constexpr const auto import_resources_transferfile {"importresourcestransferfile"};
constexpr const auto delete_changes {"deletechanges"};
constexpr const auto clear_caches {"clearcaches"};
constexpr const auto trim_caches {"trimcaches"};
constexpr const auto create_empty_bible {"createemptybible"};
constexpr const auto import_bible_images {"importbibleimages"};
constexpr const auto get_google_access_token {"getgoogleaccesstoken"};
}


std::string tasks_logic_folder ();
void tasks_logic_queue (std::string command, std::vector <std::string> parameters = { });
bool tasks_logic_queued (std::string command, std::vector <std::string> parameters = { });

