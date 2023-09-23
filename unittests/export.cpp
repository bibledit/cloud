/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <text/text.h>
#include <esword/text.h>
#include <olb/text.h>
#include <html/text.h>
#include <odf/text.h>
#include <tbsx/text.h>
#include <filter/url.h>
using namespace std;


TEST (filter, export) 
{
  refresh_sandbox (false);

  // Test the plain text export tool.
  {
    Text_Text text_text;
    text_text.addtext ("text one");
    text_text.addnotetext ("note one");
    EXPECT_EQ ("text one", text_text.get ());
    EXPECT_EQ ("note one", text_text.getnote ());
  }

  {
    Text_Text text_text;
    text_text.paragraph ("paragraph1");
    text_text.note ("note1");
    text_text.paragraph ("paragraph2");
    text_text.note ("note");
    text_text.addnotetext ("2");
    EXPECT_EQ ("paragraph1\nparagraph2", text_text.get ());
    EXPECT_EQ ("note1\nnote2", text_text.getnote ());
  }
  
  {
    Text_Text text_text;
    text_text.paragraph ("paragraph");
    EXPECT_EQ ("paragraph", text_text.line ());
  }

  // Test export title via the e-Sword tool.
  {
    Esword_Text esword_text = Esword_Text ("The Word of the Lord Jesus Christ");
    vector <string> sql = {
      {"PRAGMA foreign_keys=OFF;"},
      {"PRAGMA synchronous=OFF;"},
      {"CREATE TABLE Details (Description NVARCHAR(255), Abbreviation NVARCHAR(50), Comments TEXT, Version INT, Font NVARCHAR(50), Unicode BOOL, RightToLeft BOOL, OT BOOL, NT BOOL, Apocrypha BOOL, Strong BOOL);"},
      {"INSERT INTO Details VALUES ('The Word of the Lord Jesus Christ', 'The Word of the Lord Jesus Christ', 'The Word of the Lord Jesus Christ', 1, 'UNICODE', 1, 0, 1, 1, 0, 0);"},
      {"CREATE TABLE Bible (Book INT, Chapter INT, Verse INT, Scripture TEXT);"}
    };
    EXPECT_EQ (sql, esword_text.get_sql ());
  }
  
  // Test e-Sword text zero reference.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.add_text ("The Word of God");
    esword_text.finalize ();
    vector <string> sql = {
      {"PRAGMA foreign_keys=OFF;"},
      {"PRAGMA synchronous=OFF;"},
      {"CREATE TABLE Details (Description NVARCHAR(255), Abbreviation NVARCHAR(50), Comments TEXT, Version INT, Font NVARCHAR(50), Unicode BOOL, RightToLeft BOOL, OT BOOL, NT BOOL, Apocrypha BOOL, Strong BOOL);"},
      {"INSERT INTO Details VALUES ('', '', '', 1, 'UNICODE', 1, 0, 1, 1, 0, 0);"},
      {"CREATE TABLE Bible (Book INT, Chapter INT, Verse INT, Scripture TEXT);"},
      {"INSERT INTO Bible VALUES (0, 0, 0, '\\u84?\\u104?\\u101?\\u32?\\u87?\\u111?\\u114?\\u100?\\u32?\\u111?\\u102?\\u32?\\u71?\\u111?\\u100?');"},
      {"CREATE INDEX BookChapterVerseIndex ON Bible (Book, Chapter, Verse);"}
    };
    EXPECT_EQ (sql, esword_text.get_sql ());
  }
  
  // Test e-Sword converter John 2:3.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.newBook (43);
    esword_text.newChapter (2);
    esword_text.newVerse (3);
    esword_text.add_text ("In the beginning was the Word, and the Word was with God, and the Word was God.");
    esword_text.finalize ();
    vector <string> sql = {
      {"PRAGMA foreign_keys=OFF;"},
      {"PRAGMA synchronous=OFF;"},
      {"CREATE TABLE Details (Description NVARCHAR(255), Abbreviation NVARCHAR(50), Comments TEXT, Version INT, Font NVARCHAR(50), Unicode BOOL, RightToLeft BOOL, OT BOOL, NT BOOL, Apocrypha BOOL, Strong BOOL);"},
      {"INSERT INTO Details VALUES ('', '', '', 1, 'UNICODE', 1, 0, 1, 1, 0, 0);"},
      {"CREATE TABLE Bible (Book INT, Chapter INT, Verse INT, Scripture TEXT);"},
      {"INSERT INTO Bible VALUES (43, 2, 3, '\\u73?\\u110?\\u32?\\u116?\\u104?\\u101?\\u32?\\u98?\\u101?\\u103?\\u105?\\u110?\\u110?\\u105?\\u110?\\u103?\\u32?\\u119?\\u97?\\u115?\\u32?\\u116?\\u104?\\u101?\\u32?\\u87?\\u111?\\u114?\\u100?\\u44?\\u32?\\u97?\\u110?\\u100?\\u32?\\u116?\\u104?\\u101?\\u32?\\u87?\\u111?\\u114?\\u100?\\u32?\\u119?\\u97?\\u115?\\u32?\\u119?\\u105?\\u116?\\u104?\\u32?\\u71?\\u111?\\u100?\\u44?\\u32?\\u97?\\u110?\\u100?\\u32?\\u116?\\u104?\\u101?\\u32?\\u87?\\u111?\\u114?\\u100?\\u32?\\u119?\\u97?\\u115?\\u32?\\u71?\\u111?\\u100?\\u46?');"},
      {"CREATE INDEX BookChapterVerseIndex ON Bible (Book, Chapter, Verse);"}
    };
    EXPECT_EQ (sql, esword_text.get_sql ());
  }
  
  // Test e-Sword converter fragmented text.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.newBook (43);
    esword_text.newChapter (1);
    esword_text.newVerse (1);
    esword_text.add_text ("In the beginning was the Word");
    esword_text.add_text (", and the Word was with God");
    esword_text.add_text (", and the Word was God.");
    esword_text.finalize ();
    vector <string> sql = {
      {"PRAGMA foreign_keys=OFF;"},
      {"PRAGMA synchronous=OFF;"},
      {"CREATE TABLE Details (Description NVARCHAR(255), Abbreviation NVARCHAR(50), Comments TEXT, Version INT, Font NVARCHAR(50), Unicode BOOL, RightToLeft BOOL, OT BOOL, NT BOOL, Apocrypha BOOL, Strong BOOL);"},
      {"INSERT INTO Details VALUES ('', '', '', 1, 'UNICODE', 1, 0, 1, 1, 0, 0);"},
      {"CREATE TABLE Bible (Book INT, Chapter INT, Verse INT, Scripture TEXT);"},
      {"INSERT INTO Bible VALUES (43, 1, 1, '\\u73?\\u110?\\u32?\\u116?\\u104?\\u101?\\u32?\\u98?\\u101?\\u103?\\u105?\\u110?\\u110?\\u105?\\u110?\\u103?\\u32?\\u119?\\u97?\\u115?\\u32?\\u116?\\u104?\\u101?\\u32?\\u87?\\u111?\\u114?\\u100?\\u44?\\u32?\\u97?\\u110?\\u100?\\u32?\\u116?\\u104?\\u101?\\u32?\\u87?\\u111?\\u114?\\u100?\\u32?\\u119?\\u97?\\u115?\\u32?\\u119?\\u105?\\u116?\\u104?\\u32?\\u71?\\u111?\\u100?\\u44?\\u32?\\u97?\\u110?\\u100?\\u32?\\u116?\\u104?\\u101?\\u32?\\u87?\\u111?\\u114?\\u100?\\u32?\\u119?\\u97?\\u115?\\u32?\\u71?\\u111?\\u100?\\u46?');"},
      {"CREATE INDEX BookChapterVerseIndex ON Bible (Book, Chapter, Verse);"}
    };
    EXPECT_EQ (sql, esword_text.get_sql ());
  }

  // Test e-Sword converter switch reference.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.newBook (1);
    esword_text.newChapter (2);
    esword_text.newVerse (3);
    esword_text.add_text ("But as many as received him, to them gave he power to become the sons of God, even to them that believe on his name.");
    esword_text.newBook (4);
    esword_text.newChapter (5);
    esword_text.newVerse (6);
    esword_text.add_text ("Which were born, not of blood, nor of the will of the flesh, nor of the will of man, but of God.");
    esword_text.finalize ();
    vector <string> sql = {
      {"PRAGMA foreign_keys=OFF;"},
      {"PRAGMA synchronous=OFF;"},
      {"CREATE TABLE Details (Description NVARCHAR(255), Abbreviation NVARCHAR(50), Comments TEXT, Version INT, Font NVARCHAR(50), Unicode BOOL, RightToLeft BOOL, OT BOOL, NT BOOL, Apocrypha BOOL, Strong BOOL);"},
      {"INSERT INTO Details VALUES ('', '', '', 1, 'UNICODE', 1, 0, 1, 1, 0, 0);"},
      {"CREATE TABLE Bible (Book INT, Chapter INT, Verse INT, Scripture TEXT);"},
      {"INSERT INTO Bible VALUES (1, 2, 3, '\\u66?\\u117?\\u116?\\u32?\\u97?\\u115?\\u32?\\u109?\\u97?\\u110?\\u121?\\u32?\\u97?\\u115?\\u32?\\u114?\\u101?\\u99?\\u101?\\u105?\\u118?\\u101?\\u100?\\u32?\\u104?\\u105?\\u109?\\u44?\\u32?\\u116?\\u111?\\u32?\\u116?\\u104?\\u101?\\u109?\\u32?\\u103?\\u97?\\u118?\\u101?\\u32?\\u104?\\u101?\\u32?\\u112?\\u111?\\u119?\\u101?\\u114?\\u32?\\u116?\\u111?\\u32?\\u98?\\u101?\\u99?\\u111?\\u109?\\u101?\\u32?\\u116?\\u104?\\u101?\\u32?\\u115?\\u111?\\u110?\\u115?\\u32?\\u111?\\u102?\\u32?\\u71?\\u111?\\u100?\\u44?\\u32?\\u101?\\u118?\\u101?\\u110?\\u32?\\u116?\\u111?\\u32?\\u116?\\u104?\\u101?\\u109?\\u32?\\u116?\\u104?\\u97?\\u116?\\u32?\\u98?\\u101?\\u108?\\u105?\\u101?\\u118?\\u101?\\u32?\\u111?\\u110?\\u32?\\u104?\\u105?\\u115?\\u32?\\u110?\\u97?\\u109?\\u101?\\u46?');"},
      {"INSERT INTO Bible VALUES (4, 5, 6, '\\u87?\\u104?\\u105?\\u99?\\u104?\\u32?\\u119?\\u101?\\u114?\\u101?\\u32?\\u98?\\u111?\\u114?\\u110?\\u44?\\u32?\\u110?\\u111?\\u116?\\u32?\\u111?\\u102?\\u32?\\u98?\\u108?\\u111?\\u111?\\u100?\\u44?\\u32?\\u110?\\u111?\\u114?\\u32?\\u111?\\u102?\\u32?\\u116?\\u104?\\u101?\\u32?\\u119?\\u105?\\u108?\\u108?\\u32?\\u111?\\u102?\\u32?\\u116?\\u104?\\u101?\\u32?\\u102?\\u108?\\u101?\\u115?\\u104?\\u44?\\u32?\\u110?\\u111?\\u114?\\u32?\\u111?\\u102?\\u32?\\u116?\\u104?\\u101?\\u32?\\u119?\\u105?\\u108?\\u108?\\u32?\\u111?\\u102?\\u32?\\u109?\\u97?\\u110?\\u44?\\u32?\\u98?\\u117?\\u116?\\u32?\\u111?\\u102?\\u32?\\u71?\\u111?\\u100?\\u46?');"},
      {"CREATE INDEX BookChapterVerseIndex ON Bible (Book, Chapter, Verse);"}
    };
    EXPECT_EQ (sql, esword_text.get_sql ());
  }

  // Test e-Sword converter create module.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.add_text ("In the beginning was the Word, and the Word was with God, and the Word was God.");
    esword_text.finalize ();
    string filename = "/tmp/module.bblx";
    esword_text.createModule (filename);
    int filesize = filter_url_filesize (filename);
    EXPECT_EQ (16384, filesize);
    filter_url_unlink (filename);
  }
  
  // Test tool to export Online Bible.
  {
    OnlineBible_Text onlinebible_text = OnlineBible_Text ();
    onlinebible_text.add_text ("No verse given, so discard this.");
    onlinebible_text.newVerse (2, 2, 2);
    onlinebible_text.add_text ("Text for Exodus 2:2, not verse 2-6a.");
    onlinebible_text.storeData ();
    onlinebible_text.add_text ("Verse was stored, no new verse given, so discard this.");
    string filename = "/tmp/OLBTextTest1.exp";
    onlinebible_text.save (filename);
    string standard = filter_url_file_get_contents (filter_url_create_root_path ({"unittests", "tests", "onlinebible1.exp"}));
    string result = filter_url_file_get_contents (filename);
    EXPECT_EQ (standard, result);
    filter_url_unlink (filename);
  }
  
  {
    OnlineBible_Text onlinebible_text = OnlineBible_Text ();
    onlinebible_text.addNote ();
    onlinebible_text.add_text ("Discard this note text because no verse has been given yet.");
    onlinebible_text.close_current_note ();
    onlinebible_text.newVerse (1, 2, 3);
    onlinebible_text.addNote ();
    onlinebible_text.add_text ("Output this note text.");
    onlinebible_text.close_current_note ();
    string filename = "/tmp/OLBTextTest2.exp";
    onlinebible_text.save (filename);
    string standard = filter_url_file_get_contents (filter_url_create_root_path ({"unittests", "tests", "onlinebible2.exp"}));
    string result = filter_url_file_get_contents (filename);
    EXPECT_EQ (standard, result);
    filter_url_unlink (filename);
  }

  // TBS export book ID and book name.
  {
    Tbsx_Text tbsx;
    tbsx.set_book_id("MAT");
    tbsx.set_book_name("Matthew");
    string standard =
R"(###MAT
###! Matthew)";
    EXPECT_EQ (standard, tbsx.get_document ());
  }
  
  // Test TBS exporter chapter handling.
  {
    Tbsx_Text tbsx;
    tbsx.set_chapter(2);
    tbsx.set_header("Header");
    string standard =
R"(##2
##! Header)";
    EXPECT_EQ (standard, tbsx.get_document ());
  }
  
  // Text the TBS exporter tool paragraph handling.
  {
    Tbsx_Text tbsx;
    tbsx.open_paragraph();
    tbsx.add_text("1 Text contents");
    string standard =
R"(#%
#1 Text contents)";
    EXPECT_EQ (standard, tbsx.get_document ());
  }
  
  // Text the TBS exporter tool supplied text handling.
  {
    Tbsx_Text tbsx;
    tbsx.open_paragraph();
    tbsx.add_text("1 Text ");
    tbsx.add_text("added", true);
    tbsx.add_text(" content");
    string standard =
R"(#%
#1 Text *added* content)";
    EXPECT_EQ (standard, tbsx.get_document ());
  }

  // Test the TBS text export tool.
  {
    Tbsx_Text tbsx;
    tbsx.add_text ("Text");
    tbsx.open_note();
    tbsx.add_text("note");
    tbsx.close_note();
    tbsx.add_text (" of verse.");
    string standard = R"(#Text[note] of verse.)";
    EXPECT_EQ (standard, tbsx.get_document ());
  }
}


#endif

