/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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


#include <filter/css.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <config/globals.h>
#include <webserver/request.h>
#include <styles/logic.h>


std::string Filter_Css::directionUnspecified (int value)
{
  value = value % 10;
  if (value == 0) return "checked";
  else return std::string();
}


std::string Filter_Css::directionLeftToRight (int value)
{
  value = value % 10;
  if (value == 1) return "checked";
  else return std::string();
}


std::string Filter_Css::directionRightToLeft (int value)
{
  value = value % 10;
  if (value == 2) return "checked";
  else return std::string();
}


std::string Filter_Css::ltr ()
{
  return "ltr";
}


std::string Filter_Css::rtl ()
{
  return "rtl";
}


int Filter_Css::directionValue (std::string direction)
{
  if (direction == ltr ()) return 1;
  if (direction == rtl ()) return 2;
  return 0;
}


std::string Filter_Css::writingModeUnspecified (int value)
{
  value = value / 10;
  value = value % 10;
  if (value == 0) return "checked";
  else return std::string();
}


std::string Filter_Css::writingModeTopBottomLeftRight (int value)
{
  value = value / 10;
  value = value % 10;
  if (value == 1) return "checked";
  else return std::string();
}


std::string Filter_Css::writingModeTopBottomRightLeft (int value)
{
  value = value / 10;
  value = value % 10;
  if (value == 2) return "checked";
  else return std::string();
}


std::string Filter_Css::writingModeBottomTopLeftRight (int value)
{
  value = value / 10;
  value = value % 10;
  if (value == 3) return "checked";
  else return std::string();
}


std::string Filter_Css::writingModeBottomTopRightLeft (int value)
{
  value = (value / 10);
  value = value % 10;
  if (value == 4) return "checked";
  else return std::string();
}


std::string Filter_Css::tb_lr ()
{
  return "tb-lr";
}


std::string Filter_Css::tb_rl ()
{
  return "tb-rl";
}


std::string Filter_Css::bt_lr ()
{
  return "bt-lr";
}


std::string Filter_Css::bt_rl ()
{
  return "bt-rl";
}


int Filter_Css::writingModeValue (std::string mode)
{
  if (mode == tb_lr ()) return 1;
  if (mode == tb_rl ()) return 2;
  if (mode == bt_lr ()) return 3;
  if (mode == bt_rl ()) return 4;
  return 0;
}


// The purpose of the function is to convert the name of the bible into a string
// that is acceptable as a class identifier in HTML.
// Since a bible can contain any Unicode character,
// just using the bible as the class identifier will not work.
// The function solves that.
std::string Filter_Css::getClass (std::string bible)
{
  std::string classs = md5 (bible);
  classs = classs.substr (0, 6);
  classs = "custom" + classs;
  return classs;
}


// This function produces CSS based on input.
// class: The class for the CSS.
// font: The name or URL of the font to use. It may be empty.
// directionvalue: The value for the text direction.
// $lineheigh: Value in percents.
// $letterspacing: Value multiplied by 10, in pixels.
std::string Filter_Css::get_css (std::string class_, std::string font, int directionvalue, int lineheight, int letterspacing)
{
  std::vector <std::string> css;
  
  // If the font has a URL, then it is a web font.
  if ((font != filter_url_basename_web (font)) && !font.empty()) {
    css.push_back ("@font-face");
    css.push_back ("{");
    css.push_back ("font-family: " + class_ + ";");
    css.push_back ("src: url(" + font + ");");
    css.push_back ("}");
    // Below, properly reference the above web font as the class.
    font = class_;
  }
  
  css.push_back ("." + class_);
  css.push_back ("{");
  
  if (font != "") {
    css.push_back ("font-family: " + font + ";");
  }
  
  int direction = directionvalue % 10;
  
  if (direction > 0) {
    std::string line = "direction: ";
    if (direction == 2) line += rtl ();
    else line += ltr ();
    line += ";";
    css.push_back (line);
  }
  
  int mode = directionvalue / 10;
  mode = mode % 10;
  
  if (mode > 0) {
    std::string line = "writing-mode: ";
    switch (mode) {
      case 1: line += tb_lr (); break;
      case 2: line += tb_rl (); break;
      case 3: line += bt_lr (); break;
      case 4: line += bt_rl (); break;
      default: line += tb_lr (); break;
    }
    line += ";";
    css.push_back (line);
  }
  
  if (lineheight != 100) {
    std::string line = "line-height: " + std::to_string(lineheight) + "%;";
    css.push_back (line);
  }
  
  if (letterspacing != 0) {
    float value = static_cast <float> (letterspacing / 10);
    std::string line = "letter-spacing: " + filter::strings::convert_to_string (value) + "px;";
    css.push_back (line);
  }
  
  css.push_back ("}");
  
  return filter::strings::implode (css, "\n");
}


void Filter_Css::distinction_set_basic ()
{
  return;
}


std::string Filter_Css::distinction_set_light (int itemstyleindex)
{
  if (itemstyleindex == 0) return "light-background";
  if (itemstyleindex == 1) return "light-menu-tabs";
  if (itemstyleindex == 2) return "light-editor";
  if (itemstyleindex == 3) return "light-active-editor";
  if (itemstyleindex == 4) return "light-workspacewrapper";
  return std::string();
}


std::string Filter_Css::distinction_set_dark (int itemstyleindex)
{
  if (itemstyleindex == 0) return "dark-background";
  if (itemstyleindex == 1) return "dark-menu-tabs";
  if (itemstyleindex == 2) return "dark-editor";
  if (itemstyleindex == 3) return "dark-active-editor";
  if (itemstyleindex == 4) return "dark-workspacewrapper";
  if (itemstyleindex == 5) return "dark-versebeam";
  return std::string();
}


std::string Filter_Css::distinction_set_redblue_light (int itemstyleindex)
{
  std::string standard_light = distinction_set_light (itemstyleindex);
  if (itemstyleindex == 1) {
    return standard_light = "redblue-menu-tabs";
  } else {
    return standard_light;
  }
}


std::string Filter_Css::distinction_set_redblue_dark (int itemstyleindex)
{
  std::string standard_dark = distinction_set_dark (itemstyleindex);
  if (itemstyleindex == 1) {
    return standard_dark = "redblue-menu-tabs";
  } else {
    return standard_dark;
  }
}


std::string Filter_Css::distinction_set_notes (int itemstyleindex)
{
  if (itemstyleindex == 0) return "note-status-new";
  if (itemstyleindex == 1) return "note-status-pending";
  if (itemstyleindex == 2) return "note-status-inprogress";
  if (itemstyleindex == 3) return "note-status-done";
  if (itemstyleindex == 4) return "note-status-reopened";
  if (itemstyleindex == 5) return "note-status-unset";
  return std::string();
}


std::string Filter_Css::theme_picker (int indexnumber, int itemstyleindex)
{
  if (indexnumber == 0) distinction_set_basic ();
  if (indexnumber == 1) return distinction_set_light (itemstyleindex);
  if (indexnumber == 2) return distinction_set_dark (itemstyleindex);
  if (indexnumber == 3) return distinction_set_redblue_light (itemstyleindex);
  if (indexnumber == 4) return distinction_set_redblue_dark (itemstyleindex);
  return std::string();
}


std::string filter_css_grey_background ()
{
  return R"(style="background-color: #CCCCCC")";
}

