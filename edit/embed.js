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


var embedded_classnames = [];


// Creates CSS for embedded character styles.
function css4embeddedstyles ()
{
  var spans = $ (".ql-editor span");
  spans.each (function () {
    var classname = $ (this).attr ("class");
    if (classname) {
      var index = classname.indexOf ("0");
      if (index > 0) {
        if ($.inArray (classname, embedded_classnames) < 0) {
          embedded_classnames.push (classname);
          var name = classname.substring (2);
          var bits = name.split ("0");
          var font_style = null;
          var font_weight = null;
          var font_variant = null;
          // var font_size = null;
          var text_decoration = null;
          var vertical_align = null;
          var color = null;
          var background_color = null;
          for (i = 0; i < bits.length; i++) {
            name = bits [i];
            var element = document.createElement("span");
            element.className = name;
            document.body.appendChild (element);
            var properties = window.getComputedStyle (element, null);
            var value = properties.getPropertyValue ("font-style");
            if (value != "normal") font_style = value;
            value = properties.getPropertyValue ("font-weight");
            if (value != "normal") font_weight = value;
            value = properties.getPropertyValue ("font-variant");
            if (value != "normal") font_variant = value;
            // The font-size is calculated in "px", and is not very important, so is left out.
            // value = properties.getPropertyValue ("font-size");
            // if (value != "12px") font_size = value;
            value = properties.getPropertyValue ("text-decoration");
            if (value != "none") text_decoration = value;
            value = properties.getPropertyValue ("vertical-align");
            if (value != "baseline") vertical_align = value;
            value = properties.getPropertyValue ("color");
            if (value != "rgb(0, 0, 0)") color = value;
            value = properties.getPropertyValue ("background-color");
            if (value != "rgba(0, 0, 0, 0)") background_color = value;
          }
          if (font_style) $.stylesheet ("." + classname, "font-style", font_style);
          if (font_weight) $.stylesheet ("." + classname, "font-weight", font_weight);
          if (font_variant) $.stylesheet ("." + classname, "font-variant", font_variant);
          //if (font_size) $.stylesheet ("." + classname, "font-size", font_size);
          if (text_decoration) $.stylesheet ("." + classname, "text-decoration", text_decoration);
          if (vertical_align) $.stylesheet ("." + classname, "vertical-align", vertical_align);
          if (color) $.stylesheet ("." + classname, "color", color);
          if (background_color) $.stylesheet ("." + classname, "background-color", background_color);
        }
      }
    }
  });
}


// Returns the character style to apply based on the actual and desired style.
function editor_determine_character_style (actual, desired)
{
  // Deal with undefined actual style.
  if (!actual) actual = "";
  
  // If the desired style is already applied, remove it again.
  if (desired == actual) return "";

  // If no character style has been applied yet, apply it.
  if (actual == "") return desired;

  // If the applied style is embedded already, just apply the desired style only.
  if (actual.indexOf ("0") > 0) return desired;
  
  // Add the desired style to the one already there.
  return actual + "0" + desired;
}


function post_embedded_style_application (style)
{
  if (style.indexOf ("0") >= 0) css4embeddedstyles ();
}
