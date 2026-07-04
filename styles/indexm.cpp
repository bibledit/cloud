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


#include <access/user.h>
#include <assets/header.h>
#include <assets/page.h>
#include <assets/view.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <locale/translate.h>
#include <styles/indexm.h>
#include <styles/logic.h>
#include <styles/sheets.h>
#include <webserver/request.h>
#include "database/styles.h"


std::string styles_indexm_url()
{
    return "styles/indexm";
}


bool styles_indexm_acl(Webserver_Request& webserver_request)
{
    return roles::access_control(webserver_request, roles::translator);
}


std::string styles_indexm(Webserver_Request& webserver_request)
{
    Assets_Header header(translate("Styles"), webserver_request);
    std::string page = header.run();

    Assets_View view{};

    const std::string& username{webserver_request.session_logic()->get_username()};
    int user_level{webserver_request.session_logic()->get_level()};

    if (webserver_request.post_count("new"))
    {
        std::string name{webserver_request.post_get("entry")};
        // Remove spaces at the ends of the name for the new stylesheet.
        // Because predictive keyboards can add a space to the name,
        // and the stylesheet system is not built for whitespace at the start / end of the name of the stylesheet.
        name = filter::string::trim(name);
        if (std::vector existing{database::styles::get_sheets()};
            std::ranges::find(existing, name) != existing.end())
        {
            page += assets_page::error(translate("This stylesheet already exists"));
        }
        else
        {
            database::styles::create_sheet(name);
            database::styles::grant_write_access(username, name);
            styles_sheets_create_all();
            page += assets_page::success(translate("The stylesheet has been created"));
        }
    }
    if (webserver_request.query.contains("new"))
    {
        Dialog_Entry dialog_entry("indexm", translate("Please enter the name for the new stylesheet"),
                                  std::string(), "new", std::string());
        page += dialog_entry.run();
        return page;
    }

    if (webserver_request.query.contains("delete"))
    {
        if (const std::string del{webserver_request.query["delete"]}; not del.empty())
        {
            const std::string confirm{webserver_request.query["confirm"]};
            if (confirm == "yes")
            {
                bool write = database::styles::has_write_access(username, del);
                if (user_level >= roles::admin) write = true;
                if (write)
                {
                    database::styles::delete_sheet(del);
                    database::styles::revoke_write_access(std::string(), del);
                    page += assets_page::success(translate("The stylesheet has been deleted"));
                }
            }
            if (confirm.empty())
            {
                Dialog_Yes dialog_yes("indexm", translate("Would you like to delete this stylesheet?"));
                dialog_yes.add_query("delete", del);
                page += dialog_yes.run();
                return page;
            }
        }
    }

    // Delete empty sheet that may have been there.
    database::styles::delete_sheet(std::string());

    std::vector<std::string> sheets = database::styles::get_sheets();
    std::stringstream sheet_block{};
    std::ranges::for_each(sheets, [&sheet_block, user_level, &username](const auto& sheet)
    {
        sheet_block << "<p>";
        sheet_block << sheet;
        bool editable = database::styles::has_write_access(username, sheet);
        if (user_level >= roles::admin) editable = true;
        // Cannot edit the Standard stylesheet.
        if (sheet == stylesv2::standard_sheet()) editable = false;
        if (editable)
            sheet_block << "<a href=" << std::quoted("sheetm?name=" + sheet) << ">[" << translate("edit") << "]</a>";
        sheet_block << "</p>";
    });

    view.set_variable("sheetblock", sheet_block.str());

    page += view.render("styles", "indexm");

    page += assets_page::footer();

    return page;
}
