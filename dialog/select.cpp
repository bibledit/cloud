/*
Copyright (©) 2021 Aranggi Toar.

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


#include <dialog/select.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
#include <filter/string.h>
#include <filter/url.h>


namespace dialog::select {


std::string create_options(const std::vector<std::string>& values,
                           const std::vector<std::string>& displayed,
                           const std::string& selected)
{
  pugi::xml_document document {};
  for (size_t i {0}; i < values.size(); i++) {
    pugi::xml_node option_node = document.append_child("option");
    option_node.append_attribute("value") = values[i].c_str();
    if (selected == values[i])
      option_node.append_attribute("selected");
    const std::string display = (i >= displayed.size()) ? values[i] : displayed[i];
    option_node.text().set(display.c_str());
  }
  std::stringstream html_ss {};
  document.print (html_ss, "", pugi::format_raw);
  return html_ss.str();
}


// Create the html <select> element and fill them with the values.
static void create_select(pugi::xml_node parent, const Settings& settings)
{
  pugi::xml_node select_node = parent.append_child("select");
  select_node.append_attribute("id") = settings.identification;
  select_node.append_attribute("name") = settings.identification;
  if (settings.disabled)
    select_node.append_attribute("disabled") = "";
  for (size_t v {0}; v < settings.values.size(); v++) {
    pugi::xml_node option_node = select_node.append_child("option");
    option_node.append_attribute("value") = settings.values.at(v).c_str();
    if (settings.selected and settings.selected.value() == settings.values.at(v))
      option_node.append_attribute("selected");
    const std::string display = (v >= settings.displayed.size()) ? settings.values.at(v) : settings.displayed.at(v);
    option_node.text().set(display.c_str());
  }
}


// Create the info for the select node.
enum class Position { before, after };
static void create_info(pugi::xml_node parent, const Position position, std::optional<std::string> info)
{
  if (info) {
    pugi::xml_node span = parent.append_child("span");
    if (position == Position::before)
      info.value().append(" ");
    if (position == Position::after)
      info.value().insert(0, " ");
    span.text().set(info.value());
  }
}

std::string ajax(Settings& settings)
{
  pugi::xml_document document {};

  create_info(document, Position::before, settings.info_before);
  create_select (document, settings);
  create_info(document, Position::after, settings.info_after);

  // The Javascript to POST the selected value if it changes.
  std::string javascript = filter_url_file_get_contents(filter_url_create_root_path({"dialog/select.js"}));
  javascript = filter::strings::replace("identification", settings.identification, std::move(javascript));
  
  // Update the Javascript with the parameters to append to the POST request.
  std::stringstream ss{};
  for (const std::pair<std::string, std::string>& parameter : settings.parameters) {
    if (!ss.str().empty())
      ss << ", ";
    ss << parameter.first << ": " << std::quoted(parameter.second);
  }
  javascript = filter::strings::replace("parameters", std::move(ss).str(), std::move(javascript));
  
  pugi::xml_node script_node = document.append_child("script");
  script_node.text().set(javascript.c_str());
  
  // Convert it to html including Javascript.
  std::stringstream html_ss {};
  document.print (html_ss, "", pugi::format_raw);
  return html_ss.str();
}


std::string form(Settings& settings) // Todo
{
  pugi::xml_document document {};

  // The parameters, if any, to append to the POST action.
  std::stringstream ss{};
  for (const std::pair<std::string, std::string>& parameter : settings.parameters) {
    if (!ss.str().empty())
      ss << "&";
    ss << parameter.first << "=" << parameter.second;
  }
  std::string action {"?" + ss.str()};

  // Create the form.
  pugi::xml_node form_node = document.append_child("form");
  form_node.append_attribute("action") = action.c_str();
  form_node.append_attribute("method") = "post";
  create_info(form_node, Position::before, settings.info_before);
  create_select (form_node, settings);
  pugi::xml_node input_node = form_node.append_child("input");
  input_node.append_attribute("type") = "submit";
  create_info(form_node, Position::after, settings.info_after);

  // Convert it to html including Javascript.
  std::stringstream html_ss {};
  document.print (html_ss, "", pugi::format_raw);
  return html_ss.str();
}


}
