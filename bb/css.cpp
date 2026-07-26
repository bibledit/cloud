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


#include <access/bible.h>
#include <assets/header.h>
#include <assets/page.h>
#include <assets/view.h>
#include <bb/css.h>
#include <database/config/bible.h>
#include <filter/css.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <fonts/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>


std::string bible_css_url()
{
    return "bible/css";
}


bool bible_css_acl(Webserver_Request& webserver_request)
{
    return roles::access_control(webserver_request, roles::translator);
}


std::string bible_css(Webserver_Request& webserver_request)
{
    Assets_Header header(translate("Font and text direction"), webserver_request);
    std::string page{header.run()};

    Assets_View view{};

    // The name of the Bible.
    const std::string bible = access_bible::clamp(webserver_request, webserver_request.query["bible"]);
    view.set_variable("bible", filter::string::escape_special_xml_characters(bible));

    // Data submission.
    if (webserver_request.post_count("submit"))
    {
        const std::string font = filter::string::trim(webserver_request.post_get("font"));
#ifdef HAVE_CLIENT
        // Bibledit client storage.
        database::config::bible::set_text_font_client(bible, font);
#else
        // Bibledit Cloud storage.
        database::config::bible::set_text_font(bible, font);
#endif

        const std::string focused_verse_font = filter::string::trim(webserver_request.post_get("focusedversefont"));
        database::config::bible::set_verse_editor_focused_verse_font(bible, focused_verse_font);

        const std::string s_direction = webserver_request.post_get("direction");
        const int i_direction = filter::css::direction_value(s_direction);

        const std::string s_mode = webserver_request.post_get("mode");
        const int i_mode = filter::css::writing_mode_value(s_mode);

        database::config::bible::set_text_direction(bible, i_mode * 10 + i_direction);

        const int lineheight = std::clamp(filter::string::convert_to_int(webserver_request.post_get("lineheight")), 50, 300);
        database::config::bible::set_line_height(bible, lineheight);

        const float letterspacing = std::clamp(filter::string::convert_to_float(webserver_request.post_get("letterspacing")), -3.0f, 3.0f);
        database::config::bible::set_letter_spacing(bible, static_cast<int>(10 * letterspacing));

        page += assets_page::success("The information was saved.");
    }

#ifdef HAVE_CLIENT
    view.enable_zone("client");
#endif

    const std::string font = fonts::logic::get_text_font(bible);
    view.set_variable("font", font);

    const std::string focused_verse_font = database::config::bible::get_verse_editor_focused_verse_font(bible);
    view.set_variable("focusedversefont", focused_verse_font);

    const int direction = database::config::bible::get_text_direction(bible);

    view.set_variable("direction_none", filter::css::direction_unspecified(direction));
    view.set_variable("direction_ltr", filter::css::direction_left_to_right(direction));
    view.set_variable("direction_rtl", filter::css::direction_right_to_left(direction));

    view.set_variable("mode_none", filter::css::writing_mode_unspecified(direction));
    view.set_variable("mode_tblr", filter::css::writing_mode_top_bottom_left_right(direction));
    view.set_variable("mode_tbrl", filter::css::writing_mode_top_bottom_right_left(direction));
    view.set_variable("mode_btlr", filter::css::writing_mode_bottom_top_left_right(direction));
    view.set_variable("mode_btrl", filter::css::writing_mode_bottom_top_right_left(direction));

    const int lineheight = database::config::bible::get_line_height(bible);
    view.set_variable("lineheight", std::to_string(lineheight));

    auto letterspacing = static_cast<float>(database::config::bible::get_letter_spacing(bible));
    letterspacing /= 10;
    view.set_variable("letterspacing", filter::string::convert_to_string(letterspacing));

    const std::string custom_class = filter::css::get_class(bible);
    view.set_variable("custom_class", custom_class);
    const std::string custom_css = filter::css::get_css(custom_class,
                                                        fonts::logic::get_font_path(font), {},
                                                        direction,
                                                        lineheight,
                                                        database::config::bible::get_letter_spacing(bible));
    view.set_variable("custom_css", custom_css);

    page += view.render("bb", "css");

    page += assets_page::footer();

    return page;
}
