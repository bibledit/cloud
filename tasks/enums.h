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

namespace tasks::enums {

enum class task
{
    none,
    rotate_journal,
    receive_email,
    send_email,
    reindex_bibles,
    reindex_notes,
    create_css,
    import_bible,
    import_resource,
    compare_usfm,
    maintain_database,
    clean_tmp_files,
    link_git_repository,
    send_receive_bibles,
    sync_bibles,
    sync_notes,
    sync_settings,
    sync_changes,
    sync_files,
    sync_resources,
    clean_demo,
    convert_bible_to_resource,
    convert_resource_to_bible,
    notes_statistics,
    generate_changes,
    check_bible,
    export_all,
    export_text_usfm,
    export_usfm,
    export_odt,
    export_info,
    export_html,
    export_web_main,
    export_web_index,
    export_online_bible,
    export_esword,
    setup_paratext,
    sync_paratext,
    refresh_sword_modules,
    install_sword_module,
    update_sword_modules,
    list_usfm_resources,
    create_sample_bible,
    cache_resources,
    refresh_web_resources,
    produce_bibles_transferfile,
    import_bibles_transferfile,
    produce_notes_transferfile,
    import_notes_transferfile,
    produce_resources_transferfile,
    import_resources_transferfile,
    delete_changes,
    clear_caches,
    trim_caches,
    create_empty_bible,
    import_bible_images,
    get_google_access_token,
};

enum class paratext_sync {
  none,
  bi_directional,
  bibledit_to_paratext,
  paratext_to_bibledit
};

}

