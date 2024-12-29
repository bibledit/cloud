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


#include <odf/text.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/image.h>
#include <filter/usfm.h>
#include <database/books.h>
#include <database/config/bible.h>
#include <database/bibleimages.h>
#include <styles/logic.h>


// Class for creating OpenDocument text documents.
// Initially the ODF Toolkit was used.
// But the Java code to generate this became too big for the Java compiler.
// The other thing is that Java is slow compared to the methods below written in C++.


odf_text::odf_text (std::string bible)
{
  m_bible = bible;

  // Unpack the .odt template.
  std::string template_odf = filter_url_create_root_path ({"odf", "template.odt"});
  unpacked_odt_folder = filter_archive_unzip (template_odf);
  filter_url_rmdir (filter_url_create_path ({unpacked_odt_folder, "Configurations2"}));
  // Create the Pictures folder.
  // pictures_folder = filter_url_create_path (unpackedOdtFolder, "Pictures");
  //filter_url_mkdir (pictures_folder);
  
  initialize_content_xml ();
  initialize_styles_xml ();
  
  automatic_note_caller = database::config::bible::get_odt_automatic_note_caller(m_bible);
}


odf_text::~odf_text ()
{
  filter_url_rmdir (unpacked_odt_folder);
}


// Build the default content.xml for the template.
void odf_text::initialize_content_xml ()
{
  pugi::xml_node rootnode = content_dom.append_child ("office:document-content");

  rootnode.append_attribute ("xmlns:office") = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
  rootnode.append_attribute ("xmlns:style") = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
  rootnode.append_attribute ("xmlns:text") = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
  rootnode.append_attribute ("xmlns:table") = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
  rootnode.append_attribute ("xmlns:draw") = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
  rootnode.append_attribute ("xmlns:fo") = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0";
  rootnode.append_attribute ("xmlns:xlink") = "http://www.w3.org/1999/xlink";
  rootnode.append_attribute ("xmlns:dc") = "http://purl.org/dc/elements/1.1/";
  rootnode.append_attribute ("xmlns:meta") = "urn:oasis:names:tc:opendocument:xmlns:meta:1.0";
  rootnode.append_attribute ("xmlns:number") = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";
  rootnode.append_attribute ("xmlns:svg") = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
  rootnode.append_attribute ("xmlns:chart") = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
  rootnode.append_attribute ("xmlns:dr3d") = "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0";
  rootnode.append_attribute ("xmlns:math") = "http://www.w3.org/1998/Math/MathML";
  rootnode.append_attribute ("xmlns:form") = "urn:oasis:names:tc:opendocument:xmlns:form:1.0";
  rootnode.append_attribute ("xmlns:script") = "urn:oasis:names:tc:opendocument:xmlns:script:1.0";
  rootnode.append_attribute ("xmlns:ooo") = "http://openoffice.org/2004/office";
  rootnode.append_attribute ("xmlns:ooow") = "http://openoffice.org/2004/writer";
  rootnode.append_attribute ("xmlns:oooc") = "http://openoffice.org/2004/calc";
  rootnode.append_attribute ("xmlns:dom") = "http://www.w3.org/2001/xml-events";
  rootnode.append_attribute ("xmlns:xforms") = "http://www.w3.org/2002/xforms";
  rootnode.append_attribute ("xmlns:xsd") = "http://www.w3.org/2001/XMLSchema";
  rootnode.append_attribute ("xmlns:xsi") = "http://www.w3.org/2001/XMLSchema-instance";
  rootnode.append_attribute ("xmlns:rpt") = "http://openoffice.org/2005/report";
  rootnode.append_attribute ("xmlns:of") = "urn:oasis:names:tc:opendocument:xmlns:of:1.2";
  rootnode.append_attribute ("xmlns:xhtml") = "http://www.w3.org/1999/xhtml";
  rootnode.append_attribute ("xmlns:grddl") = "http://www.w3.org/2003/g/data-view#";
  rootnode.append_attribute ("xmlns:tableooo") = "http://openoffice.org/2009/table";
  rootnode.append_attribute ("xmlns:field") = "urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0";
  rootnode.append_attribute ("xmlns:formx") = "urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0";
  rootnode.append_attribute ("xmlns:css3t") = "http://www.w3.org/TR/css3-text/";
  rootnode.append_attribute ("office:version") = "1.2";
  rootnode.append_attribute ("grddl:transformation") = "http://docs.oasis-open.org/office/1.2/xslt/odf2rdf.xsl";

  pugi::xml_node office_scripts = rootnode.append_child ("office:scripts");
  if (office_scripts) {}

  pugi::xml_node office_font_face_decls = rootnode.append_child ("office:font-face-decls");
  {
    pugi::xml_node childnode;

    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Times New Roman";
    childnode.append_attribute ("svg:font-family") = "'Times New Roman'";
    childnode.append_attribute ("style:font-family-generic") = "roman";
    childnode.append_attribute ("style:font-pitch") = "variable";
    
    std::string fontname = database::config::bible::get_export_font (m_bible);
    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = fontname.c_str();
    fontname.insert (0, "'");
    fontname.append ("'");
    childnode.append_attribute ("svg:font-family") = fontname.c_str();
    childnode.append_attribute ("style:font-family-generic") = "roman";
    childnode.append_attribute ("style:font-pitch") = "variable";
    
    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Arial";
    childnode.append_attribute ("svg:font-family") = "Arial";
    childnode.append_attribute ("style:font-family-generic") = "swiss";
    childnode.append_attribute ("style:font-pitch") = "variable";
  
    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Droid Sans Fallback";
    childnode.append_attribute ("svg:font-family") = "'Droid Sans Fallback'";
    childnode.append_attribute ("style:font-family-generic") = "system";
    childnode.append_attribute ("style:font-pitch") = "variable";
  
    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Lohit Hindi";
    childnode.append_attribute ("svg:font-family") = "'Lohit Hindi'";
    childnode.append_attribute ("style:font-family-generic") = "system";
    childnode.append_attribute ("style:font-pitch") = "variable";
  }

  pugi::xml_node office_automatic_styles = rootnode.append_child ("office:automatic-styles");
  {
    pugi::xml_node style_style;
    style_style = office_automatic_styles.append_child ("style:style");
    style_style.append_attribute ("style:name") = "P1";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = "Header";
    {
      pugi::xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      {
        pugi::xml_node style_tab_stops = style_paragraph_properties.append_child ("style:tab-stops");
        {
          pugi::xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = "9.005cm";
          style_tab_stop.append_attribute ("style:type") = "center";
          style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = "17.986cm";
          style_tab_stop.append_attribute ("style:type") = "right";
        }
      }
    }
    style_style = office_automatic_styles.append_child ("style:style");
    style_style.append_attribute ("style:name") = "P2";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = "Header_20_left";
    {
      pugi::xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      {
        pugi::xml_node style_tab_stops = style_paragraph_properties.append_child ("style:tab-stops");
        {
          pugi::xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = "8.193cm";
          style_tab_stop.append_attribute ("style:type") = "center";
          style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = "17.963cm";
          style_tab_stop.append_attribute ("style:type") = "right";
        }
      }
    }

    // Add the automatic image style.
    // <style:style style:name="fr1" style:family="graphic" style:parent-style-name="Graphics">
    //   <style:graphic-properties style:mirror="none" fo:clip="rect(0mm, 0mm, 0mm, 0mm)" draw:luminance="0%" draw:contrast="0%" draw:red="0%" draw:green="0%" draw:blue="0%" draw:gamma="100%" draw:color-inversion="false" draw:image-opacity="100%" draw:color-mode="standard" />
    // </style:style>
    style_style = office_automatic_styles.append_child ("style:style");
    style_style.append_attribute ("style:name") = "fr1";
    style_style.append_attribute ("style:family") = "graphic";
    style_style.append_attribute ("style:parent-style-name") = "Graphics";
    {
      pugi::xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      style_paragraph_properties.append_attribute ("style:mirror") = "none";
      style_paragraph_properties.append_attribute ("fo:clip") = "rect(0mm, 0mm, 0mm, 0mm)";
      style_paragraph_properties.append_attribute ("draw:luminance") = "0%";
      style_paragraph_properties.append_attribute ("draw:contrast") = "0%";
      style_paragraph_properties.append_attribute ("draw:red") = "0%";
      style_paragraph_properties.append_attribute ("draw:green") = "0%";
      style_paragraph_properties.append_attribute ("draw:blue") = "0%";
      style_paragraph_properties.append_attribute ("draw:gamma") = "100%";
      style_paragraph_properties.append_attribute ("draw:color-inversion") = "false";
      style_paragraph_properties.append_attribute ("draw:image-opacity") = "100%";
      style_paragraph_properties.append_attribute ("draw:color-mode") = "standard";
    }
  }

  pugi::xml_node office_body = rootnode.append_child ("office:body");
  {
    office_text_node = office_body.append_child ("office:text");
    {
      pugi::xml_node text_sequence_decls = office_text_node.append_child ("text:sequence-decls");
      {
        pugi::xml_node text_sequence_decl = text_sequence_decls.append_child ("text:sequence-decl");
        text_sequence_decl.append_attribute ("text:display-outline-level") = "0";
        text_sequence_decl.append_attribute ("text:name") = "Illustration";
        text_sequence_decl = text_sequence_decls.append_child ("text:sequence-decl");
        text_sequence_decl.append_attribute ("text:display-outline-level") = "0";
        text_sequence_decl.append_attribute ("text:name") = "Table";
        text_sequence_decl = text_sequence_decls.append_child ("text:sequence-decl");
        text_sequence_decl.append_attribute ("text:display-outline-level") = "0";
        text_sequence_decl.append_attribute ("text:name") = "Text";
        text_sequence_decl = text_sequence_decls.append_child ("text:sequence-decl");
        text_sequence_decl.append_attribute ("text:display-outline-level") = "0";
        text_sequence_decl.append_attribute ("text:name") = "Drawing";
      }
      // The default text:p element is not added so the document appears empty.
    }
  }
}


// Build the default styles.xml for the template.
void odf_text::initialize_styles_xml ()
{
  pugi::xml_node rootnode = styles_dom.append_child ("office:document-styles");

  rootnode.append_attribute ("xmlns:office") = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
  rootnode.append_attribute ("xmlns:style") = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
  rootnode.append_attribute ("xmlns:text") = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
  rootnode.append_attribute ("xmlns:table") = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
  rootnode.append_attribute ("xmlns:draw") = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
  rootnode.append_attribute ("xmlns:fo") = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0";
  rootnode.append_attribute ("xmlns:xlink") = "http://www.w3.org/1999/xlink";
  rootnode.append_attribute ("xmlns:dc") = "http://purl.org/dc/elements/1.1/";
  rootnode.append_attribute ("xmlns:meta") = "urn:oasis:names:tc:opendocument:xmlns:meta:1.0";
  rootnode.append_attribute ("xmlns:number") = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";
  rootnode.append_attribute ("xmlns:svg") = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
  rootnode.append_attribute ("xmlns:chart") = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
  rootnode.append_attribute ("xmlns:dr3d") = "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0";
  rootnode.append_attribute ("xmlns:math") = "http://www.w3.org/1998/Math/MathML";
  rootnode.append_attribute ("xmlns:form") = "urn:oasis:names:tc:opendocument:xmlns:form:1.0";
  rootnode.append_attribute ("xmlns:script") = "urn:oasis:names:tc:opendocument:xmlns:script:1.0";
  rootnode.append_attribute ("xmlns:ooo") = "http://openoffice.org/2004/office";
  rootnode.append_attribute ("xmlns:ooow") = "http://openoffice.org/2004/writer";
  rootnode.append_attribute ("xmlns:oooc") = "http://openoffice.org/2004/calc";
  rootnode.append_attribute ("xmlns:dom") = "http://www.w3.org/2001/xml-events";
  rootnode.append_attribute ("xmlns:rpt") = "http://openoffice.org/2005/report";
  rootnode.append_attribute ("xmlns:of") = "urn:oasis:names:tc:opendocument:xmlns:of:1.2";
  rootnode.append_attribute ("xmlns:xhtml") = "http://www.w3.org/1999/xhtml";
  rootnode.append_attribute ("xmlns:grddl") = "http://www.w3.org/2003/g/data-view#";
  rootnode.append_attribute ("xmlns:tableooo") = "http://openoffice.org/2009/table";
  rootnode.append_attribute ("xmlns:css3t") = "http://www.w3.org/TR/css3-text/";
  rootnode.append_attribute ("office:version") = "1.2";
  rootnode.append_attribute ("grddl:transformation") = "http://docs.oasis-open.org/office/1.2/xslt/odf2rdf.xsl";

  pugi::xml_node office_font_face_decls = rootnode.append_child ("office:font-face-decls");
  {
    pugi::xml_node childnode;

    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Times New Roman";
    childnode.append_attribute ("svg:font-family") = "'Times New Roman'";
    childnode.append_attribute ("style:font-family-generic") = "roman";
    childnode.append_attribute ("style:font-pitch") = "variable";
  
    std::string fontname = database::config::bible::get_export_font (m_bible);
    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = fontname.c_str();
    fontname.insert (0, "'");
    fontname.append ("'");
    childnode.append_attribute ("svg:font-family") = fontname.c_str();
    childnode.append_attribute ("style:font-family-generic") = "roman";
    childnode.append_attribute ("style:font-pitch") = "variable";

    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Arial";
    childnode.append_attribute ("svg:font-family") = "Arial";
    childnode.append_attribute ("style:font-family-generic") = "swiss";
    childnode.append_attribute ("style:font-pitch") = "variable";
  
    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Droid Sans Fallback";
    childnode.append_attribute ("svg:font-family") = "'Droid Sans Fallback'";
    childnode.append_attribute ("style:font-family-generic") = "system";
    childnode.append_attribute ("style:font-pitch") = "variable";
  
    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Lohit Hindi";
    childnode.append_attribute ("svg:font-family") = "'Lohit Hindi'";
    childnode.append_attribute ("style:font-family-generic") = "system";
    childnode.append_attribute ("style:font-pitch") = "variable";
  }

  office_styles_node = rootnode.append_child ("office:styles");
  {
    pugi::xml_node style_default_style = office_styles_node.append_child ("style:default-style");
    style_default_style.append_attribute ("style:family") = "graphic";
    pugi::xml_node style_paragraph_properties = style_default_style.append_child ("style:paragraph-properties");
    if (style_paragraph_properties) {}
    pugi::xml_node style_text_properties = style_default_style.append_child ("style:text-properties");
    if (style_text_properties) {}
  }
  {
    pugi::xml_node style_default_style = office_styles_node.append_child ("style:default-style");
    style_default_style.append_attribute ("style:family") = "paragraph";
    pugi::xml_node style_paragraph_properties = style_default_style.append_child ("style:paragraph-properties");
    if (style_paragraph_properties) {}
    pugi::xml_node style_text_properties = style_default_style.append_child ("style:text-properties");
    if (style_text_properties) {}
  }
  {
    pugi::xml_node style_default_style = office_styles_node.append_child ("style:default-style");
    style_default_style.append_attribute ("style:family") = "table";
  }
  {
    pugi::xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:class") = "text";
  }
  {
    pugi::xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = "Heading";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:next-style-name") = "Text_20_body";
    style_style.append_attribute ("style:class") = "text";
    {
      pugi::xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      style_paragraph_properties.append_attribute ("fo:margin-top") = "0.423cm";
      style_paragraph_properties.append_attribute ("fo:margin-bottom") = "0.212cm";
      style_paragraph_properties.append_attribute ("fo:keep-with-next") = "always";
    }
  }
  {
    pugi::xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = "Text_20_body";
    style_style.append_attribute ("style:display-name") = "Text body";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:class") = "text";
    {
      pugi::xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      style_paragraph_properties.append_attribute ("fo:margin-top") = "0cm";
      style_paragraph_properties.append_attribute ("fo:margin-bottom") = "0.212cm";
    }
  }
  {
    pugi::xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = "Header";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:class") = "extra";
  }
  {
    pugi::xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = "Header_20_left";
    style_style.append_attribute ("style:display-name") = "Header left";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:class") = "extra";
  }

  // Update the tab-stops in the header style. The tab stops depend on page and margin dimensions.
  int centerPosition = filter::strings::convert_to_int (database::config::bible::get_page_width (m_bible)) - filter::strings::convert_to_int (database::config::bible::get_inner_margin (m_bible)) - filter::strings::convert_to_int (database::config::bible::get_outer_margin (m_bible));

  pugi::xml_node office_automatic_styles = rootnode.append_child ("office:automatic-styles");
  {
    pugi::xml_node style_style = office_automatic_styles.append_child ("style:style");
    style_style.append_attribute ("style:name") = "MP1";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = "Header";
    {
      pugi::xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      {
        pugi::xml_node style_tab_stops = style_paragraph_properties.append_child ("style:tab-stops");
        centerPosition /= 2;
        {
          pugi::xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = filter::strings::convert_to_string (std::to_string (centerPosition) + "mm").c_str();
          style_tab_stop.append_attribute ("style:type") = "center";
        }
        {
          pugi::xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = filter::strings::convert_to_string (std::to_string (centerPosition * 2) + "mm").c_str();
          style_tab_stop.append_attribute ("style:type") = "right";
        }
      }
    }
  }
  {
    pugi::xml_node style_style = office_automatic_styles.append_child ("style:style");
    style_style.append_attribute ("style:name") = "MP2";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = "Header_20_left";
    {
      pugi::xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      {
        pugi::xml_node style_tab_stops = style_paragraph_properties.append_child ("style:tab-stops");
        {
          pugi::xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = filter::strings::convert_to_string (std::to_string (centerPosition) + "mm").c_str();
          style_tab_stop.append_attribute ("style:type") = "center";
        }
        {
          pugi::xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = filter::strings::convert_to_string (std::to_string (centerPosition * 2) + "mm").c_str();
          style_tab_stop.append_attribute ("style:type") = "right";
        }
      }
    }
  }
  {
    pugi::xml_node style_page_layout = office_automatic_styles.append_child ("style:page-layout");
    style_page_layout.append_attribute ("style:name") = "Mpm1";
    style_page_layout.append_attribute ("style:page-usage") = "mirrored";
    {
      pugi::xml_node style_page_layout_properties = style_page_layout.append_child ("style:page-layout-properties");
      // Take the page size and margins from the Bible's settings.
      style_page_layout_properties.append_attribute ("fo:page-width") = filter::strings::convert_to_string (database::config::bible::get_page_width (m_bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("fo:page-height") = filter::strings::convert_to_string (database::config::bible::get_page_height (m_bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("style:num-format") = "1";
      style_page_layout_properties.append_attribute ("style:print-orientation") = "portrait";
      style_page_layout_properties.append_attribute ("fo:margin-top") = filter::strings::convert_to_string (database::config::bible::get_top_margin (m_bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("fo:margin-bottom") = filter::strings::convert_to_string (database::config::bible::get_bottom_margin (m_bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("fo:margin-left") = filter::strings::convert_to_string (database::config::bible::get_inner_margin (m_bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("fo:margin-right") = filter::strings::convert_to_string (database::config::bible::get_outer_margin (m_bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("style:writing-mode") = "lr-tb";
      style_page_layout_properties.append_attribute ("style:footnote-max-height") = "0cm";
      {
        pugi::xml_node style_footnote_sep = style_page_layout_properties.append_child ("style:footnote-sep");
        style_footnote_sep.append_attribute ("style:width") = "0.018cm";
        style_footnote_sep.append_attribute ("style:distance-before-sep") = "0.101cm";
        style_footnote_sep.append_attribute ("style:distance-after-sep") = "0.101cm";
        style_footnote_sep.append_attribute ("style:adjustment") = "left";
        style_footnote_sep.append_attribute ("style:rel-width") = "25%";
        style_footnote_sep.append_attribute ("style:color") = "#000000";
      }
    }
    {
      pugi::xml_node style_header_style = style_page_layout.append_child ("style:header-style");
      {
        pugi::xml_node style_header_footer_properties = style_header_style.append_child ("style:header-footer-properties");
        style_header_footer_properties.append_attribute ("fo:min-height") = "0.799cm";
        style_header_footer_properties.append_attribute ("fo:margin-left") = "0cm";
        style_header_footer_properties.append_attribute ("fo:margin-right") = "0cm";
        style_header_footer_properties.append_attribute ("fo:margin-bottom") = "0.3cm";
        style_header_footer_properties.append_attribute ("style:dynamic-spacing") = "false";
      }
      [[maybe_unused]] pugi::xml_node style_footer_style = style_page_layout.append_child ("style:footer-style");
    }
  }
  {
    // Do not write the date style for the running headers, so that it takes the default style.
    /*
    xml_node number_date_style = .append_child ("number:date-style");
    xmlAddChild (office_automatic_styles, number_date_style);
    .append_attribute (number_date_style,  "style:name") = "N81";
    {
      xml_node number_day = .append_child ("number:day");
      xmlAddChild (number_date_style, number_day);
      xml_node number_text = .append_child ("number:text");
      xmlAddChild (number_date_style, number_text);
      {
        xml_node textnode = xmlNewText( ".");
        xmlAddChild (number_text, textnode);
      }
      xml_node number_month = .append_child ("number:month");
      xmlAddChild (number_date_style, number_month);
      {
        .append_attribute (number_month,  "number:style") = "long";
        .append_attribute (number_month,  "number:textual") = "true";
      }
      number_text = .append_child ("number:text");
      xmlAddChild (number_date_style, number_text);
      xml_node number_year = .append_child ("number:year");
      xmlAddChild (number_date_style, number_year);
      {
        .append_attribute (number_year,  "number:style") = "long";
      }
    }
    */
  }
  pugi::xml_node office_master_styles = rootnode.append_child ("office:master-styles");
  {
    pugi::xml_node style_master_page = office_master_styles.append_child ("style:master-page");
    style_master_page.append_attribute ("style:name") = styles_logic_standard_sheet ().c_str();
    style_master_page.append_attribute ("style:page-layout-name") = "Mpm1";
    {
      pugi::xml_node style_header = style_master_page.append_child ("style:header");
      {
        pugi::xml_node text_p = style_header.append_child ("text:p");
        text_p.append_attribute ("text:style-name") = "MP1";
        {
          pugi::xml_node node = text_p.append_child ("text:page-number");
          node.append_attribute ("text:select-page") = "current";
          node.text ().set ("1");
        }
        {
          text_p.append_child ("text:tab");
        }
        // Whether and how to put the date in the running headers.
        if (database::config::bible::get_date_in_header (m_bible)) {
          pugi::xml_node node = text_p.append_child ("text:date");
          node.append_attribute ("style:data-style-name") = "N81";
          node.append_attribute ("text:date-value") = "";
          node.text ().set ("");
        }
        {
          text_p.append_child ("text:tab");
        }
        {
          pugi::xml_node node = text_p.append_child ("text:chapter");
          node.append_attribute ("text:display") = "name";
          node.append_attribute ("text:outline-level") = "1";
        }
      }
      pugi::xml_node style_header_left = style_master_page.append_child ("style:header-left");
      {
        pugi::xml_node text_p = style_header_left.append_child ("text:p");
        text_p.append_attribute ("text:style-name") = "MP2";
        {
          pugi::xml_node node = text_p.append_child ("text:chapter");
          node.append_attribute ("text:display") = "name";
          node.append_attribute ("text:outline-level") = "1";
        }
        {
          text_p.append_child ("text:tab");
        }
        // Whether and how to put the date in the running headers.
        if (database::config::bible::get_date_in_header (m_bible)) {
          pugi::xml_node node = text_p.append_child ("text:date");
          node.append_attribute ("style:data-style-name") = "N81";
          node.append_attribute ("text:date-value") = "";
          node.text ().set ("");
        }
        {
          text_p.append_child ("text:tab");
        }
        {
          pugi::xml_node node = text_p.append_child ("text:page-number");
          node.append_attribute ("text:select-page") = "current";
          node.text ().set ("1");
        }
      }
    }
  }
}


void odf_text::new_paragraph (std::string style)
{
  current_text_p_node = office_text_node.append_child ("text:p");
  current_text_p_node_style_name = current_text_p_node.append_attribute ("text:style-name") = style.c_str();
  m_current_text_p_node_opened = true;
  m_current_paragraph_style = style;
  m_current_paragraph_content.clear();
}


// This function adds text to the current paragraph.
// $text: The text to add.
void odf_text::add_text (std::string text)
{
  // Bail out if there's no text.
  if (text.empty()) return;

  // Ensure a paragraph has started.
  if (!m_current_text_p_node_opened) new_paragraph ();
  
  // Temporal styles array should have at least one style for the code below to work.
  // So ensure it has at least one style.
  std::vector <std::string> styles (m_current_text_style.begin (), m_current_text_style.end ());
  if (styles.empty()) styles.push_back (std::string());
  
  // Write a text span element, nesting the second and later ones.
  pugi::xml_node dom_node = current_text_p_node;
  for (std::string style : styles) {
    pugi::xml_node text_span_node = dom_node.append_child ("text:span");
    if (!style.empty ()) {
      text_span_node.append_attribute ("text:style-name") = convert_style_name (style).c_str();
    }
    dom_node = text_span_node;
  }
  // It used to escape the special XML characters.
  // Ecaping e.g. the apostrophy (') would lead to the following fragment in the .odt file:
  // &apos;
  // So it no longer escapes the special XML characters.
  dom_node.text ().set (text.c_str());

  // Update public paragraph text.
  m_current_paragraph_content += text;
}


// This creates a heading with contents styled "Heading 1".
// $text: Contents.
void odf_text::new_heading1 (std::string text, bool hide)
{
  new_named_heading ("Heading 1", text, hide);
}


// This creates the page break style.
void odf_text::create_page_break_style ()
{
  // This is how the style looks in styles.xml:
  // <style:style style:display-name="Page Break" style:family="paragraph" style:name="Page_20_Break">
  // <style:paragraph-properties fo:break-after="page" fo:line-height="0.05cm" fo:margin-bottom="0cm" fo:margin-top="0cm"/>
  // <style:text-properties fo:font-size="2pt" style:font-size-asian="2pt" style:font-size-complex="2pt"/>
  // </style:style>
  pugi::xml_node style_dom_element = office_styles_node.append_child ("style:style");
  style_dom_element.append_attribute ("style:name") = "Page_20_Break";
  style_dom_element.append_attribute ("style:display-name") = "Page Break";
  style_dom_element.append_attribute ("style:family") = "paragraph";

  pugi::xml_node style_paragraph_properties_dom_element = style_dom_element.append_child ("style:paragraph-properties");
  style_paragraph_properties_dom_element.append_attribute ("fo:break-after") = "page";
  style_paragraph_properties_dom_element.append_attribute ("fo:line-height") = "0.05cm";
  style_paragraph_properties_dom_element.append_attribute ("fo:margin-bottom") = "0cm";
  style_paragraph_properties_dom_element.append_attribute ("fo:margin-top") = "0cm";

  pugi::xml_node style_text_properties_dom_element = style_dom_element.append_child ("style:text-properties");
  style_text_properties_dom_element.append_attribute ("fo:font-size") = "2pt";
  style_text_properties_dom_element.append_attribute ("style:font-size-asian") = "2pt";
  style_text_properties_dom_element.append_attribute ("style:font-size-complex") = "2pt";
}


// This applies a page break.
void odf_text::new_page_break ()
{
  new_paragraph ("Page_20_Break");
  // Always clear the paragraph-opened-flag,
  // because we don't want subsequent text to be added to this page break,
  // since it would be nearly invisible, and thus text would mysteriously get lost.
  m_current_text_p_node_opened = false;
  m_current_paragraph_style.clear ();
  m_current_paragraph_content.clear ();
}


// This creates a paragraph style.
// $name: the name of the style, e.g. 'p'.
// $dropcaps: If 0, there are no drop caps.
//            If greater than 0, it the number of characters in drop caps style.
void odf_text::create_paragraph_style (std::string name,
                                       std::string fontname,
                                       float fontsize,
                                       int italic, int bold, int underline, int smallcaps,
                                       int alignment,
                                       float spacebefore, float spaceafter,
                                       float leftmargin, float rightmargin,
                                       float firstlineindent,
                                       bool keep_with_next,
                                       int dropcaps)
{
  // Whether to align verse numbers in poetry to the left of the margin,
  // and if so, whether this is one of the defined poetry styles.
  bool is_poetry_q_style {false};
  if (database::config::bible::get_odt_poetry_verses_left (m_bible)) {
    is_poetry_q_style = filter::usfm::is_standard_q_poetry (name);
  }
  
  // It looks like this in styles.xml:
  // <style:style style:display-name="p_c1" style:family="paragraph" style:name="p_c1">
  //   <style:paragraph-properties fo:margin-bottom="0mm" fo:margin-left="0mm" fo:margin-right="0mm" fo:margin-top="0mm" fo:text-align="justify" fo:text-indent="0mm"/>
  //     <style:drop-cap style:distance="0.15cm" style:length="1" style:lines="2"/>
  //   <style:paragraph-properties>
  //   <style:text-properties fo:font-size="12pt" style:font-size-asian="12pt" style:font-size-complex="12pt"/>
  // </style:style>
  pugi::xml_node style_style_node = office_styles_node.append_child ("style:style");
  style_style_node.append_attribute ("style:name") = convert_style_name (name).c_str();
  style_style_node.append_attribute ("style:display-name") = name.c_str();
  style_style_node.append_attribute ("style:family") = "paragraph";

  pugi::xml_node style_paragraph_properties_node = style_style_node.append_child ("style:paragraph-properties");

  pugi::xml_node style_text_properties_node = style_style_node.append_child ("style:text-properties");

  style_paragraph_properties_node.append_attribute ("style:font-name") = fontname.c_str();
  fontname.insert (0, "'");
  fontname.append ("'");
  style_text_properties_node.append_attribute ("fo:font-family") = fontname.c_str();

  std::string sfontsize = filter::strings::convert_to_string (fontsize) + "pt";
  style_text_properties_node.append_attribute ("fo:font-size") = sfontsize.c_str();
  style_text_properties_node.append_attribute ("style:font-size-asian") = sfontsize.c_str();
  style_text_properties_node.append_attribute ("style:font-size-complex") = sfontsize.c_str();

  // Italics, bold, underline, small caps can be either ooitOff or ooitOn for a paragraph.
  if (italic != ooitOff) {
    style_text_properties_node.append_attribute ("fo:font-style") = "italic";
    style_text_properties_node.append_attribute ("style:font-style-asian") = "italic";
    style_text_properties_node.append_attribute ("style:font-style-complex") = "italic";
  }
  if (bold != ooitOff) {
    style_text_properties_node.append_attribute ("fo:font-weight") = "bold";
    style_text_properties_node.append_attribute ("style:font-weight-asian") = "bold";
    style_text_properties_node.append_attribute ("style:font-weight-complex") = "bold";
  }
  if (underline != ooitOff) {
    style_text_properties_node.append_attribute ("style:text-underline-style") = "solid";
    style_text_properties_node.append_attribute ("style:text-underline-width") = "auto";
    style_text_properties_node.append_attribute ("style:text-underline-color") = "font-color";
  }
  if (smallcaps != ooitOff) {
    style_text_properties_node.append_attribute ("fo:font-variant") = "small-caps";
  }

  // Text alignment can be: AlignmentLeft, AlignmentCenter, AlignmentRight, AlignmentJustify.
  std::string alignmenttext {};
  switch (alignment) {
    case AlignmentLeft:    alignmenttext = "start";   break;
    case AlignmentCenter:  alignmenttext = "center";  break;
    case AlignmentRight:   alignmenttext = "end";     break;
    case AlignmentJustify: alignmenttext = "justify"; break;
    default: break;
  }
  style_paragraph_properties_node.append_attribute ("fo:text-align") = alignmenttext.c_str();
  style_paragraph_properties_node.append_attribute ("style:justify-single-word") = "false";

  // Deal with the paragraph dimensions.
  // The values are given in millimeters.
  // First the top and bottom margins.
  std::string space_before_mm = filter::strings::convert_to_string (spacebefore) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:margin-top") = space_before_mm.c_str();
  std::string space_after_mm = filter::strings::convert_to_string (spaceafter) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:margin-bottom") = space_after_mm.c_str();
  std::string left_margin_mm = filter::strings::convert_to_string (leftmargin) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:margin-left") = left_margin_mm.c_str();
  std::string right_margin_mm = filter::strings::convert_to_string (rightmargin) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:margin-right") = right_margin_mm.c_str();
  // In a normal paragraph the first line indent is as given in the stylesheet.
  // In a poetry paragraph the first line indent is the negative left margin.
  // The goal is that the left is at a 0 left margin,
  // and that the verse is aligned at the very left of the column.
  // (And then a tab puts the text at the desired first line indent space.)
  int millimeters = static_cast<int>(firstlineindent);
  if (is_poetry_q_style) millimeters = static_cast <int> (0 - leftmargin);
  std::string first_lineindent_mm = std::to_string (millimeters) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:text-indent") = first_lineindent_mm.c_str();

  if (keep_with_next) {
    style_paragraph_properties_node.append_attribute ("fo:keep-together") = "always";
    style_paragraph_properties_node.append_attribute ("fo:keep-with-next") = "always";
  }

  if (dropcaps > 0) {
    // E.g.: <style:drop-cap style:lines="2" style:length="2" style:distance="0.15cm"/>
    std::string length = std::to_string (dropcaps);
    pugi::xml_node style_drop_cap_node = style_paragraph_properties_node.append_child ("style:drop-cap");
    style_drop_cap_node.append_attribute ("style:lines") = "2";
    style_drop_cap_node.append_attribute ("style:length") = length.c_str();
    style_drop_cap_node.append_attribute ("style:distance") = "0.15cm";
  }
  
  // For poetry styles like q, q1, and so on,
  // there's an additional definition of the tab settings.
  // Later there were more tab stops added,
  // each tab stop slightly deeper than the previous one.
  // The reason for adding more tab stops is this:
  // The chapter number at times is wider than the first tab stop,
  // pushing the indent of the first line too deep.
  // See issue https://github.com/bibledit/cloud/issues/671
  if (is_poetry_q_style) {
    pugi::xml_node style_tab_stops = style_paragraph_properties_node.append_child("style:tab-stops");
    int tab_indent = static_cast<int> (firstlineindent);
    for (int i = 0; i < 10; i++) {
      pugi::xml_node style_tab_stop = style_tab_stops.append_child("style:tab-stop");
      std::string tab_stop = std::to_string(tab_indent) + "mm";
      style_tab_stop.append_attribute("style:position") = tab_stop.c_str();
      tab_indent++;
    }
  }
}


// This updates the style name of the current paragraph.
// $name: the name of the style, e.g. 'p'.
void odf_text::update_current_paragraph_style (std::string name)
{
  if (!m_current_text_p_node_opened) new_paragraph ();
  current_text_p_node.remove_attribute (current_text_p_node_style_name);
  current_text_p_node_style_name = current_text_p_node.append_attribute ("text:style-name");
  current_text_p_node_style_name = convert_style_name (name).c_str();
}


// This opens a text style.
// $style: the object with the style variables.
// $note: Whether this refers to notes.
// $embed: boolean: Whether nest $style in an existing character style.
void odf_text::open_text_style (Database_Styles_Item style, bool note, bool embed)
{
  std::string marker = style.marker;
  if (find (created_styles.begin(), created_styles.end(), marker) == created_styles.end()) {
    int italic = style.italic;
    int bold = style.bold;
    int underline = style.underline;
    int smallcaps = style.smallcaps;
    int superscript = style.superscript;
    std::string color = style.color;
    std::string backgroundcolor = style.backgroundcolor;
    created_styles.push_back (marker);

    // The style entry looks like this in styles.xml, e.g., for italic:
    // <style:style style:name="T1" style:family="text">
    // <style:text-properties fo:font-style="italic" style:font-style-asian="italic" style:font-style-complex="italic"/>
    // </style:style>
    pugi::xml_node style_dom_element = office_styles_node.append_child ("style:style");
    style_dom_element.append_attribute ("style:name") = convert_style_name (marker).c_str();
    style_dom_element.append_attribute ("style:display-name") = marker.c_str();
    style_dom_element.append_attribute ("style:family") = "text";

    pugi::xml_node style_text_properties_dom_element = style_dom_element.append_child ("style:text-properties");

    // Italics, bold, underline, small caps can be ooitOff or ooitOn or ooitInherit or ooitToggle.
    // Not all features are implemented.
    if ((italic == ooitOn) || (italic == ooitToggle)) {
      style_text_properties_dom_element.append_attribute ("fo:font-style") = "italic";
      style_text_properties_dom_element.append_attribute ("style:font-style-asian") = "italic";
      style_text_properties_dom_element.append_attribute ("style:font-style-complex") = "italic";
    }
    if ((bold == ooitOn) || (bold == ooitToggle)) {
      style_text_properties_dom_element.append_attribute ("fo:font-weight") = "bold";
      style_text_properties_dom_element.append_attribute ("style:font-weight-asian") = "bold";
      style_text_properties_dom_element.append_attribute ("style:font-weight-complex") = "bold";
    }
    if ((underline == ooitOn) || (underline == ooitToggle)) {
      style_text_properties_dom_element.append_attribute ("style:text-underline-style") = "solid";
      style_text_properties_dom_element.append_attribute ("style:text-underline-width") = "auto";
      style_text_properties_dom_element.append_attribute ("style:text-underline-color") = "font-color";
    }
    if ((smallcaps == ooitOn) || (smallcaps == ooitToggle)) {
      style_text_properties_dom_element.append_attribute ("fo:font-variant") = "small-caps";
    }

    if (superscript) {
      //$styleTextPropertiesDomElement->setAttribute ("style:text-position", "super 58%");
      // If the percentage is not specified, an appropriate font height is used.
      style_text_properties_dom_element.append_attribute ("style:text-position") = "super";
      // The mere setting of the superscript value makes the font smaller. No need to set it manually.
      //$styleTextPropertiesDomElement->setAttribute ("fo:font-size", "87%";
      //$styleTextPropertiesDomElement->setAttribute ("style:font-size-asian", "87%";
      //$styleTextPropertiesDomElement->setAttribute ("style:font-size-complex", "87%";
    }

    if (color != "#000000") {
      style_text_properties_dom_element.append_attribute ("fo:color") = color.c_str();
    }

    if (backgroundcolor != "#FFFFFF") {
      style_text_properties_dom_element.append_attribute ("fo:background-color") = backgroundcolor.c_str();
    }

  }

  if (note) {
    if (!embed) m_current_note_text_style.clear();
    m_current_note_text_style.push_back (marker);
  } else {
    if (!embed) m_current_text_style.clear ();
    m_current_text_style.push_back (marker);
  }
}


// This closes any open text style.
// $note: Whether this refers to notes.
// $embed: boolean: Whether to close embedded style.
void odf_text::close_text_style (bool note, bool embed)
{
  if (note) {
    if (!embed) m_current_note_text_style.clear();
    if (!m_current_note_text_style.empty ()) m_current_note_text_style.pop_back ();
  } else {
    if (!embed) m_current_text_style.clear();
    if (!m_current_text_style.empty()) m_current_text_style.pop_back ();
  }
}


// This places text in a frame in OpenDocument.
// It does all the housekeeping to get it display properly.
// $text - the text to place in the frame.
// $style - the name of the style of the $text.
// $fontsize - given in points.
// $italic, $bold - integer values.
void odf_text::place_text_in_frame (std::string text, std::string style, float fontsize, int italic, int bold)
{
  // Empty text is discarded.
  if (text.empty ()) return;

  // The frame goes in an existing paragraph (text:p) element, just like a 'text:span' element.
  // Ensure that a paragraph is open.
  if (!m_current_text_p_node_opened) new_paragraph ();

  // The frame looks like this, in content.xml:
  // <draw:frame draw:style-name="fr1" draw:name="frame1" text:anchor-type="paragraph" svg:y="0cm" fo:min-width="0.34cm" draw:z-index="0">
  //   <draw:text-box fo:min-height="0.34cm">
  //     <text:p text:style-name="c">1</text:p>
  //   </draw:text-box>
  // </draw:frame>
  pugi::xml_node draw_frame_dom_element = current_text_p_node.append_child ("draw:frame");
  draw_frame_dom_element.append_attribute ("draw:style-name") = "chapterframe";
  m_frame_count++;
  draw_frame_dom_element.append_attribute ("draw:name") = filter::strings::convert_to_string ("frame" + std::to_string (m_frame_count)).c_str();
  draw_frame_dom_element.append_attribute ("text:anchor-type") = "paragraph";
  draw_frame_dom_element.append_attribute ("svg:y") = "0cm";
  draw_frame_dom_element.append_attribute ("fo:min-width") = "0.34cm";
  draw_frame_dom_element.append_attribute ("draw:z-index") = "0";

  pugi::xml_node draw_text_box_dom_element = draw_frame_dom_element.append_child ("draw:text-box");
  draw_text_box_dom_element.append_attribute ("fo:min-height") = "0.34cm";

  pugi::xml_node text_p_dom_element = draw_text_box_dom_element.append_child ("text:p");
  text_p_dom_element.append_attribute ("text:style-name") = convert_style_name (style).c_str();
  text_p_dom_element.text().set( filter::strings::escape_special_xml_characters (text).c_str());

  // File styles.xml contains the appropriate styles for this frame and text box and paragraph.
  // Create the styles once for the whole document.
  if (find (created_styles.begin(), created_styles.end (), style) == created_styles.end()) {
    created_styles.push_back (style);
    
    {
      // The style for the text:p element looks like this:
      // <style:style style:name="c" style:family="paragraph">
      //   <style:paragraph-properties fo:text-align="justify" style:justify-single-word="false"/>
      //   <style:text-properties fo:font-size="24pt" fo:font-weight="bold" style:font-size-asian="24pt" style:font-weight-asian="bold" style:font-size-complex="24pt" style:font-weight-complex="bold"/>
      // </style:style>
      pugi::xml_node style_dom_element = office_styles_node.append_child ("style:style");
      style_dom_element.append_attribute ("style:name") = convert_style_name (style).c_str();
      style_dom_element.append_attribute ("style:family") = "paragraph";
  
      pugi::xml_node style_paragraph_properties_dom_element = style_dom_element.append_child ("style:paragraph-properties");
      style_paragraph_properties_dom_element.append_attribute ("fo:text-align") = "justify";
      style_paragraph_properties_dom_element.append_attribute ("style:justify-single-word") = "false";
  
      pugi::xml_node style_text_properties_dom_element = style_dom_element.append_child ("style:text-properties");
      std::string sfontsize = filter::strings::convert_to_string (fontsize) + "pt";
      style_text_properties_dom_element.append_attribute ("fo:font-size") = sfontsize.c_str();
      style_text_properties_dom_element.append_attribute ("style:font-size-asian") = sfontsize.c_str();
      style_text_properties_dom_element.append_attribute ("style:font-size-complex") = sfontsize.c_str();
      if (italic != ooitOff) {
        style_text_properties_dom_element.append_attribute ("fo:font-style") = "italic";
        style_text_properties_dom_element.append_attribute ("style:font-style-asian") = "italic";
        style_text_properties_dom_element.append_attribute ("style:font-style-complex") = "italic";
      }
      if (bold != ooitOff) {
        style_text_properties_dom_element.append_attribute ("fo:font-weight") = "bold";
        style_text_properties_dom_element.append_attribute ("style:font-weight-asian") = "bold";
        style_text_properties_dom_element.append_attribute ("style:font-weight-complex") = "bold";
      }
    }
    {
      // The style for the draw:frame element looks like this:
      // <style:style style:name="chapterframe" style:family="graphic" style:parent-style-name="ChapterFrameParent">
      //   <style:graphic-properties fo:margin-left="0cm" fo:margin-right="0.199cm" fo:margin-top="0cm" fo:margin-bottom="0cm" style:vertical-pos="from-top" style:vertical-rel="paragraph-content" style:horizontal-pos="left" style:horizontal-rel="paragraph" fo:background-color="transparent" style:background-transparency="100%" fo:padding="0cm" fo:border="none" style:shadow="none" style:flow-with-text="true">
      //   <style:background-image/>
      //   </style:graphic-properties>
      // </style:style>
      pugi::xml_node style_dom_element = office_styles_node.append_child ("style:style");
      style_dom_element.append_attribute ("style:name") = "chapterframe";
      style_dom_element.append_attribute ("style:family") = "graphic";
  
      pugi::xml_node style_graphic_properties_dom_element = style_dom_element.append_child ("style:graphic-properties");
      style_graphic_properties_dom_element.append_attribute ("fo:margin-left") = "0cm";
      style_graphic_properties_dom_element.append_attribute ("fo:margin-right") = "0.2cm";
      style_graphic_properties_dom_element.append_attribute ("fo:margin-top") = "0cm";
      style_graphic_properties_dom_element.append_attribute ("fo:margin-bottom") = "0cm";
      style_graphic_properties_dom_element.append_attribute ("style:vertical-pos") = "from-top";
      style_graphic_properties_dom_element.append_attribute ("style:vertical-rel") = "paragraph-content";
      style_graphic_properties_dom_element.append_attribute ("style:horizontal-pos") = "left";
      style_graphic_properties_dom_element.append_attribute ("style:horizontal-rel") = "paragraph";
      style_graphic_properties_dom_element.append_attribute ("fo:background-color") = "transparent";
      style_graphic_properties_dom_element.append_attribute ("style:background-transparency") = "100%";
      style_graphic_properties_dom_element.append_attribute ("fo:padding") = "0cm";
      style_graphic_properties_dom_element.append_attribute ("fo:border") = "none";
      style_graphic_properties_dom_element.append_attribute ("style:shadow") = "none";
      style_graphic_properties_dom_element.append_attribute ("style:flow-with-text") = "true";
    }
  }

}


// This creates the superscript style.
void odf_text::create_superscript_style ()
{
  // The style entry looks like this in styles.xml:
  // <style:style style:name="superscript" style:family="text">
  //   <style:text-properties style:text-position="super 58%"/>
  // </style:style>
  pugi::xml_node style_dom_element = office_styles_node.append_child ("style:style");
  style_dom_element.append_attribute ("style:name") = "superscript";
  style_dom_element.append_attribute ("style:family") = "text";

  pugi::xml_node style_text_properties_dom_element = style_dom_element.append_child ("style:text-properties");
  //$styleTextPropertiesDomElement->setAttribute ("style:text-position", "super 58%");
  // If the percentage is not specified, an appropriate font height is used.
  style_text_properties_dom_element.append_attribute ("style:text-position") = "super";
  // Setting the superscript attribute automatically makes the font smaller. No need to set it manually.
  //$styleTextPropertiesDomElement->setAttribute ("fo:font-size", "87%";
  //$styleTextPropertiesDomElement->setAttribute ("style:font-size-asian", "87%";
  //$styleTextPropertiesDomElement->setAttribute ("style:font-size-complex", "87%";
}


// This function adds a note to the current paragraph.
// $caller: The text of the note caller, that is, the note citation.
// $style: Style name for the paragraph in the footnote body.
// $endnote: Whether this is a footnote and cross reference (false), or an endnote (true).
void odf_text::add_note (std::string caller, std::string style, bool endnote)
{
  // Ensure that a paragraph is open, so that the note can be added to it.
  if (!m_current_text_p_node_opened) new_paragraph ();

  pugi::xml_node text_note_dom_element = current_text_p_node.append_child ("text:note");
  text_note_dom_element.append_attribute ("text:id") = filter::strings::convert_to_string ("ftn" + std::to_string (m_note_count)).c_str();
  m_note_count++;
  m_note_text_p_opened = true;
  std::string noteclass;
  if (endnote) noteclass = "endnote";
  else noteclass = "footnote";
  text_note_dom_element.append_attribute ("text:note-class") = noteclass.c_str();

  // The note citation, the 'caller' is normally in superscript in the OpenDocument.
  // The default values of the application are used.
  // The Bibledit stylesheet is not consulted.
  // It handles the setting on the export page for having an automatic note caller.
  pugi::xml_node text_note_citation_dom_element = text_note_dom_element.append_child ("text:note-citation");
  if (!automatic_note_caller) {
    text_note_citation_dom_element.append_attribute ("text:label") = filter::strings::escape_special_xml_characters (caller).c_str();
  }
  text_note_citation_dom_element.text().set( filter::strings::escape_special_xml_characters (caller).c_str());

  pugi::xml_node text_note_body_dom_element = text_note_dom_element.append_child ("text:note-body");

  note_text_p_dom_element = text_note_body_dom_element.append_child ("text:p");
  note_text_p_dom_element.append_attribute ("text:style-name") = convert_style_name (style).c_str();

  close_text_style (true, false);
}


// This function adds text to the current footnote.
// $text: The text to add.
void odf_text::add_note_text (std::string text)
{
  // Bail out if there's no text.
  if (text == "") return;

  // Ensure a note has started.
  if (!m_note_text_p_opened) add_note ("?", "");

  // Temporal styles array should have at least one style for the code below to work.
  std::vector <std::string> styles (m_current_note_text_style.begin(), m_current_note_text_style.end());
  if (styles.empty ()) styles.push_back ("");

  // Write a text span element, nesting the second and later ones.
  pugi::xml_node dom_element = note_text_p_dom_element;
  for (std::string style : styles) {
    pugi::xml_node text_span_dom_element = dom_element.append_child ("text:span");
    if (!style.empty()) {
      text_span_dom_element.append_attribute ("text:style-name") = convert_style_name (style).c_str();
    }
    dom_element = text_span_dom_element;
  }
  dom_element.text().set( filter::strings::escape_special_xml_characters (text).c_str());
}


// This function closes the current footnote.
void odf_text::close_current_note ()
{
  close_text_style (true, false);
  m_note_text_p_opened = false;
}


// This creates a heading with styled content.
// $style: A style name.
// $text: Content.
void odf_text::new_named_heading (std::string style, std::string text, bool hide)
{
  // Heading looks like this in content.xml:
  // <text:h text:style-name="Heading_20_1" text:outline-level="1">Text</text:h>
  pugi::xml_node text_h_dom_element = office_text_node.append_child ("text:h");
  text_h_dom_element.append_attribute ("text:style-name") = convert_style_name (style).c_str();
  text_h_dom_element.append_attribute ("text:outline-level") = "1";
  text_h_dom_element.text().set(filter::strings::escape_special_xml_characters (text).c_str());

  // Heading style looks like this in styles.xml:
  // <style:style style:name="Heading_20_1" style:display-name="Heading 1" style:family="paragraph" style:parent-style-name="Heading" style:next-style-name="Text_20_body" style:default-outline-level="1" style:class="text">
  // <style:text-properties fo:font-size="115%" fo:font-weight="bold" style:font-size-asian="115%" style:font-weight-asian="bold" style:font-size-complex="115%" style:font-weight-complex="bold"/>
  // </style:style>
  // Create the style if it does not yet exist.
  if (find (created_styles.begin(), created_styles.end (), style) == created_styles.end()) {
    pugi::xml_node style_dom_element = office_styles_node.append_child ("style:style");
    style_dom_element.append_attribute ("style:name") = convert_style_name (style).c_str();
    style_dom_element.append_attribute ("style:display-name") = style.c_str();
    style_dom_element.append_attribute ("style:family") = "paragraph";
    style_dom_element.append_attribute ("style:parent-style-name") = "Heading";
    style_dom_element.append_attribute ("style:next-style-name") = "Text_20_body";
    style_dom_element.append_attribute ("style:default-outline-level") = "1";
    style_dom_element.append_attribute ("style:class") = "text";
    {
      pugi::xml_node style_text_properties_dom_element = style_dom_element.append_child ("style:text-properties");
      style_text_properties_dom_element.append_attribute ("fo:font-size") = "115%";
      style_text_properties_dom_element.append_attribute ("fo:font-weight") = "bold";
      style_text_properties_dom_element.append_attribute ("style:font-size-asian") = "115%";
      style_text_properties_dom_element.append_attribute ("style:font-weight-asian") = "bold";
      style_text_properties_dom_element.append_attribute ("style:font-size-complex") = "115%";
      style_text_properties_dom_element.append_attribute ("style:font-weight-complex") = "bold";
      if (hide) {
        style_text_properties_dom_element.append_attribute ("text:display") = "none";
      }
    }
    created_styles.push_back (style);
  }

  // Make paragraph null, so that adding subsequent text creates a new paragraph.
  m_current_text_p_node_opened = false;
  m_current_paragraph_style.clear ();
  m_current_paragraph_content.clear ();
}


// This converts the name of a style so that it is fit for use in OpenDocument files.
// E.g. 'Heading 1' becomes 'Heading_20_1'
// $style: Input
// It returns the converted style name.
std::string odf_text::convert_style_name (std::string style)
{
  style = filter::strings::replace (" ", "_20_", style);
  return style;
}


// This saves the OpenDocument to file
// $name: the name of the file to save to.
void odf_text::save (std::string name)
{
  // Create the content.xml file.
  // No formatting because some white space is processed.
  std::string content_xml_path = filter_url_create_path ({unpacked_odt_folder, "content.xml"});
  std::stringstream content_xml;
  content_dom.print (content_xml, "", pugi::format_raw);
  filter_url_file_put_contents (content_xml_path, content_xml.str ());

  // Create the styles.xml file.
  // No formatting because some white space is processed.
  std::string styles_xml_path = filter_url_create_path ({unpacked_odt_folder, "styles.xml"});
  std::stringstream styles_xml;
  styles_dom.print (styles_xml, "", pugi::format_raw);
  filter_url_file_put_contents (styles_xml_path, styles_xml.str ());

  // Save the OpenDocument file.
  std::string zippedfile = filter_archive_zip_folder (unpacked_odt_folder);
  filter_url_file_put_contents (name, filter_url_file_get_contents (zippedfile));
  filter_url_unlink (zippedfile);
}


// Add an image to the document.
// <text:p text:style-name="p">
//   <draw:frame draw:style-name="fr1" draw:name="Image1" text:anchor-type="char" svg:width="180mm" svg:height="66.55mm" draw:z-index="0">
//     <draw:image xlink:href="../bibleimage2.png" xlink:type="simple" xlink:show="embed" xlink:actuate="onLoad" draw:filter-name="&lt;All formats&gt;" draw:mime-type="image/png" />
//   </draw:frame>
// </text:p>
void odf_text::add_image (std::string style, [[maybe_unused]] std::string alt, std::string src, std::string caption)
{
  // The parent paragraph for the image has the "p" style.
  current_text_p_node = office_text_node.append_child ("text:p");
  current_text_p_node_style_name = current_text_p_node.append_attribute ("text:style-name") = "p";
  m_current_text_p_node_opened = true;
  m_current_paragraph_style = style;
  m_current_paragraph_content.clear();

  // Get the width and height of the image in pixels.
  int image_width_pixels {0};
  int image_height_pixels {0};
  {
    std::string path = filter_url_create_root_path ({filter_url_temp_dir (), "image_contents"});
    std::string contents = database::bible_images::get(src);
    filter_url_file_put_contents(path, contents);
    filter_image_get_sizes (path, image_width_pixels, image_height_pixels);
  }

  // Determine the width of the available space so the image width will be equal to that.
  // Then the image height depends on the ratio of the image width and height in pixels.
  int available_width_mm {0};
  int available_height_mm {50};
  {
    available_width_mm = filter::strings::convert_to_int (database::config::bible::get_page_width (m_bible)) - filter::strings::convert_to_int (database::config::bible::get_inner_margin (m_bible)) - filter::strings::convert_to_int (database::config::bible::get_outer_margin (m_bible));
    if (image_width_pixels && image_height_pixels) {
      available_height_mm = available_width_mm * image_height_pixels / image_width_pixels;
    }
  }
  
  {
    m_image_counter++;
    pugi::xml_node draw_frame_node = current_text_p_node.append_child("draw:frame");
    draw_frame_node.append_attribute("draw:style-name") = "fr1";
    draw_frame_node.append_attribute("draw:name") = ("Image" + std::to_string(m_image_counter)).c_str();
    draw_frame_node.append_attribute("text:anchor-type") = "char";
    draw_frame_node.append_attribute("svg:width") = (std::to_string (available_width_mm) + "mm").c_str();
    // draw_frame_node.append_attribute("style:rel-width") = "100%";
    draw_frame_node.append_attribute("svg:height") = (std::to_string (available_height_mm) + "mm").c_str();
    // draw_frame_node.append_attribute("style:rel-height") = "scale";
    draw_frame_node.append_attribute("draw:z-index") = "0";
    {
      pugi::xml_node draw_image_node = draw_frame_node.append_child("draw:image");
      // draw_image_node.append_attribute("xlink:href") = string("Pictures/" + src).c_str();
      draw_image_node.append_attribute("xlink:href") = ("../" + src).c_str();
      draw_image_node.append_attribute("xlink:type") = "simple";
      draw_image_node.append_attribute("xlink:show") = "embed";
      draw_image_node.append_attribute("xlink:actuate") = "onLoad";
      draw_image_node.append_attribute("draw:filter-name") = "&lt;All formats&gt;";
      // The mime type should have been set according to the MIME type of the actual image.
      // But omitting it is the easier solution.
      // LibreOffice can still open the document without any problems.
      //draw_image_node.append_attribute("draw:mime-type") = "image/png";
    }
  }
  
  // Optionally add the caption if given.
  if (!caption.empty()) {
    new_paragraph (style);
    add_text (caption);
//    xml_node text_node = current_text_p_node.append_child(node_pcdata);
//    text_node.set_value(caption.c_str());
  }

  // Save the picture into the Pictures output folder.
  // Later on this was not done,
  // because it would require an updated meta-inf/manifest.xml.
  // <manifest:file-entry manifest:full-path="Pictures/100002010000035C0000013E97D1D9088C414E87.png" manifest:media-type="image/png"/>
  // Another advantage of not including the pictures in the opendocument file is:
  // The OpenDocument file without pictures in them would be smaller as it contains only text.
  {
    //string contents = database::bible_images::get(src);
    //string path = filter_url_create_path(pictures_folder, src);
    //filter_url_file_put_contents(path, contents);
  }

  // Close the current paragraph.
  // Goal: Any text that will be added will be output into a new paragraph.
  m_current_text_p_node_opened = false;
  m_current_paragraph_style.clear ();
  m_current_paragraph_content.clear ();
}


// This function adds a tab to the current paragraph.
void odf_text::add_tab ()
{
  // Ensure a paragraph has started.
  if (!m_current_text_p_node_opened) new_paragraph ();
  
  // Write a text tab element.
  current_text_p_node.append_child ("text:tab");

  // Update public paragraph text.
  m_current_paragraph_content += "\t";
}
