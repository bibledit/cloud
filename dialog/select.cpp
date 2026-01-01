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
  if (settings.tooltip)
    select_node.append_attribute("title") = settings.tooltip.value().c_str();
}


static std::string remove_comment_from_code (std::string code)
{
  // Code with a comment may look like this:
  /* Code */
  filter::string::replace_between (code, "/*", "*/", std::string());
  return code;
}


std::string ajax(const Settings& settings)
{
  pugi::xml_document document {};

  create_select (document, settings);

  // The Javascript to POST the selected value if it changes.
  // The script should be a module because that defers execution till the document has been loaded.
  std::string javascript = filter_url_file_get_contents(filter_url_create_root_path({"dialog/selectajax.js"}));
  javascript = remove_comment_from_code (std::move(javascript));
  javascript = filter::string::replace("identification", settings.identification, std::move(javascript));
  
  pugi::xml_node script_node = document.append_child("script");
  script_node.append_attribute("type") = "module";
  script_node.text().set(javascript.c_str());
  
  // Convert it to html including Javascript.
  std::stringstream html_ss {};
  document.print (html_ss, "", pugi::format_raw);
  
  // Update the html with the parameters to append to the POST request.
  std::string html = html_ss.str();
  const std::string url = filter_url_build_http_query(settings.url, settings.parameters);
  html = filter::string::replace("URL", url, std::move(html));

  return html;
}


std::string form(const Settings& settings, const Form& form)
{
  pugi::xml_document document {};

  // The parameters, if any, to append to the POST action.
  std::stringstream ss{};
  for (const std::pair<std::string, std::string>& parameter : settings.parameters) {
    ss << (ss.str().empty()?"?":"&");
    ss << parameter.first << "=" << parameter.second;
  }
  const std::string action {settings.url + ss.str()};

  // Create the form.
  pugi::xml_node form_node = document.append_child("form");
  form_node.append_attribute("action") = action.c_str();
  form_node.append_attribute("method") = "post";
  form_node.append_attribute("style") = "display:inline!important;";
  create_select (form_node, settings);
  if (!form.auto_submit) {
    pugi::xml_node input_node = form_node.append_child("input");
    input_node.append_attribute("type") = "submit";
    if (settings.submit)
      input_node.append_attribute("value") = settings.submit.value().c_str();
  }
  
  // If automatic submit, add a script that does the job.
  // The script should be a module because that defers execution till the document has been loaded.
  if (form.auto_submit) {
    std::string javascript = filter_url_file_get_contents(filter_url_create_root_path({"dialog/selectform.js"}));
    javascript = remove_comment_from_code (std::move(javascript));
    javascript = filter::string::replace("identification", settings.identification, std::move(javascript));
    pugi::xml_node script_node = document.append_child("script");
    script_node.append_attribute("type") = "module";
    script_node.text().set(javascript.c_str());
  }

  // Convert it to html including Javascript.
  std::stringstream html_ss {};
  document.print (html_ss, "", pugi::format_raw);
  return html_ss.str();
}


}
