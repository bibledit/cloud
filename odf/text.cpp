/*
Copyright (©) 2003-2022 Teus Benschop.

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


Odf_Text::Odf_Text (string bible_in)
{
  bible = bible_in;
  current_text_p_node_opened = false;
  current_paragraph_style.clear();
  current_paragraph_content.clear();
  currentTextStyle.clear();
  frameCount = 0;
  note_text_p_opened = false;
  noteCount = 0;
  currentNoteTextStyle.clear();
  image_counter = 0;

  // Unpack the .odt template.
  string templateOdf = filter_url_create_root_path ("odf", "template.odt");
  unpackedOdtFolder = filter_archive_unzip (templateOdf);
  filter_url_rmdir (filter_url_create_path (unpackedOdtFolder, "Configurations2"));
  // Create the Pictures folder.
  // pictures_folder = filter_url_create_path (unpackedOdtFolder, "Pictures");
  //filter_url_mkdir(pictures_folder);
  
  initialize_content_xml ();
  initialize_styles_xml ();
}


Odf_Text::~Odf_Text ()
{
  filter_url_rmdir (unpackedOdtFolder);
}


// Build the default content.xml for the template.
void Odf_Text::initialize_content_xml ()
{
  xml_node rootnode = contentDom.append_child ("office:document-content");

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

  xml_node office_scripts = rootnode.append_child ("office:scripts");
  if (office_scripts) {}

  xml_node office_font_face_decls = rootnode.append_child ("office:font-face-decls");
  {
    xml_node childnode;

    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Times New Roman";
    childnode.append_attribute ("svg:font-family") = "'Times New Roman'";
    childnode.append_attribute ("style:font-family-generic") = "roman";
    childnode.append_attribute ("style:font-pitch") = "variable";
    
    string fontname = Database_Config_Bible::getExportFont (bible);
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

  xml_node office_automatic_styles = rootnode.append_child ("office:automatic-styles");
  {
    xml_node style_style;
    style_style = office_automatic_styles.append_child ("style:style");
    style_style.append_attribute ("style:name") = "P1";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = "Header";
    {
      xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      {
        xml_node style_tab_stops = style_paragraph_properties.append_child ("style:tab-stops");
        {
          xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
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
      xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      {
        xml_node style_tab_stops = style_paragraph_properties.append_child ("style:tab-stops");
        {
          xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
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
      xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
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

  xml_node office_body = rootnode.append_child ("office:body");
  {
    office_text_node = office_body.append_child ("office:text");
    {
      xml_node text_sequence_decls = office_text_node.append_child ("text:sequence-decls");
      {
        xml_node text_sequence_decl = text_sequence_decls.append_child ("text:sequence-decl");
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
void Odf_Text::initialize_styles_xml ()
{
  xml_node rootnode = stylesDom.append_child ("office:document-styles");

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

  xml_node office_font_face_decls = rootnode.append_child ("office:font-face-decls");
  {
    xml_node childnode;

    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Times New Roman";
    childnode.append_attribute ("svg:font-family") = "'Times New Roman'";
    childnode.append_attribute ("style:font-family-generic") = "roman";
    childnode.append_attribute ("style:font-pitch") = "variable";
  
    string fontname = Database_Config_Bible::getExportFont (bible);
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
    xml_node style_default_style = office_styles_node.append_child ("style:default-style");
    style_default_style.append_attribute ("style:family") = "graphic";
    xml_node style_paragraph_properties = style_default_style.append_child ("style:paragraph-properties");
    if (style_paragraph_properties) {}
    xml_node style_text_properties = style_default_style.append_child ("style:text-properties");
    if (style_text_properties) {}
  }
  {
    xml_node style_default_style = office_styles_node.append_child ("style:default-style");
    style_default_style.append_attribute ("style:family") = "paragraph";
    xml_node style_paragraph_properties = style_default_style.append_child ("style:paragraph-properties");
    if (style_paragraph_properties) {}
    xml_node style_text_properties = style_default_style.append_child ("style:text-properties");
    if (style_text_properties) {}
  }
  {
    xml_node style_default_style = office_styles_node.append_child ("style:default-style");
    style_default_style.append_attribute ("style:family") = "table";
  }
  {
    xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:class") = "text";
  }
  {
    xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = "Heading";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:next-style-name") = "Text_20_body";
    style_style.append_attribute ("style:class") = "text";
    {
      xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      style_paragraph_properties.append_attribute ("fo:margin-top") = "0.423cm";
      style_paragraph_properties.append_attribute ("fo:margin-bottom") = "0.212cm";
      style_paragraph_properties.append_attribute ("fo:keep-with-next") = "always";
    }
  }
  {
    xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = "Text_20_body";
    style_style.append_attribute ("style:display-name") = "Text body";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:class") = "text";
    {
      xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      style_paragraph_properties.append_attribute ("fo:margin-top") = "0cm";
      style_paragraph_properties.append_attribute ("fo:margin-bottom") = "0.212cm";
    }
  }
  {
    xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = "Header";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:class") = "extra";
  }
  {
    xml_node style_style = office_styles_node.append_child ("style:style");
    style_style.append_attribute ("style:name") = "Header_20_left";
    style_style.append_attribute ("style:display-name") = "Header left";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:class") = "extra";
  }

  // Update the tab-stops in the header style. The tab stops depend on page and margin dimensions.
  int centerPosition = convert_to_int (Database_Config_Bible::getPageWidth (bible)) - convert_to_int (Database_Config_Bible::getInnerMargin (bible)) - convert_to_int (Database_Config_Bible::getOuterMargin (bible));

  xml_node office_automatic_styles = rootnode.append_child ("office:automatic-styles");
  {
    xml_node style_style = office_automatic_styles.append_child ("style:style");
    style_style.append_attribute ("style:name") = "MP1";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = "Header";
    {
      xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      {
        xml_node style_tab_stops = style_paragraph_properties.append_child ("style:tab-stops");
        centerPosition /= 2;
        {
          xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = convert_to_string (convert_to_string (centerPosition) + "mm").c_str();
          style_tab_stop.append_attribute ("style:type") = "center";
        }
        {
          xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = convert_to_string (convert_to_string (centerPosition * 2) + "mm").c_str();
          style_tab_stop.append_attribute ("style:type") = "right";
        }
      }
    }
  }
  {
    xml_node style_style = office_automatic_styles.append_child ("style:style");
    style_style.append_attribute ("style:name") = "MP2";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = "Header_20_left";
    {
      xml_node style_paragraph_properties = style_style.append_child ("style:paragraph-properties");
      {
        xml_node style_tab_stops = style_paragraph_properties.append_child ("style:tab-stops");
        {
          xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = convert_to_string (convert_to_string (centerPosition) + "mm").c_str();
          style_tab_stop.append_attribute ("style:type") = "center";
        }
        {
          xml_node style_tab_stop = style_tab_stops.append_child ("style:tab-stop");
          style_tab_stop.append_attribute ("style:position") = convert_to_string (convert_to_string (centerPosition * 2) + "mm").c_str();
          style_tab_stop.append_attribute ("style:type") = "right";
        }
      }
    }
  }
  {
    xml_node style_page_layout = office_automatic_styles.append_child ("style:page-layout");
    style_page_layout.append_attribute ("style:name") = "Mpm1";
    style_page_layout.append_attribute ("style:page-usage") = "mirrored";
    {
      xml_node style_page_layout_properties = style_page_layout.append_child ("style:page-layout-properties");
      // Take the page size and margins from the Bible's settings.
      style_page_layout_properties.append_attribute ("fo:page-width") = convert_to_string (Database_Config_Bible::getPageWidth (bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("fo:page-height") = convert_to_string (Database_Config_Bible::getPageHeight (bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("style:num-format") = "1";
      style_page_layout_properties.append_attribute ("style:print-orientation") = "portrait";
      style_page_layout_properties.append_attribute ("fo:margin-top") = convert_to_string (Database_Config_Bible::getTopMargin (bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("fo:margin-bottom") = convert_to_string (Database_Config_Bible::getBottomMargin (bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("fo:margin-left") = convert_to_string (Database_Config_Bible::getInnerMargin (bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("fo:margin-right") = convert_to_string (Database_Config_Bible::getOuterMargin (bible) + "mm").c_str();
      style_page_layout_properties.append_attribute ("style:writing-mode") = "lr-tb";
      style_page_layout_properties.append_attribute ("style:footnote-max-height") = "0cm";
      {
        xml_node style_footnote_sep = style_page_layout_properties.append_child ("style:footnote-sep");
        style_footnote_sep.append_attribute ("style:width") = "0.018cm";
        style_footnote_sep.append_attribute ("style:distance-before-sep") = "0.101cm";
        style_footnote_sep.append_attribute ("style:distance-after-sep") = "0.101cm";
        style_footnote_sep.append_attribute ("style:adjustment") = "left";
        style_footnote_sep.append_attribute ("style:rel-width") = "25%";
        style_footnote_sep.append_attribute ("style:color") = "#000000";
      }
    }
    {
      xml_node style_header_style = style_page_layout.append_child ("style:header-style");
      {
        xml_node style_header_footer_properties = style_header_style.append_child ("style:header-footer-properties");
        style_header_footer_properties.append_attribute ("fo:min-height") = "0.799cm";
        style_header_footer_properties.append_attribute ("fo:margin-left") = "0cm";
        style_header_footer_properties.append_attribute ("fo:margin-right") = "0cm";
        style_header_footer_properties.append_attribute ("fo:margin-bottom") = "0.3cm";
        style_header_footer_properties.append_attribute ("style:dynamic-spacing") = "false";
      }
      xml_node style_footer_style = style_page_layout.append_child ("style:footer-style");
      if (style_footer_style) {}
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
  xml_node office_master_styles = rootnode.append_child ("office:master-styles");
  {
    xml_node style_master_page = office_master_styles.append_child ("style:master-page");
    style_master_page.append_attribute ("style:name") = styles_logic_standard_sheet ().c_str();
    style_master_page.append_attribute ("style:page-layout-name") = "Mpm1";
    {
      xml_node style_header = style_master_page.append_child ("style:header");
      {
        xml_node text_p = style_header.append_child ("text:p");
        text_p.append_attribute ("text:style-name") = "MP1";
        {
          xml_node node = text_p.append_child ("text:page-number");
          node.append_attribute ("text:select-page") = "current";
          node.text ().set ("1");
        }
        {
          text_p.append_child ("text:tab");
        }
        // Whether and how to put the date in the running headers.
        if (Database_Config_Bible::getDateInHeader (bible)) {
          xml_node node = text_p.append_child ("text:date");
          node.append_attribute ("style:data-style-name") = "N81";
          node.append_attribute ("text:date-value") = "";
          node.text ().set ("");
        }
        {
          text_p.append_child ("text:tab");
        }
        {
          xml_node node = text_p.append_child ("text:chapter");
          node.append_attribute ("text:display") = "name";
          node.append_attribute ("text:outline-level") = "1";
        }
      }
      xml_node style_header_left = style_master_page.append_child ("style:header-left");
      {
        xml_node text_p = style_header_left.append_child ("text:p");
        text_p.append_attribute ("text:style-name") = "MP2";
        {
          xml_node node = text_p.append_child ("text:chapter");
          node.append_attribute ("text:display") = "name";
          node.append_attribute ("text:outline-level") = "1";
        }
        {
          text_p.append_child ("text:tab");
        }
        // Whether and how to put the date in the running headers.
        if (Database_Config_Bible::getDateInHeader (bible)) {
          xml_node node = text_p.append_child ("text:date");
          node.append_attribute ("style:data-style-name") = "N81";
          node.append_attribute ("text:date-value") = "";
          node.text ().set ("");
        }
        {
          text_p.append_child ("text:tab");
        }
        {
          xml_node node = text_p.append_child ("text:page-number");
          node.append_attribute ("text:select-page") = "current";
          node.text ().set ("1");
        }
      }
    }
  }
}


void Odf_Text::new_paragraph (string style)
{
  current_text_p_node = office_text_node.append_child ("text:p");
  current_text_p_node_style_name = current_text_p_node.append_attribute ("text:style-name") = style.c_str();
  current_text_p_node_opened = true;
  current_paragraph_style = style;
  current_paragraph_content.clear();
}


// This function adds text to the current paragraph.
// $text: The text to add.
void Odf_Text::add_text (string text)
{
  // Bail out if there's no text.
  if (text.empty()) return;

  // Ensure a paragraph has started.
  if (!current_text_p_node_opened) new_paragraph ();
  
  // Temporal styles array should have at least one style for the code below to work.
  // So ensure it has at least one style.
  vector <string> styles (currentTextStyle.begin (), currentTextStyle.end ());
  if (styles.empty()) styles.push_back (string());
  
  // Write a text span element, nesting the second and later ones.
  xml_node dom_node = current_text_p_node;
  for (string style : styles) {
    xml_node text_span_node = dom_node.append_child ("text:span");
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
  current_paragraph_content += text;
}


// This creates a heading with contents styled "Heading 1".
// $text: Contents.
void Odf_Text::new_heading1 (string text, bool hide)
{
  newNamedHeading ("Heading 1", text, hide);
}


// This creates the page break style.
void Odf_Text::createPageBreakStyle ()
{
  // This is how the style looks in styles.xml:
  // <style:style style:display-name="Page Break" style:family="paragraph" style:name="Page_20_Break">
  // <style:paragraph-properties fo:break-after="page" fo:line-height="0.05cm" fo:margin-bottom="0cm" fo:margin-top="0cm"/>
  // <style:text-properties fo:font-size="2pt" style:font-size-asian="2pt" style:font-size-complex="2pt"/>
  // </style:style>
  xml_node styleDomElement = office_styles_node.append_child ("style:style");
  styleDomElement.append_attribute ("style:name") = "Page_20_Break";
  styleDomElement.append_attribute ("style:display-name") = "Page Break";
  styleDomElement.append_attribute ("style:family") = "paragraph";

  xml_node styleParagraphPropertiesDomElement = styleDomElement.append_child ("style:paragraph-properties");
  styleParagraphPropertiesDomElement.append_attribute ("fo:break-after") = "page";
  styleParagraphPropertiesDomElement.append_attribute ("fo:line-height") = "0.05cm";
  styleParagraphPropertiesDomElement.append_attribute ("fo:margin-bottom") = "0cm";
  styleParagraphPropertiesDomElement.append_attribute ("fo:margin-top") = "0cm";

  xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");
  styleTextPropertiesDomElement.append_attribute ("fo:font-size") = "2pt";
  styleTextPropertiesDomElement.append_attribute ("style:font-size-asian") = "2pt";
  styleTextPropertiesDomElement.append_attribute ("style:font-size-complex") = "2pt";
}


// This applies a page break.
void Odf_Text::newPageBreak ()
{
  new_paragraph ("Page_20_Break");
  // Always clear the paragraph-opened-flag,
  // because we don't want subsequent text to be added to this page break,
  // since it would be nearly invisible, and thus text would mysteriously get lost.
  current_text_p_node_opened = false;
  current_paragraph_style.clear ();
  current_paragraph_content.clear ();
}


// This creates a paragraph style.
// $name: the name of the style, e.g. 'p'.
// $dropcaps: If 0, there are no drop caps.
//            If greater than 0, it the number of characters in drop caps style.
void Odf_Text::create_paragraph_style (string name,
                                       string fontname,
                                       float fontsize,
                                       int italic, int bold, int underline, int smallcaps,
                                       int alignment,
                                       float spacebefore, float spaceafter,
                                       float leftmargin, float rightmargin,
                                       float firstlineindent,
                                       bool keepWithNext,
                                       int dropcaps)
{
  // Whether to align verse numbers in poetry to the left of the margin,
  // and if so, whether this is one of the defined poetry styles.
  bool is_poetry_q_style = false;
  if (Database_Config_Bible::getOdtPoetryVersesLeft (bible)) {
    is_poetry_q_style = usfm_is_standard_q_poetry (name);
  }
  
  // It looks like this in styles.xml:
  // <style:style style:display-name="p_c1" style:family="paragraph" style:name="p_c1">
  //   <style:paragraph-properties fo:margin-bottom="0mm" fo:margin-left="0mm" fo:margin-right="0mm" fo:margin-top="0mm" fo:text-align="justify" fo:text-indent="0mm"/>
  //     <style:drop-cap style:distance="0.15cm" style:length="1" style:lines="2"/>
  //   <style:paragraph-properties>
  //   <style:text-properties fo:font-size="12pt" style:font-size-asian="12pt" style:font-size-complex="12pt"/>
  // </style:style>
  xml_node style_style_node = office_styles_node.append_child ("style:style");
  style_style_node.append_attribute ("style:name") = convert_style_name (name).c_str();
  style_style_node.append_attribute ("style:display-name") = name.c_str();
  style_style_node.append_attribute ("style:family") = "paragraph";

  xml_node style_paragraph_properties_node = style_style_node.append_child ("style:paragraph-properties");

  xml_node style_text_properties_node = style_style_node.append_child ("style:text-properties");

  style_paragraph_properties_node.append_attribute ("style:font-name") = fontname.c_str();
  fontname.insert (0, "'");
  fontname.append ("'");
  style_text_properties_node.append_attribute ("fo:font-family") = fontname.c_str();

  string sfontsize = convert_to_string (fontsize) + "pt";
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
  string alignmenttext;
  switch (alignment) {
    case AlignmentLeft:    alignmenttext = "start"; break;
    case AlignmentCenter:  alignmenttext = "center"; break;
    case AlignmentRight:   alignmenttext = "end"; break;
    case AlignmentJustify: alignmenttext = "justify"; break;
  }
  style_paragraph_properties_node.append_attribute ("fo:text-align") = alignmenttext.c_str();
  style_paragraph_properties_node.append_attribute ("style:justify-single-word") = "false";

  // Deal with the paragraph dimensions.
  // The values are given in millimeters.
  // First the top and bottom margins.
  string space_before_mm = convert_to_string (spacebefore) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:margin-top") = space_before_mm.c_str();
  string space_after_mm = convert_to_string (spaceafter) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:margin-bottom") = space_after_mm.c_str();
  string left_margin_mm = convert_to_string (leftmargin) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:margin-left") = left_margin_mm.c_str();
  string right_margin_mm = convert_to_string (rightmargin) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:margin-right") = right_margin_mm.c_str();
  // In a normal paragraph the first line indent is as given in the stylesheet.
  // In a poetry paragraph the first line indent is the negative left margin.
  // The goal is that the left is at a 0 left margin,
  // and that the verse is aligned at the very left of the column.
  // (And then a tab puts the text at the desired first line indent space.)
  int millimeters = firstlineindent;
  if (is_poetry_q_style) millimeters = 0 - leftmargin;
  string first_lineindent_mm = convert_to_string (millimeters) + "mm";
  style_paragraph_properties_node.append_attribute ("fo:text-indent") = first_lineindent_mm.c_str();

  if (keepWithNext) {
    style_paragraph_properties_node.append_attribute ("fo:keep-together") = "always";
    style_paragraph_properties_node.append_attribute ("fo:keep-with-next") = "always";
  }

  if (dropcaps > 0) {
    // E.g.: <style:drop-cap style:lines="2" style:length="2" style:distance="0.15cm"/>
    string length = convert_to_string (dropcaps);
    xml_node style_drop_cap_node = style_paragraph_properties_node.append_child ("style:drop-cap");
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
    xml_node style_tab_stops = style_paragraph_properties_node.append_child("style:tab-stops");
    int tab_indent = firstlineindent;
    for (int i = 0; i < 10; i++) {
      xml_node style_tab_stop = style_tab_stops.append_child("style:tab-stop");
      string tab_stop = convert_to_string(tab_indent) + "mm";
      style_tab_stop.append_attribute("style:position") = tab_stop.c_str();
      tab_indent++;
    }
  }
}


// This updates the style name of the current paragraph.
// $name: the name of the style, e.g. 'p'.
void Odf_Text::update_current_paragraph_style (string name)
{
  if (!current_text_p_node_opened) new_paragraph ();
  current_text_p_node.remove_attribute (current_text_p_node_style_name);
  current_text_p_node_style_name = current_text_p_node.append_attribute ("text:style-name");
  current_text_p_node_style_name = convert_style_name (name).c_str();
}


// This opens a text style.
// $style: the object with the style variables.
// $note: Whether this refers to notes.
// $embed: boolean: Whether nest $style in an existing character style.
void Odf_Text::open_text_style (Database_Styles_Item style, bool note, bool embed)
{
  string marker = style.marker;
  if (find (createdStyles.begin(), createdStyles.end(), marker) == createdStyles.end()) {
    int italic = style.italic;
    int bold = style.bold;
    int underline = style.underline;
    int smallcaps = style.smallcaps;
    int superscript = style.superscript;
    string color = style.color;
    string backgroundcolor = style.backgroundcolor;
    createdStyles.push_back (marker);

    // The style entry looks like this in styles.xml, e.g., for italic:
    // <style:style style:name="T1" style:family="text">
    // <style:text-properties fo:font-style="italic" style:font-style-asian="italic" style:font-style-complex="italic"/>
    // </style:style>
    xml_node styleDomElement = office_styles_node.append_child ("style:style");
    styleDomElement.append_attribute ("style:name") = convert_style_name (marker).c_str();
    styleDomElement.append_attribute ("style:display-name") = marker.c_str();
    styleDomElement.append_attribute ("style:family") = "text";

    xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");

    // Italics, bold, underline, small caps can be ooitOff or ooitOn or ooitInherit or ooitToggle.
    // Not all features are implemented.
    if ((italic == ooitOn) || (italic == ooitToggle)) {
      styleTextPropertiesDomElement.append_attribute ("fo:font-style") = "italic";
      styleTextPropertiesDomElement.append_attribute ("style:font-style-asian") = "italic";
      styleTextPropertiesDomElement.append_attribute ("style:font-style-complex") = "italic";
    }
    if ((bold == ooitOn) || (bold == ooitToggle)) {
      styleTextPropertiesDomElement.append_attribute ("fo:font-weight") = "bold";
      styleTextPropertiesDomElement.append_attribute ("style:font-weight-asian") = "bold";
      styleTextPropertiesDomElement.append_attribute ("style:font-weight-complex") = "bold";
    }
    if ((underline == ooitOn) || (underline == ooitToggle)) {
      styleTextPropertiesDomElement.append_attribute ("style:text-underline-style") = "solid";
      styleTextPropertiesDomElement.append_attribute ("style:text-underline-width") = "auto";
      styleTextPropertiesDomElement.append_attribute ("style:text-underline-color") = "font-color";
    }
    if ((smallcaps == ooitOn) || (smallcaps == ooitToggle)) {
      styleTextPropertiesDomElement.append_attribute ("fo:font-variant") = "small-caps";
    }

    if (superscript) {
      //$styleTextPropertiesDomElement->setAttribute ("style:text-position", "super 58%");
      // If the percentage is not specified, an appropriate font height is used.
      styleTextPropertiesDomElement.append_attribute ("style:text-position") = "super";
      // The mere setting of the superscript value makes the font smaller. No need to set it manually.
      //$styleTextPropertiesDomElement->setAttribute ("fo:font-size", "87%";
      //$styleTextPropertiesDomElement->setAttribute ("style:font-size-asian", "87%";
      //$styleTextPropertiesDomElement->setAttribute ("style:font-size-complex", "87%";
    }

    if (color != "#000000") {
      styleTextPropertiesDomElement.append_attribute ("fo:color") = color.c_str();
    }

    if (backgroundcolor != "#FFFFFF") {
      styleTextPropertiesDomElement.append_attribute ("fo:background-color") = backgroundcolor.c_str();
    }

  }

  if (note) {
    if (!embed) currentNoteTextStyle.clear();
    currentNoteTextStyle.push_back (marker);
  } else {
    if (!embed) currentTextStyle.clear ();
    currentTextStyle.push_back (marker);
  }
}


// This closes any open text style.
// $note: Whether this refers to notes.
// $embed: boolean: Whether to close embedded style.
void Odf_Text::closeTextStyle (bool note, bool embed)
{
  if (note) {
    if (!embed) currentNoteTextStyle.clear();
    if (!currentNoteTextStyle.empty ()) currentNoteTextStyle.pop_back ();
  } else {
    if (!embed) currentTextStyle.clear();
    if (!currentTextStyle.empty()) currentTextStyle.pop_back ();
  }
}


// This places text in a frame in OpenDocument.
// It does all the housekeeping to get it display properly.
// $text - the text to place in the frame.
// $style - the name of the style of the $text.
// $fontsize - given in points.
// $italic, $bold - integer values.
void Odf_Text::placeTextInFrame (string text, string style, float fontsize, int italic, int bold)
{
  // Empty text is discarded.
  if (text.empty ()) return;

  // The frame goes in an existing paragraph (text:p) element, just like a 'text:span' element.
  // Ensure that a paragraph is open.
  if (!current_text_p_node_opened) new_paragraph ();

  // The frame looks like this, in content.xml:
  // <draw:frame draw:style-name="fr1" draw:name="frame1" text:anchor-type="paragraph" svg:y="0cm" fo:min-width="0.34cm" draw:z-index="0">
  //   <draw:text-box fo:min-height="0.34cm">
  //     <text:p text:style-name="c">1</text:p>
  //   </draw:text-box>
  // </draw:frame>
  xml_node drawFrameDomElement = current_text_p_node.append_child ("draw:frame");
  drawFrameDomElement.append_attribute ("draw:style-name") = "chapterframe";
  frameCount++;
  drawFrameDomElement.append_attribute ("draw:name") = convert_to_string ("frame" + convert_to_string (frameCount)).c_str();
  drawFrameDomElement.append_attribute ("text:anchor-type") = "paragraph";
  drawFrameDomElement.append_attribute ("svg:y") = "0cm";
  drawFrameDomElement.append_attribute ("fo:min-width") = "0.34cm";
  drawFrameDomElement.append_attribute ("draw:z-index") = "0";

  xml_node drawTextBoxDomElement = drawFrameDomElement.append_child ("draw:text-box");
  drawTextBoxDomElement.append_attribute ("fo:min-height") = "0.34cm";

  xml_node textPDomElement = drawTextBoxDomElement.append_child ("text:p");
  textPDomElement.append_attribute ("text:style-name") = convert_style_name (style).c_str();
  textPDomElement.text().set( escape_special_xml_characters (text).c_str());

  // File styles.xml contains the appropriate styles for this frame and text box and paragraph.
  // Create the styles once for the whole document.
  if (find (createdStyles.begin(), createdStyles.end (), style) == createdStyles.end()) {
    createdStyles.push_back (style);
    
    {
      // The style for the text:p element looks like this:
      // <style:style style:name="c" style:family="paragraph">
      //   <style:paragraph-properties fo:text-align="justify" style:justify-single-word="false"/>
      //   <style:text-properties fo:font-size="24pt" fo:font-weight="bold" style:font-size-asian="24pt" style:font-weight-asian="bold" style:font-size-complex="24pt" style:font-weight-complex="bold"/>
      // </style:style>
      xml_node styleDomElement = office_styles_node.append_child ("style:style");
      styleDomElement.append_attribute ("style:name") = convert_style_name (style).c_str();
      styleDomElement.append_attribute ("style:family") = "paragraph";
  
      xml_node styleParagraphPropertiesDomElement = styleDomElement.append_child ("style:paragraph-properties");
      styleParagraphPropertiesDomElement.append_attribute ("fo:text-align") = "justify";
      styleParagraphPropertiesDomElement.append_attribute ("style:justify-single-word") = "false";
  
      xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");
      string sfontsize = convert_to_string (fontsize) + "pt";
      styleTextPropertiesDomElement.append_attribute ("fo:font-size") = sfontsize.c_str();
      styleTextPropertiesDomElement.append_attribute ("style:font-size-asian") = sfontsize.c_str();
      styleTextPropertiesDomElement.append_attribute ("style:font-size-complex") = sfontsize.c_str();
      if (italic != ooitOff) {
        styleTextPropertiesDomElement.append_attribute ("fo:font-style") = "italic";
        styleTextPropertiesDomElement.append_attribute ("style:font-style-asian") = "italic";
        styleTextPropertiesDomElement.append_attribute ("style:font-style-complex") = "italic";
      }
      if (bold != ooitOff) {
        styleTextPropertiesDomElement.append_attribute ("fo:font-weight") = "bold";
        styleTextPropertiesDomElement.append_attribute ("style:font-weight-asian") = "bold";
        styleTextPropertiesDomElement.append_attribute ("style:font-weight-complex") = "bold";
      }
    }
    {
      // The style for the draw:frame element looks like this:
      // <style:style style:name="chapterframe" style:family="graphic" style:parent-style-name="ChapterFrameParent">
      //   <style:graphic-properties fo:margin-left="0cm" fo:margin-right="0.199cm" fo:margin-top="0cm" fo:margin-bottom="0cm" style:vertical-pos="from-top" style:vertical-rel="paragraph-content" style:horizontal-pos="left" style:horizontal-rel="paragraph" fo:background-color="transparent" style:background-transparency="100%" fo:padding="0cm" fo:border="none" style:shadow="none" style:flow-with-text="true">
      //   <style:background-image/>
      //   </style:graphic-properties>
      // </style:style>
      xml_node styleDomElement = office_styles_node.append_child ("style:style");
      styleDomElement.append_attribute ("style:name") = "chapterframe";
      styleDomElement.append_attribute ("style:family") = "graphic";
  
      xml_node styleGraphicPropertiesDomElement = styleDomElement.append_child ("style:graphic-properties");
      styleGraphicPropertiesDomElement.append_attribute ("fo:margin-left") = "0cm";
      styleGraphicPropertiesDomElement.append_attribute ("fo:margin-right") = "0.2cm";
      styleGraphicPropertiesDomElement.append_attribute ("fo:margin-top") = "0cm";
      styleGraphicPropertiesDomElement.append_attribute ("fo:margin-bottom") = "0cm";
      styleGraphicPropertiesDomElement.append_attribute ("style:vertical-pos") = "from-top";
      styleGraphicPropertiesDomElement.append_attribute ("style:vertical-rel") = "paragraph-content";
      styleGraphicPropertiesDomElement.append_attribute ("style:horizontal-pos") = "left";
      styleGraphicPropertiesDomElement.append_attribute ("style:horizontal-rel") = "paragraph";
      styleGraphicPropertiesDomElement.append_attribute ("fo:background-color") = "transparent";
      styleGraphicPropertiesDomElement.append_attribute ("style:background-transparency") = "100%";
      styleGraphicPropertiesDomElement.append_attribute ("fo:padding") = "0cm";
      styleGraphicPropertiesDomElement.append_attribute ("fo:border") = "none";
      styleGraphicPropertiesDomElement.append_attribute ("style:shadow") = "none";
      styleGraphicPropertiesDomElement.append_attribute ("style:flow-with-text") = "true";
    }
  }

}


// This creates the superscript style.
void Odf_Text::createSuperscriptStyle ()
{
  // The style entry looks like this in styles.xml:
  // <style:style style:name="superscript" style:family="text">
  //   <style:text-properties style:text-position="super 58%"/>
  // </style:style>
  xml_node styleDomElement = office_styles_node.append_child ("style:style");
  styleDomElement.append_attribute ("style:name") = "superscript";
  styleDomElement.append_attribute ("style:family") = "text";

  xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");
  //$styleTextPropertiesDomElement->setAttribute ("style:text-position", "super 58%");
  // If the percentage is not specified, an appropriate font height is used.
  styleTextPropertiesDomElement.append_attribute ("style:text-position") = "super";
  // Setting the superscript attribute automatically makes the font smaller. No need to set it manually.
  //$styleTextPropertiesDomElement->setAttribute ("fo:font-size", "87%";
  //$styleTextPropertiesDomElement->setAttribute ("style:font-size-asian", "87%";
  //$styleTextPropertiesDomElement->setAttribute ("style:font-size-complex", "87%";
}


// This function adds a note to the current paragraph.
// $caller: The text of the note caller, that is, the note citation.
// $style: Style name for the paragraph in the footnote body.
// $endnote: Whether this is a footnote and cross reference (false), or an endnote (true).
void Odf_Text::addNote (string caller, string style, bool endnote)
{
  // Ensure that a paragraph is open, so that the note can be added to it.
  if (!current_text_p_node_opened) new_paragraph ();

  xml_node textNoteDomElement = current_text_p_node.append_child ("text:note");
  textNoteDomElement.append_attribute ("text:id") = convert_to_string ("ftn" + convert_to_string (noteCount)).c_str();
  noteCount++;
  note_text_p_opened = true;
  string noteclass;
  if (endnote) noteclass = "endnote";
  else noteclass = "footnote";
  textNoteDomElement.append_attribute ("text:note-class") = noteclass.c_str();

  // The note citation, the 'caller' is normally in superscript in the OpenDocument.
  // The default values of the application are used.
  // The Bibledit stylesheet is not consulted.
  xml_node textNoteCitationDomElement = textNoteDomElement.append_child ("text:note-citation");
  textNoteCitationDomElement.append_attribute ("text:label") = escape_special_xml_characters (caller).c_str();
  textNoteCitationDomElement.text().set( escape_special_xml_characters (caller).c_str());

  xml_node textNoteBodyDomElement = textNoteDomElement.append_child ("text:note-body");

  noteTextPDomElement = textNoteBodyDomElement.append_child ("text:p");
  noteTextPDomElement.append_attribute ("text:style-name") = convert_style_name (style).c_str();

  closeTextStyle (true, false);
}


// This function adds text to the current footnote.
// $text: The text to add.
void Odf_Text::addNoteText (string text)
{
  // Bail out if there's no text.
  if (text == "") return;

  // Ensure a note has started.
  if (!note_text_p_opened) addNote ("?", "");

  // Temporal styles array should have at least one style for the code below to work.
  vector <string> styles (currentNoteTextStyle.begin(), currentNoteTextStyle.end());
  if (styles.empty ()) styles.push_back ("");

  // Write a text span element, nesting the second and later ones.
  xml_node domElement = noteTextPDomElement;
  for (string style : styles) {
    xml_node textSpanDomElement = domElement.append_child ("text:span");
    if (!style.empty()) {
      textSpanDomElement.append_attribute ("text:style-name") = convert_style_name (style).c_str();
    }
    domElement = textSpanDomElement;
  }
  domElement.text().set( escape_special_xml_characters (text).c_str());
}


// This function closes the current footnote.
void Odf_Text::closeCurrentNote ()
{
  closeTextStyle (true, false);
  note_text_p_opened = false;
}


// This creates a heading with styled content.
// $style: A style name.
// $text: Content.
void Odf_Text::newNamedHeading (string style, string text, bool hide)
{
  // Heading looks like this in content.xml:
  // <text:h text:style-name="Heading_20_1" text:outline-level="1">Text</text:h>
  xml_node textHDomElement = office_text_node.append_child ("text:h");
  textHDomElement.append_attribute ("text:style-name") = convert_style_name (style).c_str();
  textHDomElement.append_attribute ("text:outline-level") = "1";
  textHDomElement.text().set(escape_special_xml_characters (text).c_str());

  // Heading style looks like this in styles.xml:
  // <style:style style:name="Heading_20_1" style:display-name="Heading 1" style:family="paragraph" style:parent-style-name="Heading" style:next-style-name="Text_20_body" style:default-outline-level="1" style:class="text">
  // <style:text-properties fo:font-size="115%" fo:font-weight="bold" style:font-size-asian="115%" style:font-weight-asian="bold" style:font-size-complex="115%" style:font-weight-complex="bold"/>
  // </style:style>
  // Create the style if it does not yet exist.
  if (find (createdStyles.begin(), createdStyles.end (), style) == createdStyles.end()) {
    xml_node styleDomElement = office_styles_node.append_child ("style:style");
    styleDomElement.append_attribute ("style:name") = convert_style_name (style).c_str();
    styleDomElement.append_attribute ("style:display-name") = style.c_str();
    styleDomElement.append_attribute ("style:family") = "paragraph";
    styleDomElement.append_attribute ("style:parent-style-name") = "Heading";
    styleDomElement.append_attribute ("style:next-style-name") = "Text_20_body";
    styleDomElement.append_attribute ("style:default-outline-level") = "1";
    styleDomElement.append_attribute ("style:class") = "text";
    {
      xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");
      styleTextPropertiesDomElement.append_attribute ("fo:font-size") = "115%";
      styleTextPropertiesDomElement.append_attribute ("fo:font-weight") = "bold";
      styleTextPropertiesDomElement.append_attribute ("style:font-size-asian") = "115%";
      styleTextPropertiesDomElement.append_attribute ("style:font-weight-asian") = "bold";
      styleTextPropertiesDomElement.append_attribute ("style:font-size-complex") = "115%";
      styleTextPropertiesDomElement.append_attribute ("style:font-weight-complex") = "bold";
      if (hide) {
        styleTextPropertiesDomElement.append_attribute ("text:display") = "none";
      }
    }
    createdStyles.push_back (style);
  }

  // Make paragraph null, so that adding subsequent text creates a new paragraph.
  current_text_p_node_opened = false;
  current_paragraph_style.clear ();
  current_paragraph_content.clear ();
}


// This converts the name of a style so that it is fit for use in OpenDocument files.
// E.g. 'Heading 1' becomes 'Heading_20_1'
// $style: Input
// It returns the converted style name.
string Odf_Text::convert_style_name (string style)
{
  style = filter_string_str_replace (" ", "_20_", style);
  return style;
}


// This saves the OpenDocument to file
// $name: the name of the file to save to.
void Odf_Text::save (string name)
{
  // Create the content.xml file.
  // No formatting because some white space is processed.
  string contentXmlPath = filter_url_create_path (unpackedOdtFolder, "content.xml");
  stringstream contentXml;
  contentDom.print (contentXml, "", format_raw);
  filter_url_file_put_contents (contentXmlPath, contentXml.str ());

  // Create the styles.xml file.
  // No formatting because some white space is processed.
  string stylesXmlPath = filter_url_create_path (unpackedOdtFolder, "styles.xml");
  stringstream stylesXml;
  stylesDom.print (stylesXml, "", format_raw);
  filter_url_file_put_contents (stylesXmlPath, stylesXml.str ());

  // Save the OpenDocument file.
  string zippedfile = filter_archive_zip_folder (unpackedOdtFolder);
  filter_url_file_put_contents (name, filter_url_file_get_contents (zippedfile));
  filter_url_unlink_cpp17 (zippedfile);
}


// Add an image to the document.
// <text:p text:style-name="p">
//   <draw:frame draw:style-name="fr1" draw:name="Image1" text:anchor-type="char" svg:width="180mm" svg:height="66.55mm" draw:z-index="0">
//     <draw:image xlink:href="../bibleimage2.png" xlink:type="simple" xlink:show="embed" xlink:actuate="onLoad" draw:filter-name="&lt;All formats&gt;" draw:mime-type="image/png" />
//   </draw:frame>
// </text:p>
void Odf_Text::add_image ([[maybe_unused]] string alt, string src, string caption)
{
  // The parent paragraph for the image has the "p" style.
  const char * style = "p";
  current_text_p_node = office_text_node.append_child ("text:p");
  current_text_p_node_style_name = current_text_p_node.append_attribute ("text:style-name") = style;
  current_text_p_node_opened = true;
  current_paragraph_style = style;
  current_paragraph_content.clear();

  // Get the width and height of the image in pixels.
  int image_width_pixels = 0, image_height_pixels = 0;
  {
    Database_BibleImages database_bibleimages;
    string path = filter_url_create_root_path (filter_url_temp_dir (), "image_contents");
    string contents = database_bibleimages.get(src);
    filter_url_file_put_contents(path, contents);
    filter_image_get_sizes (path, image_width_pixels, image_height_pixels);

  }

  // Determine the width of the available space so the image width will be equal to that.
  // Then the image height depends on the ratio of the image width and height in pixels.
  int available_width_mm = 0;
  int available_height_mm = 50;
  {
    available_width_mm = convert_to_int (Database_Config_Bible::getPageWidth (bible)) - convert_to_int (Database_Config_Bible::getInnerMargin (bible)) - convert_to_int (Database_Config_Bible::getOuterMargin (bible));
    if (image_width_pixels && image_height_pixels) {
      available_height_mm = available_width_mm * image_height_pixels / image_width_pixels;
    }
  }
  
  {
    image_counter++;
    xml_node draw_frame_node = current_text_p_node.append_child("draw:frame");
    draw_frame_node.append_attribute("draw:style-name") = "fr1";
    draw_frame_node.append_attribute("draw:name") = string ("Image" + convert_to_string(image_counter)).c_str();
    draw_frame_node.append_attribute("text:anchor-type") = "char";
    draw_frame_node.append_attribute("svg:width") = string(convert_to_string (available_width_mm) + "mm").c_str();
    // draw_frame_node.append_attribute("style:rel-width") = "100%";
    draw_frame_node.append_attribute("svg:height") = string (convert_to_string (available_height_mm) + "mm").c_str();
    // draw_frame_node.append_attribute("style:rel-height") = "scale";
    draw_frame_node.append_attribute("draw:z-index") = "0";
    {
      xml_node draw_image_node = draw_frame_node.append_child("draw:image");
      // draw_image_node.append_attribute("xlink:href") = string("Pictures/" + src).c_str();
      draw_image_node.append_attribute("xlink:href") = string("../" + src).c_str();
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
    xml_node text_node = current_text_p_node.append_child(node_pcdata);
    text_node.set_value(caption.c_str());
  }

  // Save the picture into the Pictures output folder.
  // Later on this was not done,
  // because it would require an updated meta-inf/manifest.xml.
  // <manifest:file-entry manifest:full-path="Pictures/100002010000035C0000013E97D1D9088C414E87.png" manifest:media-type="image/png"/>
  // Another advantage of not including the pictures in the opendocument file is:
  // The OpenDocument file without pictures in them would be smaller as it contains only text.
  {
    //Database_BibleImages database_bibleimages;
    //string contents = database_bibleimages.get(src);
    //string path = filter_url_create_path(pictures_folder, src);
    //filter_url_file_put_contents(path, contents);
  }

  // Close the current paragraph.
  // Goal: Any text that will be added will be output into a new paragraph.
  current_text_p_node_opened = false;
  current_paragraph_style.clear ();
  current_paragraph_content.clear ();
}


// This function adds a tab to the current paragraph.
void Odf_Text::add_tab ()
{
  // Ensure a paragraph has started.
  if (!current_text_p_node_opened) new_paragraph ();
  
  // Write a text tab element.
  current_text_p_node.append_child ("text:tab");

  // Update public paragraph text.
  current_paragraph_content += "\t";
}
