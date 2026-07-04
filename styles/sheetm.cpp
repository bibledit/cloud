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


#include <styles/sheetm.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <assets/header.h>
#include <database/logic.h>
#include "database/styles.h"
#include <pugixml/include.h>


std::string styles_sheetm_url()
{
    return "styles/sheetm";
}


bool styles_sheetm_acl(Webserver_Request& webserver_request)
{
    return roles::access_control(webserver_request, roles::translator);
}


std::string styles_sheetm(Webserver_Request& webserver_request)
{
    Assets_Header header(translate("Stylesheet"), webserver_request);
    std::string page = header.run();

    Assets_View view;

    // The name of the stylesheet.
    const std::string sheet_name = webserver_request.query["name"];
    view.set_variable("name", filter::string::escape_special_xml_characters(sheet_name));

    // Whether this user has write access to the stylesheet.
    const std::string& username = webserver_request.session_logic()->get_username();
    const int user_level = webserver_request.session_logic()->get_level();
    bool write = database::styles::has_write_access(username, sheet_name);
    if (user_level >= roles::admin) write = true;

    if (const std::string del = webserver_request.query["delete"]; not del.empty())
        if (write)
        {
            database::styles::delete_marker(sheet_name, del);
        }

    pugi::xml_document html_block{};

    // List the styles v2 in the overview.
    {
        const auto get_and_sort_markers_v2 = [](const auto& name)
        {
            std::vector<std::string> markers{database::styles::get_markers(name)};
            std::vector<std::string> sorted_markers{};
            for (const stylesv2::Style& style : stylesv2::styles)
            {
                const std::string& marker = style.marker;
                if (const auto iter = std::ranges::find(std::as_const(markers), marker); iter != markers.cend())
                {
                    sorted_markers.push_back(marker);
                    markers.erase(iter);
                }
            }
            sorted_markers.insert(sorted_markers.cend(), markers.cbegin(), markers.cend());
            return sorted_markers;
        };
        const std::vector markers_v2{get_and_sort_markers_v2(sheet_name)};
        auto previous_category{stylesv2::Category::unknown};
        for (const auto& marker : markers_v2)
        {
            const stylesv2::Style* style{database::styles::get_marker_data(sheet_name, marker)};
            if (style->category != previous_category)
            {
                pugi::xml_node tr_node = html_block.append_child("tr");
                tr_node.append_child("td");
                pugi::xml_node td_node = tr_node.append_child("td");
                pugi::xml_node h_node = td_node.append_child("h3");
                std::stringstream ss{};
                ss << style->category;
                h_node.text().set(ss.str().c_str());
                previous_category = style->category;
            }
            pugi::xml_node tr_node = html_block.append_child("tr");
            {
                pugi::xml_node td_node = tr_node.append_child("td");
                pugi::xml_node a_node = td_node.append_child("a");
                const std::string href = "view?sheet=" + sheet_name + "&style=" + marker;
                a_node.append_attribute("href") = href.c_str();
                a_node.text().set(marker.c_str());
            }
            {
                pugi::xml_node td_node = tr_node.append_child("td");
                td_node.text().set(style->name.c_str());
            }
            {
                pugi::xml_node td_node = tr_node.append_child("td");
                td_node.append_child("span").text().set("[");
                pugi::xml_node a_node = td_node.append_child("a");
                const std::string href = "?name=" + sheet_name + "&delete=" + marker;
                a_node.append_attribute("href") = href.c_str();
                a_node.text().set(translate("delete").c_str());
                td_node.append_child("span").text().set("]");
            }
        }
    }

    // Generate the html and set it on the page.
    std::stringstream ss{};
    html_block.print(ss, "", pugi::format_raw);
    view.set_variable("markerblock", ss.str());

    const std::string folder = filter_url_create_root_path({database_logic_databases(), "styles", sheet_name});
    view.set_variable("folder", folder);

    page += view.render("styles", "sheetm");

    page += assets_page::footer();

    return page;
}
