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


#include <styles/new.h>
#include <styles/view.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <styles/sheets.h>
#include <assets/header.h>
#include "database/styles.h"


std::string styles_new_url()
{
    return "styles/new";
}


bool styles_new_acl(Webserver_Request& webserver_request)
{
    return roles::access_control(webserver_request, roles::translator);
}


std::string styles_new(Webserver_Request& webserver_request)
{
    Assets_Header header(translate("Stylesheet"), webserver_request);
    std::string page = header.run();

    Assets_View view;

    // The name of the stylesheet.
    const std::string name = webserver_request.query["name"];
    view.set_variable("name", filter::string::escape_special_xml_characters(name));

    // Whether this user has write access to the stylesheet.
    const std::string& username = webserver_request.session_logic()->get_username();
    const int userlevel = webserver_request.session_logic()->get_level();
    bool write = database::styles::has_write_access(username, name);
    if (userlevel >= roles::admin) write = true;

    // Allowed characters in the style.
    constexpr std::string_view allowed{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz123456789-"};

    // Handle new style submission.
    if (webserver_request.post_count("style"))
    {
        const std::string new_style = webserver_request.post_get("style");
        const std::string base_style = webserver_request.post_get("base");
        const std::vector<std::string> markers = database::styles::get_markers(name);
        if (new_style.empty())
        {
            page.append(assets_page::error(translate("Enter a name for the new style")));
        }
        else if (std::ranges::any_of(new_style, [&allowed](const char c)
        {
            return allowed.find(c) == std::string::npos;
        }))
            page.append(
                assets_page::error(translate("Allowed characters for the style are:") + " " + std::string(allowed)));
        else if (base_style.empty())
        {
            page.append(assets_page::error(translate("Select an existing style to base the new style on")));
        }
        else if (filter::string::in_array(new_style, markers))
        {
            page.append(assets_page::error(translate("The style already exists in the stylesheet")));
        }
        else if (!write)
        {
            page.append(
                assets_page::error(translate("You don't have sufficient privileges to add a style to the stylesheet")));
        }
        else
        {
            // Create the marker.
            database::styles::add_marker(name, new_style, base_style);
            // Recreate all stylesheets.
            styles::sheets::create_all();
            // Redirect to the page editing this style.
            const std::string location = filter_url_build_http_query(styles_view_url(), {
                                                                         {"sheet", name},
                                                                         {"style", new_style},
                                                                     });
            redirect_browser(webserver_request, location);
        }
    }

    // Generate the option HTML for all possible base styles and set it on the page.
    {
        std::vector markers{database::styles::get_markers(stylesv2::standard_sheet())};
        markers.emplace_back(std::string());
        std::ranges::sort(markers);
        for (const auto& marker : markers)
            view.add_iteration("option", {std::pair("style", marker)});
    }

    page += view.render("styles", "new");

    page += assets_page::footer();

    return page;
}
