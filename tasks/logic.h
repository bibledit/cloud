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
#include "enums.h"


namespace task {
constexpr auto rotate_journal {"rotatejournal"};
constexpr auto receive_email {"receiveemail"};
constexpr auto send_email {"sendemail"};
constexpr auto reindex_bibles {"reindexbibles"};
constexpr auto reindex_notes {"reindexnotes"};
constexpr auto create_css {"createcss"};
constexpr auto import_bible {"importusfm"};
constexpr auto import_resource {"importresource"};
constexpr auto compare_usfm {"compareusfm"};
constexpr auto maintain_database {"maintaindatabase"};
constexpr auto clean_tmp_files {"cleantmpfiles"};
constexpr auto link_git_repository {"linkgitrepository"};
constexpr auto send_receive_bibles {"sendreceivebibles"};
constexpr auto sync_bibles {"syncbibles"};
constexpr auto sync_notes {"syncnotes"};
constexpr auto sync_settings {"syncsettings"};
constexpr auto sync_changes {"syncchanges"};
constexpr auto sync_files {"syncfiles"};
constexpr auto sync_resources {"syncresources"};
constexpr auto clean_demo {"cleandemo"};
constexpr auto convert_bible_to_resource {"convertbible2resource"};
constexpr auto convert_resource_to_bible {"convertresource2bible"};
constexpr auto notes_statistics {"notesstatistics"};
constexpr auto generate_changes {"generatechanges"};
constexpr auto check_bible {"checkbible"};
constexpr auto export_all {"exportall"};
constexpr auto export_text_usfm {"exporttextusfm"};
constexpr auto export_usfm {"exportusfm"};
constexpr auto export_odt {"exportodt"};
constexpr auto export_info {"exportinfo"};
constexpr auto export_html {"exporthtml"};
constexpr auto export_web_main {"exportwebmain"};
constexpr auto export_web_index {"exportwebindex"};
constexpr auto export_online_bible {"exportonlinebible"};
constexpr auto export_esword {"exportesword"};
constexpr auto setup_paratext {"setupparatext"};
constexpr auto sync_paratext {"syncparatext"};
constexpr auto refresh_sword_modules {"refreshswordmodules"};
constexpr auto install_sword_module {"installswordmodule"};
constexpr auto update_sword_modules {"updateallswordmodules"};
constexpr auto list_usfm_resources {"listusfmresources"};
constexpr auto create_sample_bible {"createsamplebible"};
constexpr auto cache_resources {"cacheresources"};
constexpr auto refresh_web_resources {"refreshwebresources"};
constexpr auto produce_bibles_transferfile {"producebiblestransferfile"};
constexpr auto import_bibles_transferfile {"importbiblestransferfile"};
constexpr auto produce_notes_transferfile {"producenotestransferfile"};
constexpr auto import_notes_transferfile {"importnotestransferfile"};
constexpr auto produce_resources_transferfile {"produceresourcestransferfile"};
constexpr auto import_resources_transferfile {"importresourcestransferfile"};
constexpr auto delete_changes {"deletechanges"};
constexpr auto clear_caches {"clearcaches"};
constexpr auto trim_caches {"trimcaches"};
constexpr auto create_empty_bible {"createemptybible"};
constexpr auto import_bible_images {"importbibleimages"};
constexpr auto get_google_access_token {"getgoogleaccesstoken"};
}


std::string to_string (const tasks::enums::task& task);
std::string tasks_logic_folder ();
void tasks_logic_queue (std::string command, std::vector <std::string> parameters = {});
bool tasks_logic_queued (std::string command, std::vector <std::string> parameters = {});
void tasks_logic_queue (tasks::enums::task task, std::vector <std::string> parameters = {});
bool tasks_logic_queued (tasks::enums::task task, std::vector <std::string> parameters = {});
