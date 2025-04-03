#!/bin/env python
# -*- coding: utf-8 -*-
#
# odf2txt - Convert ODF documents to text
#
# Copyright 2013 Matthe Woehlke <mw_triad@users.sourceforge.net>
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
#

import argparse
import re
import sys
import xml.sax
import zipfile

recognized_paragraph_properties = {
  'fo:background-color',
  'fo:border',
  'fo:border-bottom',
  'fo:border-left',
  'fo:border-right',
  'fo:border-top',
  'fo:break-before',
  'fo:hyphenation-ladder-count',
  'fo:keep-together',
  'fo:keep-with-next',
  'fo:margin',
  'fo:margin-bottom',
  'fo:margin-left',
  'fo:margin-right',
  'fo:margin-top',
  'fo:orphans',
  'fo:padding',
  'fo:padding-bottom',
  'fo:padding-left',
  'fo:padding-right',
  'fo:padding-top',
  'fo:text-align',
  'fo:text-indent',
  'fo:widows',
  'style:auto-text-indent',
  'style:contextual-spacing',
  'style:font-independent-line-spacing', # NOT DISPLAYED
  'style:justify-single-word',
  'style:line-break',
  'style:page-number',
  'style:punctuation-wrap',
  'style:shadow',
  'style:tab-stop-distance',
  'style:text-autospace', # NOT DISPLAYED
  'style:writing-mode', # NOT DISPLAYED
  'text:line-number',
  'text:number-lines',
}

recognized_text_properties = {
  'fo:background-color',
  'fo:color',
  'fo:country', # NOT DISPLAYED
  'fo:font-size',
  'fo:font-style',
  'fo:font-weight',
  'fo:hyphenate',
  'fo:hyphenation-push-char-count',
  'fo:hyphenation-remain-char-count',
  'fo:language', # NOT DISPLAYED
  'fo:text-shadow',
  'officeooo:paragraph-rsid', # NOT DISPLAYED
  'officeooo:rsid', # NOT DISPLAYED
  'style:font-name',
  'style:font-relief',
  'style:letter-kerning',
  'style:text-underline-color',
  'style:text-underline-style',
  'style:text-underline-width',
  'style:use-window-font-color',
}

recognized_table_column_properties = {
  'fo:break-after',
  'fo:break-before',
  'style:column-width',
  'style:rel-column-width',
  'style:use-optimal-column-width',
}

recognized_table_row_properties = {
  'fo:background-color',
  'fo:break-after',
  'fo:break-before',
  'fo:keep-together',
  'style:min-row-height',
  'style:row-height',
  'style:use-optimal-row-height',
}

recognized_table_cell_properties = {
  'fo:background-color',
  'fo:border',
  'fo:border-bottom',
  'fo:border-left',
  'fo:border-right',
  'fo:border-top',
  'fo:padding',
  'fo:padding-bottom',
  'fo:padding-left',
  'fo:padding-right',
  'fo:padding-top',
  'style:shadow',
}

recognized_content_tags = {
  'office:forms',
  'office:text',
  'table:table',
  'table:table-cell',
  'table:table-column',
  'table:table-row',
  'text:h',
  'text:line-break',
  'text:list',
  'text:list-item',
  'text:p',
  'text:s',
  'text:sequence-decl',
  'text:sequence-decls',
  'text:soft-page-break',
  'text:span',
  'text:tab',
}

break_at = re.compile('[.?!;]([)\"\'>»’”]|&gt;)|[?!] ')

no_break_after = [
  'e.g.',
  'i.e.'
  'mrs.',
  'mr.',
  'ms.',
  'dr.',
  'jr.',
  'sr.',
  'vs.',
]

defined_styles = {}
inline_styles = {}
inline_styles_parents = {}

args = None

#------------------------------------------------------------------------------
class context_parser(xml.sax.ContentHandler):
  def __init__(self, root_parser):
    xml.sax.ContentHandler.__init__(self)
    self.root_parser = root_parser

  def beginContext(self, name, attrs):
    pass

  def endContext(self):
    pass

#------------------------------------------------------------------------------
class parse_style_tab_stops(context_parser):
  #----------------------------------------------------------------------------
  def __init__(self, root_parser):
    context_parser.__init__(self, root_parser)
    self.stops = []

  #----------------------------------------------------------------------------
  def startElement(self, name, attrs):
    if name == 'style:tab-stop':
      self.stops.append(attrs)

  #----------------------------------------------------------------------------
  def endContext(self):
    if not len(self.stops):
      return

    defs = []
    for stop in self.stops:
      text = stop['style:position']
      if 'style:type' in stop:
        text += ' ' + stop['style:type']
      defs += [text]

    print ('  tab-stops: %s;' % ', '.join(defs))

#------------------------------------------------------------------------------
def extract_rule(attrs, key, display_name = None):
  if display_name is None:
    display_name = key[key.find(':') + 1:]

  if key in attrs:
    return ['%s: %s;' % (display_name, attrs[key])]
  else:
    return []

#------------------------------------------------------------------------------
def extract_lang_rules(attrs, key, display_name = None):
  rules = []

  if display_name is None:
    display_name = key[key.find(':') + 1:]

  if key in attrs:
    common_value = attrs[key]
    rules += ['%s: %s;' % (display_name, common_value)]

    asian_key = key + '-asian'
    if asian_key in attrs and attrs[asian_key] != common_value:
      rules += ['%s-asian: %s;' % (display_name, attrs[asian_key])]

    complex_key = key + '-complex'
    if complex_key in attrs and attrs[complex_key] != common_value:
      rules += ['%s-complex: %s;' % (display_name, attrs[complex_key])]

  return rules

#------------------------------------------------------------------------------
def extract_edge_rules(attrs, key, display_name = None):
  if display_name is None:
    display_name = key[key.find(':') + 1:]

  return extract_rule(attrs, key, display_name) + \
         extract_rule(attrs, key + '-top', display_name + '-top') + \
         extract_rule(attrs, key + '-left', display_name + '-left') + \
         extract_rule(attrs, key + '-right', display_name + '-right') + \
         extract_rule(attrs, key + '-bottom', display_name + '-bottom')

#------------------------------------------------------------------------------
def extract_style_rules(name, attrs, rules = None):
  if rules is None:
    rules = []

  if name == 'style:paragraph-properties':
    rules += extract_rule(attrs, 'fo:background-color')

    rules += extract_edge_rules(attrs, 'fo:border')
    rules += extract_edge_rules(attrs, 'fo:margin')
    rules += extract_edge_rules(attrs, 'fo:padding')
    rules += extract_rule(attrs, 'style:contextual-spacing')

    rules += extract_rule(attrs, 'fo:text-align')
    rules += extract_rule(attrs, 'style:justify-single-word')
    rules += extract_rule(attrs, 'fo:text-indent')
    rules += extract_rule(attrs, 'style:auto-text-indent')
    rules += extract_rule(attrs, 'style:punctuation-wrap')

    rules += extract_rule(attrs, 'fo:hyphenation-ladder-count')

    rules += extract_rule(attrs, 'style:line-break')
    rules += extract_rule(attrs, 'fo:break-before')
    rules += extract_rule(attrs, 'fo:keep-together')
    rules += extract_rule(attrs, 'fo:keep-with-next')
    rules += extract_rule(attrs, 'fo:orphans')
    rules += extract_rule(attrs, 'fo:widows')

    rules += extract_rule(attrs, 'style:tab-stop-distance')
    rules += extract_rule(attrs, 'style:shadow')

    rules += extract_rule(attrs, 'text:number-lines')
    rules += extract_rule(attrs, 'text:line-number')
    rules += extract_rule(attrs, 'style:page-number')

    for attr in attrs.keys():
      if not attr in recognized_paragraph_properties:
        rules += ['/* unhandled %s attribute \'%s\' */' % (name[6:], attr)]

  elif name == 'style:text-properties':
    rules += extract_rule(attrs, 'fo:background-color')
    rules += extract_rule(attrs, 'fo:color')
    if 'style:use-window-font-color' in attrs:
      print ('  font-color: window;')
    rules += extract_lang_rules(attrs, 'style:font-name')
    rules += extract_lang_rules(attrs, 'fo:font-size')
    rules += extract_lang_rules(attrs, 'fo:font-style')
    rules += extract_lang_rules(attrs, 'fo:font-weight')

    if 'style:text-underline-style' in attrs:
      underline_style = attrs['style:text-underline-style']
      if 'style:text-underline-width' in attrs:
        underline_style += ' ' + attrs['style:text-underline-width']
      if 'style:text-underline-color' in attrs:
        underline_style += ' ' + attrs['style:text-underline-color']
      rules += ['text-underline: %s;' % underline_style]

    rules += extract_rule(attrs, 'fo:text-shadow')
    rules += extract_rule(attrs, 'style:font-relief', 'text-relief')

    if 'fo:hyphenate' in attrs:
      text = attrs['fo:hyphenate']
      if 'fo:hyphenation-push-char-count' in attrs:
        text += ' ' + attrs['fo:hyphenation-push-char-count']
      elif 'fo:hyphenation-remain-char-count' in attrs:
        text += ' auto'
      if 'fo:hyphenation-remain-char-count' in attrs:
        text += ' ' + attrs['fo:hyphenation-remain-char-count']
      rules += ['hyphenate: %s;' % text]
    rules += extract_lang_rules(attrs, 'style:letter-kerning')

    for attr in attrs.keys():
      if attr.endswith('-asian') or attr.endswith('-complex'):
        continue;
      if not attr in recognized_text_properties:
        rules += ['/* unhandled %s attribute \'%s\' */' % (name[6:], attr)]

  elif name == 'style:table-cell-properties':
    rules += extract_rule(attrs, 'fo:background-color')
    rules += extract_rule(attrs, 'style:shadow')

    rules += extract_edge_rules(attrs, 'fo:border')
    rules += extract_edge_rules(attrs, 'fo:padding')

    for attr in attrs.keys():
      if not attr in recognized_table_cell_properties:
        rules += ['/* unhandled %s attribute \'%s\' */' % (name[6:], attr)]

  elif name == 'style:table-row-properties':
    rules += extract_rule(attrs, 'fo:background-color')

    rules += extract_rule(attrs, 'style:row-height', 'height')
    rules += extract_rule(attrs, 'style:min-row-height', 'min-height')
    rules += extract_rule(attrs, 'style:use-optimal-row-height',
                          'optimal-height')

    rules += extract_rule(attrs, 'fo:break-before')
    rules += extract_rule(attrs, 'fo:break-after')
    rules += extract_rule(attrs, 'fo:keep-together')

    for attr in attrs.keys():
      if not attr in recognized_table_row_properties:
        rules += ['/* unhandled %s attribute \'%s\' */' % (name[6:], attr)]

  elif name == 'style:table-column-properties':
    rules += extract_rule(attrs, 'style:column-width', 'width')
    rules += extract_rule(attrs, 'style:rel-column-width', 'rel-width')
    rules += extract_rule(attrs, 'style:use-optimal-column-width',
                          'optimal-width')

    rules += extract_rule(attrs, 'fo:break-before')
    rules += extract_rule(attrs, 'fo:break-after')

    for attr in attrs.keys():
      if not attr in recognized_table_column_properties:
        rules += ['/* unhandled %s attribute \'%s\' */' % (name[6:], attr)]

  elif name == 'style:tab-stops':
    rules += ['tab-stops: none;']

  elif name == 'style:tab-stop':
    text = attrs['style:position']
    if 'style:type' in attrs:
      text += ' ' + attrs['style:type']

    for i,rule in enumerate(rules):
      if rule.startswith('tab-stops:'):
        if rule == 'tab-stops: none;':
          rules[i] = rule.replace('none', text)
        else:
          rules[i] = '%s, %s;' % (rule[:-1], text)
        break

  else:
    return ['/* unhandled tag \'%s\' */' % name]

  return rules

#------------------------------------------------------------------------------
class parse_style_style(context_parser):
  #----------------------------------------------------------------------------
  def beginContext(self, name, attrs):
    # Only handle named non-default styles
    if name == 'style:style':
      style_name = attrs['style:name']

      # Skip non-user styles (do these come from imported Word documents?)
      if style_name.startswith('WW8Num'):
        self.root_parser.popContext()
        return

      # Get display name
      style_display_name = style_name
      if 'style:display-name' in attrs:
        style_display_name = attrs['style:display-name']

      # Print selector
      if 'style:class' in attrs:
        print ('%s:%s."%s" {' % (attrs['style:family'], attrs['style:class'], style_display_name))
      else:
        print ('%s."%s" {' % (attrs['style:family'], style_display_name))

      # Add to style names map
      defined_styles[style_name] = style_display_name
      self.in_style = True;

    # Handle default styles
    elif name == 'style:default-style':
      print ('%s {' % attrs['style:family'])
      self.in_style = True;

    # If an unnamed non-default style, cancel the context
    else:
      self.root_parser.popContext()

  #----------------------------------------------------------------------------
  def endContext(self):
    print ('}\n')

  #----------------------------------------------------------------------------
  def startElement(self, name, attrs):
    if name == 'style:tab-stops':
      self.root_parser.setContext(parse_style_tab_stops(self.root_parser))

    else:
      for rule in extract_style_rules(name, attrs):
        print ('  ' + rule)

#------------------------------------------------------------------------------
class style_parser(xml.sax.ContentHandler):
  #----------------------------------------------------------------------------
  def __init__(self):
    xml.sax.ContentHandler.__init__(self)

    self.context_parsers = []
    self.context_tags = []

    self.current_name = None
    self.current_attrs = {}

  #----------------------------------------------------------------------------
  def setContext(self, context):
    self.context_tags.append(self.current_name)
    self.context_parsers.append(context)
    context.beginContext(self.current_name, self.current_attrs)

  #----------------------------------------------------------------------------
  def popContext(self):
    self.context_parsers.pop()
    self.context_tags.pop()

  #----------------------------------------------------------------------------
  def startElement(self, name, attrs):
    self.current_name = name
    self.current_attrs = attrs

    if len(self.context_parsers):
      self.context_parsers[-1].startElement(name, attrs);

    if name == 'style:style' or name == 'style:default-style':
      self.setContext(parse_style_style(self))

  #----------------------------------------------------------------------------
  def endElement(self, name):
    if len(self.context_tags) and name == self.context_tags[-1]:
      self.context_parsers[-1].endContext()
      self.popContext()

    if len(self.context_parsers):
      self.context_parsers[-1].endElement(name);

#------------------------------------------------------------------------------
class simple_content_parser(xml.sax.ContentHandler):

  #----------------------------------------------------------------------------
  def __init__(self):
    xml.sax.ContentHandler.__init__(self)
    self.indent_level = 0

  #----------------------------------------------------------------------------
  def startElement(self, name, attrs):
    if name == 'text:list':
      self.indent_level += 2
    elif name == 'text:list-item':
      sys.stdout.write('\n%s* ' % (' ' * self.indent_level))

  #----------------------------------------------------------------------------
  def endElement(self, name):
    if name == 'text:p' or name == 'text:h':
      print ('\n')
    elif name == 'text:list':
      print ('\n')
      self.indent_level -= 2
    elif name == 'text:s':
      sys.stdout.write(' ')
    elif name == 'text:tab':
      sys.stdout.write('\t')
    elif name == 'text:line-break':
      sys.stdout.write('\n%s' % (' ' * self.indent_level))

  #----------------------------------------------------------------------------
  def characters(self, content):
    print(content, end='')

#------------------------------------------------------------------------------
def print_tag_start(name, attrs):
  text = '<!-- unhandled: <' + name
  for attr in attrs.keys():
    text += ' %s=\"%s\"' % (attr, attrs[attr])
  text += '> -->'
  print (text)

#------------------------------------------------------------------------------
def print_tag_end(name):
  print ('<!-- unhandled: </%s> -->' % name)

#------------------------------------------------------------------------------
def make_styled_tag(name, attrs, name_key = 'text:style-name'):
  if name_key in attrs:
    style_name = attrs[name_key]
    if style_name in defined_styles:
      return '<%s class=\"%s\">' % (name, defined_styles[style_name])
    elif style_name in inline_styles:
      rules = ' '.join(inline_styles[style_name])
      if style_name in inline_styles_parents:
        parent = inline_styles_parents[style_name]
        return '<%s class=\"%s\" style=\"%s\">' % (name, parent, rules)
      else:
        return '<%s style=\"%s\">' % (name, rules)

  return '<%s>' % name

#------------------------------------------------------------------------------
class styled_content_parser(simple_content_parser):
  #----------------------------------------------------------------------------
  def __init__(self):
    simple_content_parser.__init__(self)

    self.in_body = False
    self.in_style = None
    self.tags = []
    self.end_tags = []
    self.text = ''

  #----------------------------------------------------------------------------
  def startElement(self, name, attrs):
    self.tags.append(name)
    end_tag = None

    if name == 'office:body':
      self.in_body = True
      self.reinterpret_next = None

    elif self.in_style is not None:
      rules = extract_style_rules(name, attrs, inline_styles[self.in_style])
      inline_styles[self.in_style] = rules

    elif self.in_body:
      self.flushText()

      if self.reinterpret_next is not None:
        end_tag = self.reinterpret_next(name, attrs)
        self.reinterpret_next = None

      elif name == 'text:p' and 'text:style-name' in attrs:
        style_name = attrs['text:style-name']
        if style_name != 'Standard':
          sys.stdout.write(make_styled_tag('p', attrs))
          if self.tags[-2] == 'table:table-cell':
            end_tag = '</p>'
          else:
            end_tag = '</p>\n\n'

      elif name == 'text:h':
        tag = 'h%s' % attrs['text:outline-level']
        sys.stdout.write(make_styled_tag(tag, attrs))
        end_tag = '</%s>\n\n' % tag

      elif name == 'text:list':
        print ('\n%s%s' % ((' ' * self.indent_level), make_styled_tag('list', attrs)))
        self.indent_level += 2

      elif name == 'text:list-item':
        def reinterpret_list_item(name, attrs):
          sys.stdout.write(' ' * self.indent_level)
          sys.stdout.write(make_styled_tag('li', attrs))
          return '</li>\n'

        self.reinterpret_next = reinterpret_list_item

      elif name == 'text:span':
        sys.stdout.write(make_styled_tag('span', attrs))
        end_tag = '</span>'

      elif name == 'table:table':
        print (make_styled_tag('table', attrs))
        self.indent_level += 2

      elif name == 'table:table-column':
        sys.stdout.write(' ' * self.indent_level)
        sys.stdout.write(make_styled_tag('tc', attrs, 'table:style-name'))
        end_tag = '</tc>\n'

      elif name == 'table:table-row':
        sys.stdout.write(' ' * self.indent_level)
        sys.stdout.write(make_styled_tag('tr', attrs, 'table:style-name'))
        self.indent_level += 2

      elif name == 'table:table-cell':
        sys.stdout.write('\n%s' % (' ' * self.indent_level))
        sys.stdout.write(make_styled_tag('td', attrs, 'table:style-name'))
        end_tag = '</td>'

      elif name not in recognized_content_tags:
        print_tag_start(name, attrs)

    elif name == 'style:style':
      self.in_style = attrs['style:name']
      inline_styles[self.in_style] = []
      if 'style:parent-style-name' in attrs:
        parent = attrs['style:parent-style-name']
        if parent != 'Standard' and parent in defined_styles:
          inline_styles_parents[self.in_style] = defined_styles[parent]

    self.end_tags.append(end_tag)

  #----------------------------------------------------------------------------
  def endElement(self, name):
    self.tags.pop()

    if self.end_tags[-1] is not None:
      self.flushText(self.end_tags[-1])

    elif name == 'text:p':
      self.flushText()
      if self.tags[-1] != 'table:table-cell':
        print ('\n')

    elif name == 'text:s':
      self.flushText('&nbsp;')

    elif name == 'text:tab':
      self.flushText('\t')

    elif name == 'text:line-break':
      self.flushText('<br/>\n')
      self.text = ' ' * self.indent_level

    elif name == 'text:list':
      self.indent_level -= 2
      self.flushText('\n')
      self.text = '%s</list>' % (' ' * self.indent_level)

    elif name == 'table:table':
      self.indent_level -= 2
      self.flushText()
      print ('\n</table>')

    elif name == 'table:table-row':
      self.indent_level -= 2
      self.flushText()
      print ('\n%s</tr>' % (' ' * self.indent_level))

    elif name == 'table:table-cell':
      self.flushText()
      sys.stdout.write('</td>%s' % (' ' * self.indent_level))

    elif name == 'style:style':
      self.in_style = None

    if name == 'office:body':
      self.flushText()

    self.end_tags.pop()

  #----------------------------------------------------------------------------
  def characters(self, content):
    content = content.replace('&', '&amp;')
    content = content.replace('<', '&lt;')
    content = content.replace('>', '&gt;')
    self.text += content
    if content.find('\n') >= 0:
      self.flushText()

  #----------------------------------------------------------------------------
  def flushText(self, append = None):
    if append is not None:
      self.text += append

    if not args.break_sentences:
      sys.stdout.write(self.text.encode('utf-8'))
      self.text = ''
      return

    # Split at sentence breaks that are identifiably not abbreviations
    def breakLine(match):
      match_text = match.group(0)

      # Don't break before markup tags
      match_end = match.end(0) + 1
      if match_end < len(match.string) and match.string[match_end] != '<':
        return match_text

      if match_text[-1] == ' ':
        match_text = match_text[0:-1]

      return match_text + '\n'

    content = break_at.sub(breakLine, self.text)

    # Split at periods, except after non-breaking words
    n = 0
    while True:
      n = content.find('. ', n)
      if n < 0:
        break

      first_part = content[0:n + 1]
      first_part_lower = first_part.lower()

      # Check if period ends a non-breaking word
      for nb in no_break_after:
        if n > len(nb) and first_part_lower.endswith(nb):
          first_part = None
          break

      # Split content and print part before split
      if first_part is not None:
        print (first_part.encode('utf-8'))
        content = content[n + 2:]
        n = 0

      n += 2

    # Write remaining content
    sys.stdout.write(content.encode('utf-8'))
    self.text = ''

#------------------------------------------------------------------------------
def print_raw(archive, filename):
  data = archive.read(filename)
  print (data)

#------------------------------------------------------------------------------
def parse_xml(archive, filename, parser):
  data = archive.read(filename)
  xml.sax.parseString(data, parser)

#------------------------------------------------------------------------------
def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('document',
                      help='path to document to convert')
  parser.add_argument('--raw', dest='raw', action='store_const',
                      const=True, default=False,
                      help='print raw XML')
  parser.add_argument('--style', dest='style', action='store_const',
                      const=True, default=False,
                      help='include simplified style markup')
  parser.add_argument('--break-sentences', dest='break_sentences',
                      action='store_const', const=True, default=False,
                      help='add line break at the end of sentences')

  global args
  args = parser.parse_args()
  archive = zipfile.ZipFile(args.document)

  # Are we reading the raw XML?
  if args.raw:
    # Print style (if requested) and content, then exit
    args.style and print_raw(archive, 'styles.xml')
    print_raw(archive, 'content.xml')
    return

  if args.style:
    parse_xml(archive, 'styles.xml', style_parser())
    parse_xml(archive, 'content.xml', styled_content_parser())
  else:
    parse_xml(archive, 'content.xml', simple_content_parser())

#------------------------------------------------------------------------------
if __name__ == '__main__':
  main()
