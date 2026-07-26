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


#include <config/globals.h>
#include <filter/css.h>
#include <filter/md5.h>
#include <filter/string.h>
#include <filter/url.h>


namespace filter::css {


template<int n>
static std::string get_direction_checked_text_internal(const int value)
{
    return value % 10 == n ? "checked" : "";
}


std::string direction_unspecified(const int value)
{
    return get_direction_checked_text_internal<0>(value);
}


std::string direction_left_to_right(const int value)
{
    return get_direction_checked_text_internal<1>(value);
}


std::string direction_right_to_left(const int value)
{
    return get_direction_checked_text_internal<2>(value);
}


std::string ltr()
{
    return "ltr";
}


std::string rtl()
{
    return "rtl";
}


int direction_value(const std::string& direction)
{
    if (direction == ltr())
        return 1;
    if (direction == rtl())
        return 2;
    return 0;
}


std::string writing_mode_unspecified(const int value)
{
    return get_direction_checked_text_internal<0>(value / 10);
}


std::string writing_mode_top_bottom_left_right(const int value)
{
    return get_direction_checked_text_internal<1>(value / 10);
}


std::string writing_mode_top_bottom_right_left(const int value)
{
    return get_direction_checked_text_internal<2>(value / 10);
}


std::string writing_mode_bottom_top_left_right(const int value)
{
    return get_direction_checked_text_internal<3>(value / 10);
}


std::string writing_mode_bottom_top_right_left(const int value)
{
    return get_direction_checked_text_internal<4>(value / 10);
}


std::string tb_lr()
{
    return "tb-lr";
}


std::string tb_rl()
{
    return "tb-rl";
}


std::string bt_lr()
{
    return "bt-lr";
}


std::string bt_rl()
{
    return "bt-rl";
}


int writing_mode_value(const std::string& mode)
{
    if (mode == tb_lr())
        return 1;
    if (mode == tb_rl())
        return 2;
    if (mode == bt_lr())
        return 3;
    if (mode == bt_rl())
        return 4;
    return 0;
}


// The purpose of the function is to convert the name of the bible into a string
// that is acceptable as a class identifier in HTML.
// Since a bible can contain any Unicode character,
// just using the bible as the class identifier will not work.
// The function solves that.
std::string get_class(const std::string& bible)
{
    std::string cls {md5(bible)};
    cls.resize(6);
    cls.insert(0, "custom");
    return cls;
}


// This function produces CSS based on input.
// class: The class for the CSS.
// font: The name or URL of the font to use. It may be empty.
// directionvalue: The value for the text direction.
// $lineheigh: Value in percents.
// $letterspacing: Value multiplied by 10, in pixels.
std::string get_css(const std::string& class_,
                    std::string font,
                    const std::string& focused_verse_font,
                    int direction_value, int line_height, int letter_spacing)
{
    std::vector<std::string> css;

    // If the font has a URL, then it is a web font.
    if (font != filter_url_basename_web(font) and not font.empty())
    {
        css.emplace_back("@font-face");
        css.emplace_back("{");
        css.push_back("font-family: " + class_ + ";");
        css.push_back("src: url(" R"(")" + font + R"(")" ");");
        css.emplace_back("}");
        // Below, properly reference the above web font as the class.
        font = class_;
    }

    css.push_back("." + class_);
    css.emplace_back("{");

    if (not font.empty())
        css.push_back("font-family: " + font + ";");

    if (const int direction = direction_value % 10; direction > 0)
    {
        std::string line = "direction: ";
        if (direction == 2)
            line.append(rtl());
        else
            line.append(ltr());
        line.append(";");
        css.push_back(std::move(line));
    }

    int mode = direction_value / 10;
    mode = mode % 10;

    if (mode > 0)
    {
        std::string line = "writing-mode: ";
        switch (mode)
        {
        case 1: line.append(tb_lr());
            break;
        case 2: line.append(tb_rl());
            break;
        case 3: line.append(bt_lr());
            break;
        case 4: line.append(bt_rl());
            break;
        default: line += tb_lr();
            break;
        }
        line.append(";");
        css.push_back(line);
    }

    if (line_height != 100)
    {
        std::string line = "line-height: " + std::to_string(line_height) + "%;";
        css.push_back(line);
    }

    if (letter_spacing != 0)
    {
        const auto value = static_cast<float>(letter_spacing) / 10.0f;
        std::string line = "letter-spacing: " + string::convert_to_string(value) + "px;";
        css.push_back(line);
    }

    css.emplace_back("}");

    // Handle the possibly set font for the focused verse in the verse editor.
    // Handle situation that this font is a web font.
    {
        std::string focused_verse_font_name {focused_verse_font};
        if (focused_verse_font != filter_url_basename_web(focused_verse_font) and not focused_verse_font.empty())
        {
            focused_verse_font_name = std::to_string(std::hash<std::string>{}(focused_verse_font));
            css.emplace_back("@font-face");
            css.emplace_back("{");
            css.push_back("font-family: " R"(")" + focused_verse_font_name + R"(")" ";");
            css.push_back("src: url(" R"(")" + focused_verse_font + R"(")" ");");
            css.emplace_back("}");
        }
        if (not focused_verse_font.empty())
        {
            css.emplace_back(".ql-editor {");
            css.push_back("font-family: " R"(")" + focused_verse_font_name + R"(")" ";");
            css.emplace_back("}");
        }
    }

    return string::implode(css, "\n");
}


std::string distinction_set_light(const int item_style_index)
{
    if (item_style_index == 0) return "light-background";
    if (item_style_index == 1) return "light-menu-tabs";
    if (item_style_index == 2) return "light-editor";
    if (item_style_index == 3) return "light-active-editor";
    if (item_style_index == 4) return "light-workspacewrapper";
    return {};
}


std::string distinction_set_dark(const int item_style_index)
{
    if (item_style_index == 0) return "dark-background";
    if (item_style_index == 1) return "dark-menu-tabs";
    if (item_style_index == 2) return "dark-editor";
    if (item_style_index == 3) return "dark-active-editor";
    if (item_style_index == 4) return "dark-workspacewrapper";
    if (item_style_index == 5) return "dark-versebeam";
    return {};
}


std::string distinction_set_red_blue_light(const int item_style_index)
{
    std::string standard_light = distinction_set_light(item_style_index);
    if (item_style_index == 1)
        standard_light = "redblue-menu-tabs";
    return standard_light;
}


std::string distinction_set_red_blue_dark(const int item_style_index)
{
    std::string standard_dark = distinction_set_dark(item_style_index);
    if (item_style_index == 1)
        standard_dark = "redblue-menu-tabs";
    return standard_dark;
}


std::string distinction_set_notes(const int item_style_index)
{
    if (item_style_index == 0) return "note-status-new";
    if (item_style_index == 1) return "note-status-pending";
    if (item_style_index == 2) return "note-status-inprogress";
    if (item_style_index == 3) return "note-status-done";
    if (item_style_index == 4) return "note-status-reopened";
    if (item_style_index == 5) return "note-status-unset";
    return {};
}


std::string theme_picker(const int theme_style_index, const int item_style_index)
{
    //if (theme_style_index == 0) {};
    if (theme_style_index == 1) return distinction_set_light(item_style_index);
    if (theme_style_index == 2) return distinction_set_dark(item_style_index);
    if (theme_style_index == 3) return distinction_set_red_blue_light(item_style_index);
    if (theme_style_index == 4) return distinction_set_red_blue_dark(item_style_index);
    return {};
}


std::string grey_background()
{
    return R"(style="background-color: #CCCCCC")";
}


}
