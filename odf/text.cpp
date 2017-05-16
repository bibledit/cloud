/*
Copyright (©) 2003-2017 Teus Benschop.

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
#include <database/books.h>
#include <database/config/bible.h>
#include <styles/logic.h>


// Class for creating OpenDocument text documents.
// Initially the ODF Toolkit was used. But the Java code to generate this became too big for the compiler.
// The other thing is that Java is slow as compared to this method employed here.


Odf_Text::Odf_Text (string bible_in)
{
  bible = bible_in;
  current_text_p_opened = false;
  currentParagraphStyle.clear();
  currentParagraphContent.clear();
  currentTextStyle.clear();
  frameCount = 0;
  note_text_p_opened = false;
  noteCount = 0;
  currentNoteTextStyle.clear();

  // Unpack the .odt template.
  string templateOdf = filter_url_create_root_path ("odf", "template.odt");
  unpackedOdtFolder = filter_archive_unzip (templateOdf);
  filter_url_rmdir (filter_url_create_path (unpackedOdtFolder, "Configurations2"));
  
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
    childnode.append_attribute ("style:name") = "Lohit Hindi1";
    childnode.append_attribute ("svg:font-family") = "'Lohit Hindi'";
  
    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Times New Roman";
    childnode.append_attribute ("svg:font-family") = "'Times New Roman'";
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
  }

  xml_node office_body = rootnode.append_child ("office:body");
  {
    officeTextDomNode = office_body.append_child ("office:text");
    {
      xml_node text_sequence_decls = officeTextDomNode.append_child ("text:sequence-decls");
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
    childnode.append_attribute ("style:name") = "Lohit Hindi1";
    childnode.append_attribute ("svg:font-family") = "'Lohit Hindi'";
  
    childnode = office_font_face_decls.append_child ("style:font-face");
    childnode.append_attribute ("style:name") = "Times New Roman";
    childnode.append_attribute ("svg:font-family") = "'Times New Roman'";
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

  officeStylesDomNode = rootnode.append_child ("office:styles");
  {
    xml_node style_default_style = officeStylesDomNode.append_child ("style:default-style");
    style_default_style.append_attribute ("style:family") = "graphic";
    xml_node style_paragraph_properties = style_default_style.append_child ("style:paragraph-properties");
    if (style_paragraph_properties) {}
    xml_node style_text_properties = style_default_style.append_child ("style:text-properties");
    if (style_text_properties) {}
  }
  {
    xml_node style_default_style = officeStylesDomNode.append_child ("style:default-style");
    style_default_style.append_attribute ("style:family") = "paragraph";
    xml_node style_paragraph_properties = style_default_style.append_child ("style:paragraph-properties");
    if (style_paragraph_properties) {}
    xml_node style_text_properties = style_default_style.append_child ("style:text-properties");
    if (style_text_properties) {}
  }
  {
    xml_node style_default_style = officeStylesDomNode.append_child ("style:default-style");
    style_default_style.append_attribute ("style:family") = "table";
  }
  {
    xml_node style_style = officeStylesDomNode.append_child ("style:style");
    style_style.append_attribute ("style:name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:class") = "text";
  }
  {
    xml_node style_style = officeStylesDomNode.append_child ("style:style");
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
    xml_node style_style = officeStylesDomNode.append_child ("style:style");
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
    xml_node style_style = officeStylesDomNode.append_child ("style:style");
    style_style.append_attribute ("style:name") = "Header";
    style_style.append_attribute ("style:family") = "paragraph";
    style_style.append_attribute ("style:parent-style-name") = styles_logic_standard_sheet ().c_str();
    style_style.append_attribute ("style:class") = "extra";
  }
  {
    xml_node style_style = officeStylesDomNode.append_child ("style:style");
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


void Odf_Text::newParagraph (string style)
{
  currentTextPDomElement = officeTextDomNode.append_child ("text:p");
  currentTextPDomElementNameNode = currentTextPDomElement.append_attribute ("text:style-name") = style.c_str();
  current_text_p_opened = true;
  currentParagraphStyle = style;
  currentParagraphContent.clear();
}


// This function adds text to the current paragraph.
// $text: The text to add.
void Odf_Text::addText (string text)
{
  // Bail out if there's no text.
  if (text.empty()) return;

  // Ensure a paragraph has started.
  if (!current_text_p_opened) newParagraph ();
  
  // Temporal styles array should have at least one style for the code below to work.
  vector <string> styles (currentTextStyle.begin (), currentTextStyle.end ());
  if (styles.empty()) styles.push_back ("");
  
  // Write a text span element, nesting the second and later ones.
  xml_node domElement = currentTextPDomElement;
  for (string style : styles) {
    xml_node textSpanDomElement = domElement.append_child ("text:span");
    if (!style.empty ()) {
      textSpanDomElement.append_attribute ("text:style-name") = convertStyleName (style).c_str();
    }
    domElement = textSpanDomElement;
  }
  domElement.text ().set (filter_string_sanitize_html (text).c_str());
  
  // Update public paragraph text.
  currentParagraphContent += text;
}


// This creates a heading with contents styled "Heading 1".
// $text: Contents.
void Odf_Text::newHeading1 (string text, bool hide)
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
  xml_node styleDomElement = officeStylesDomNode.append_child ("style:style");
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
  styleTextPropertiesDomElement.append_attribute ("fo:font-size-asian") = "2pt";
  styleTextPropertiesDomElement.append_attribute ("fo:font-size-complex") = "2pt";
}


// This applies a page break.
void Odf_Text::newPageBreak ()
{
  newParagraph ("Page_20_Break");
  // Always clear the paragraph-opened-flag,
  // because we don't want subsequent text to be added to this page break,
  // since it would be nearly invisible, and thus text would mysteriously get lost.
  current_text_p_opened = false;
  currentParagraphStyle.clear ();
  currentParagraphContent.clear ();
}


// This creates a paragraph style.
// $name: the name of the style, e.g. 'p'.
// $dropcaps: If 0, there are no drop caps.
//            If greater than 0, it the number of characters in drop caps style.
void Odf_Text::createParagraphStyle (string name, float fontsize, int italic, int bold, int underline, int smallcaps, int alignment, float spacebefore, float spaceafter, float leftmargin, float rightmargin, float firstlineindent, bool keepWithNext, int dropcaps)
{
  // It looks like this in styles.xml:
  // <style:style style:display-name="p_c1" style:family="paragraph" style:name="p_c1">
  //   <style:paragraph-properties fo:margin-bottom="0mm" fo:margin-left="0mm" fo:margin-right="0mm" fo:margin-top="0mm" fo:text-align="justify" fo:text-indent="0mm"/>
  //     <style:drop-cap style:distance="0.15cm" style:length="1" style:lines="2"/>
  //   <style:paragraph-properties>
  //   <style:text-properties fo:font-size="12pt" style:font-size-asian="12pt" style:font-size-complex="12pt"/>
  // </style:style>
  xml_node styleDomElement = officeStylesDomNode.append_child ("style:style");
  styleDomElement.append_attribute ("style:name") = convertStyleName (name).c_str();
  styleDomElement.append_attribute ("style:display-name") = name.c_str();
  styleDomElement.append_attribute ("style:family") = "paragraph";

  xml_node styleParagraphPropertiesDomElement = styleDomElement.append_child ("style:paragraph-properties");

  xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");

  string sfontsize = convert_to_string (fontsize) + "pt";
  styleTextPropertiesDomElement.append_attribute ("fo:font-size") = sfontsize.c_str();
  styleTextPropertiesDomElement.append_attribute ("fo:font-size-asian") = sfontsize.c_str();
  styleTextPropertiesDomElement.append_attribute ("fo:font-size-complex") = sfontsize.c_str();

  // Italics, bold, underline, small caps can be either ooitOff or ooitOn for a paragraph.
  if (italic != ooitOff) {
    styleTextPropertiesDomElement.append_attribute ("fo:font-style") = "italic";
    styleTextPropertiesDomElement.append_attribute ("fo:font-style-asian") = "italic";
    styleTextPropertiesDomElement.append_attribute ("fo:font-style-complex") = "italic";
  }
  if (bold != ooitOff) {
    styleTextPropertiesDomElement.append_attribute ("fo:font-weight") = "bold";
    styleTextPropertiesDomElement.append_attribute ("fo:font-weight-asian") = "bold";
    styleTextPropertiesDomElement.append_attribute ("fo:font-weight-complex") = "bold";
  }
  if (underline != ooitOff) {
    styleTextPropertiesDomElement.append_attribute ("style:text-underline-style") = "solid";
    styleTextPropertiesDomElement.append_attribute ("style:text-underline-width") = "auto";
    styleTextPropertiesDomElement.append_attribute ("style:text-underline-color") = "font-color";
  }
  if (smallcaps != ooitOff) {
    styleTextPropertiesDomElement.append_attribute ("fo:font-variant") = "small-caps";
  }

  // Text alignment can be: AlignmentLeft, AlignmentCenter, AlignmentRight, AlignmentJustify.
  string alignmenttext = "";
  switch (alignment) {
    case AlignmentLeft:    alignmenttext = "start"; break;
    case AlignmentCenter:  alignmenttext = "center"; break;
    case AlignmentRight:   alignmenttext = "end"; break;
    case AlignmentJustify: alignmenttext = "justify"; break;
  }
  styleParagraphPropertiesDomElement.append_attribute ("fo:text-align") = alignmenttext.c_str();
  styleParagraphPropertiesDomElement.append_attribute ("style:justify-single-word") = "false";

  // Paragraph measurements; given in mm.
  string sspacebefore = convert_to_string (spacebefore) + "mm";
  styleParagraphPropertiesDomElement.append_attribute ("fo:margin-top") = sspacebefore.c_str();
  string sspaceafter = convert_to_string (spaceafter) + "mm";
  styleParagraphPropertiesDomElement.append_attribute ("fo:margin-bottom") = sspaceafter.c_str();
  string sleftmargin = convert_to_string (leftmargin) + "mm";
  styleParagraphPropertiesDomElement.append_attribute ("fo:margin-left") = sleftmargin.c_str();
  string srightmargin = convert_to_string (rightmargin) + "mm";
  styleParagraphPropertiesDomElement.append_attribute ("fo:margin-right") = srightmargin.c_str();
  string sfirstlineindent = convert_to_string (firstlineindent) + "mm";
  styleParagraphPropertiesDomElement.append_attribute ("fo:text-indent") = sfirstlineindent.c_str();

  if (keepWithNext) {
    styleParagraphPropertiesDomElement.append_attribute ("fo:keep-together") = "always";
    styleParagraphPropertiesDomElement.append_attribute ("fo:keep-with-next") = "always";
  }

  if (dropcaps > 0) {
    // E.g.: <style:drop-cap style:lines="2" style:length="2" style:distance="0.15cm"/>
    string length = convert_to_string (dropcaps);
    xml_node styleDropCapDomElement = styleParagraphPropertiesDomElement.append_child ("style:drop-cap");
    styleDropCapDomElement.append_attribute ("style:lines") = "2";
    styleDropCapDomElement.append_attribute ("style:length") = length.c_str();
    styleDropCapDomElement.append_attribute ("style:distance") = "0.15cm";
  }
}


// This updates the style name of the current paragraph.
// $name: the name of the style, e.g. 'p'.
void Odf_Text::updateCurrentParagraphStyle (string name)
{
  if (!current_text_p_opened) newParagraph ();
  currentTextPDomElement.remove_attribute (currentTextPDomElementNameNode);
  currentTextPDomElementNameNode = currentTextPDomElement.append_attribute ("text:style-name");
  currentTextPDomElementNameNode = convertStyleName (name).c_str();
}


// This opens a text style.
// $style: the object with the style variables.
// $note: Whether this refers to notes.
// $embed: boolean: Whether nest $style in an existing character style.
void Odf_Text::openTextStyle (Database_Styles_Item style, bool note, bool embed)
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
    xml_node styleDomElement = officeStylesDomNode.append_child ("style:style");
    styleDomElement.append_attribute ("style:name") = convertStyleName (marker).c_str();
    styleDomElement.append_attribute ("style:display-name") = marker.c_str();
    styleDomElement.append_attribute ("style:family") = "text";

    xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");

    // Italics, bold, underline, small caps can be ooitOff or ooitOn or ooitInherit or ooitToggle.
    // Not all features are implemented.
    if ((italic == ooitOn) || (italic == ooitToggle)) {
      styleTextPropertiesDomElement.append_attribute ("fo:font-style") = "italic";
      styleTextPropertiesDomElement.append_attribute ("fo:font-style-asian") = "italic";
      styleTextPropertiesDomElement.append_attribute ("fo:font-style-complex") = "italic";
    }
    if ((bold == ooitOn) || (bold == ooitToggle)) {
      styleTextPropertiesDomElement.append_attribute ("fo:font-weight") = "bold";
      styleTextPropertiesDomElement.append_attribute ("fo:font-weight-asian") = "bold";
      styleTextPropertiesDomElement.append_attribute ("fo:font-weight-complex") = "bold";
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
      //$styleTextPropertiesDomElement->setAttribute ("fo:font-size-asian", "87%";
      //$styleTextPropertiesDomElement->setAttribute ("fo:font-size-complex", "87%";
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
  if (!current_text_p_opened) newParagraph ();

  // The frame looks like this, in content.xml:
  // <draw:frame draw:style-name="fr1" draw:name="frame1" text:anchor-type="paragraph" svg:y="0cm" fo:min-width="0.34cm" draw:z-index="0">
  //   <draw:text-box fo:min-height="0.34cm">
  //     <text:p text:style-name="c">1</text:p>
  //   </draw:text-box>
  // </draw:frame>
  xml_node drawFrameDomElement = currentTextPDomElement.append_child ("draw:frame");
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
  textPDomElement.append_attribute ("text:style-name") = convertStyleName (style).c_str();
  textPDomElement.text().set( filter_string_sanitize_html (text).c_str());

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
      xml_node styleDomElement = officeStylesDomNode.append_child ("style:style");
      styleDomElement.append_attribute ("style:name") = convertStyleName (style).c_str();
      styleDomElement.append_attribute ("style:family") = "paragraph";
  
      xml_node styleParagraphPropertiesDomElement = styleDomElement.append_child ("style:paragraph-properties");
      styleParagraphPropertiesDomElement.append_attribute ("fo:text-align") = "justify";
      styleParagraphPropertiesDomElement.append_attribute ("style:justify-single-word") = "false";
  
      xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");
      string sfontsize = convert_to_string (fontsize) + "pt";
      styleTextPropertiesDomElement.append_attribute ("fo:font-size") = sfontsize.c_str();
      styleTextPropertiesDomElement.append_attribute ("fo:font-size-asian") = sfontsize.c_str();
      styleTextPropertiesDomElement.append_attribute ("fo:font-size-complex") = sfontsize.c_str();
      if (italic != ooitOff) {
        styleTextPropertiesDomElement.append_attribute ("fo:font-style") = "italic";
        styleTextPropertiesDomElement.append_attribute ("fo:font-style-asian") = "italic";
        styleTextPropertiesDomElement.append_attribute ("fo:font-style-complex") = "italic";
      }
      if (bold != ooitOff) {
        styleTextPropertiesDomElement.append_attribute ("fo:font-weight") = "bold";
        styleTextPropertiesDomElement.append_attribute ("fo:font-weight-asian") = "bold";
        styleTextPropertiesDomElement.append_attribute ("fo:font-weight-complex") = "bold";
      }
    }
    {
      // The style for the draw:frame element looks like this:
      // <style:style style:name="chapterframe" style:family="graphic" style:parent-style-name="ChapterFrameParent">
      //   <style:graphic-properties fo:margin-left="0cm" fo:margin-right="0.199cm" fo:margin-top="0cm" fo:margin-bottom="0cm" style:vertical-pos="from-top" style:vertical-rel="paragraph-content" style:horizontal-pos="left" style:horizontal-rel="paragraph" fo:background-color="transparent" style:background-transparency="100%" fo:padding="0cm" fo:border="none" style:shadow="none" style:flow-with-text="true">
      //   <style:background-image/>
      //   </style:graphic-properties>
      // </style:style>
      xml_node styleDomElement = officeStylesDomNode.append_child ("style:style");
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
  xml_node styleDomElement = officeStylesDomNode.append_child ("style:style");
  styleDomElement.append_attribute ("style:name") = "superscript";
  styleDomElement.append_attribute ("style:family") = "text";

  xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");
  //$styleTextPropertiesDomElement->setAttribute ("style:text-position", "super 58%");
  // If the percentage is not specified, an appropriate font height is used.
  styleTextPropertiesDomElement.append_attribute ("style:text-position") = "super";
  // Setting the superscript attribute automatically makes the font smaller. No need to set it manually.
  //$styleTextPropertiesDomElement->setAttribute ("fo:font-size", "87%";
  //$styleTextPropertiesDomElement->setAttribute ("fo:font-size-asian", "87%";
  //$styleTextPropertiesDomElement->setAttribute ("fo:font-size-complex", "87%";
}


// This function adds a note to the current paragraph.
// $caller: The text of the note caller, that is, the note citation.
// $style: Style name for the paragraph in the footnote body.
// $endnote: Whether this is a footnote and cross reference (false), or an endnote (true).
void Odf_Text::addNote (string caller, string style, bool endnote)
{
  // Ensure that a paragraph is open, so that the note can be added to it.
  if (!current_text_p_opened) newParagraph ();

  xml_node textNoteDomElement = currentTextPDomElement.append_child ("text:note");
  textNoteDomElement.append_attribute ("text:id") = convert_to_string ("ftn" + convert_to_string (noteCount)).c_str();
  noteCount++;
  note_text_p_opened = true;
  string noteclass;
  if (endnote) noteclass = "endnote";
  else noteclass = "footnote";
  textNoteDomElement.append_attribute ("text:note-class") = noteclass.c_str();

  // The note citation, the 'caller' is normally in superscript in the OpenDocument.
  // The default values of the application are used. The Bibledit stylesheet is not consulted.
  xml_node textNoteCitationDomElement = textNoteDomElement.append_child ("text:note-citation");
  textNoteCitationDomElement.append_attribute ("text:label") = filter_string_sanitize_html (caller).c_str();
  textNoteCitationDomElement.text().set( filter_string_sanitize_html (caller).c_str());

  xml_node textNoteBodyDomElement = textNoteDomElement.append_child ("text:note-body");

  noteTextPDomElement = textNoteBodyDomElement.append_child ("text:p");
  noteTextPDomElement.append_attribute ("text:style-name") = convertStyleName (style).c_str();

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
      textSpanDomElement.append_attribute ("text:style-name") = convertStyleName (style).c_str();
    }
    domElement = textSpanDomElement;
  }
  domElement.text().set( filter_string_sanitize_html (text).c_str());
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
  xml_node textHDomElement = officeTextDomNode.append_child ("text:h");
  textHDomElement.append_attribute ("text:style-name") = convertStyleName (style).c_str();
  textHDomElement.append_attribute ("text:outline-level") = "1";
  textHDomElement.text().set(filter_string_sanitize_html (text).c_str());

  // Heading style looks like this in styles.xml:
  // <style:style style:name="Heading_20_1" style:display-name="Heading 1" style:family="paragraph" style:parent-style-name="Heading" style:next-style-name="Text_20_body" style:default-outline-level="1" style:class="text">
  // <style:text-properties fo:font-size="115%" fo:font-weight="bold" style:font-size-asian="115%" style:font-weight-asian="bold" style:font-size-complex="115%" style:font-weight-complex="bold"/>
  // </style:style>
  // Create the style if it does not yet exist.
  if (find (createdStyles.begin(), createdStyles.end (), style) == createdStyles.end()) {
    xml_node styleDomElement = officeStylesDomNode.append_child ("style:style");
    styleDomElement.append_attribute ("style:name") = convertStyleName (style).c_str();
    styleDomElement.append_attribute ("style:display-name"), style.c_str();
    styleDomElement.append_attribute ("style:family") = "paragraph";
    styleDomElement.append_attribute ("style:parent-style-name") = "Heading";
    styleDomElement.append_attribute ("style:next-style-name") = "Text_20_body";
    styleDomElement.append_attribute ("style:default-outline-level") = "1";
    styleDomElement.append_attribute ("style:class") = "text";
    {
      xml_node styleTextPropertiesDomElement = styleDomElement.append_child ("style:text-properties");
      styleTextPropertiesDomElement.append_attribute ("fo:font-size") = "115%";
      styleTextPropertiesDomElement.append_attribute ("fo:font-weight") = "bold";
      styleTextPropertiesDomElement.append_attribute ("fo:font-size-asian") = "115%";
      styleTextPropertiesDomElement.append_attribute ("fo:font-weight-asian") = "bold";
      styleTextPropertiesDomElement.append_attribute ("fo:font-size-complex") = "115%";
      styleTextPropertiesDomElement.append_attribute ("fo:font-weight-complex") = "bold";
      if (hide) {
        styleTextPropertiesDomElement.append_attribute ("text:display") = "none";
      }
    }
    createdStyles.push_back (style);
  }

  // Make paragraph null, so that adding subsequent text creates a new paragraph.
  current_text_p_opened = false;
  currentParagraphStyle.clear ();
  currentParagraphContent.clear ();
}


// This converts the name of a style so that it is fit for use in OpenDocument files.
// E.g. 'Heading 1' becomes 'Heading_20_1'
// $style: Input
// It returns the converted style name.
string Odf_Text::convertStyleName (string style)
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
  filter_url_unlink (zippedfile);
}


