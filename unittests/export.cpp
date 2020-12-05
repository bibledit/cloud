/*
Copyright (©) 2003-2020 Teus Benschop.

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


#include <unittests/export.h>
#include <unittests/utilities.h>
#include <text/text.h>
#include <esword/text.h>
#include <olb/text.h>
#include <html/text.h>
#include <odf/text.h>
#include <filter/url.h>


void test_export () // Todo
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);

  // Test the plain text export tool.
  {
    Text_Text text_text;
    text_text.addtext ("text one");
    text_text.addnotetext ("note one");
    evaluate (__LINE__, __func__, "text one", text_text.get ());
    evaluate (__LINE__, __func__, "note one", text_text.getnote ());
  }

  {
    Text_Text text_text;
    text_text.paragraph ("paragraph1");
    text_text.note ("note1");
    text_text.paragraph ("paragraph2");
    text_text.note ("note");
    text_text.addnotetext ("2");
    evaluate (__LINE__, __func__, "paragraph1\nparagraph2", text_text.get ());
    evaluate (__LINE__, __func__, "note1\nnote2", text_text.getnote ());
  }
  
  {
    Text_Text text_text;
    text_text.paragraph ("paragraph");
    evaluate (__LINE__, __func__, "paragraph", text_text.line ());
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
    evaluate (__LINE__, __func__, sql, esword_text.get_sql ());
  }
  
  // Test e-Sword text zero reference.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.addText ("The Word of God");
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
    evaluate (__LINE__, __func__, sql, esword_text.get_sql ());
  }
  
  // Test e-Sword converter John 2:3.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.newBook (43);
    esword_text.newChapter (2);
    esword_text.newVerse (3);
    esword_text.addText ("In the beginning was the Word, and the Word was with God, and the Word was God.");
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
    evaluate (__LINE__, __func__, sql, esword_text.get_sql ());
  }
  
  // Test e-Sword converter fragmented text.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.newBook (43);
    esword_text.newChapter (1);
    esword_text.newVerse (1);
    esword_text.addText ("In the beginning was the Word");
    esword_text.addText (", and the Word was with God");
    esword_text.addText (", and the Word was God.");
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
    evaluate (__LINE__, __func__, sql, esword_text.get_sql ());
  }

  // Test e-Sword converter switch reference.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.newBook (1);
    esword_text.newChapter (2);
    esword_text.newVerse (3);
    esword_text.addText ("But as many as received him, to them gave he power to become the sons of God, even to them that believe on his name.");
    esword_text.newBook (4);
    esword_text.newChapter (5);
    esword_text.newVerse (6);
    esword_text.addText ("Which were born, not of blood, nor of the will of the flesh, nor of the will of man, but of God.");
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
    evaluate (__LINE__, __func__, sql, esword_text.get_sql ());
  }

  // Test e-Sword converter create module.
  {
    Esword_Text esword_text = Esword_Text ("");
    esword_text.addText ("In the beginning was the Word, and the Word was with God, and the Word was God.");
    esword_text.finalize ();
    string filename = "/tmp/module.bblx";
    esword_text.createModule (filename);
    int filesize = filter_url_filesize (filename);
    evaluate (__LINE__, __func__, 16384, filesize);
    filter_url_unlink (filename);
  }
  
  // Test tool to export Online Bible.
  {
    OnlineBible_Text onlinebible_text = OnlineBible_Text ();
    onlinebible_text.addText ("No verse given, so discard this.");
    onlinebible_text.newVerse (2, 2, 2);
    onlinebible_text.addText ("Text for Exodus 2:2, not verse 2-6a.");
    onlinebible_text.storeData ();
    onlinebible_text.addText ("Verse was stored, no new verse given, so discard this.");
    string filename = "/tmp/OLBTextTest1.exp";
    onlinebible_text.save (filename);
    string standard = filter_url_file_get_contents (filter_url_create_root_path ("unittests", "tests", "onlinebible1.exp"));
    string result = filter_url_file_get_contents (filename);
    evaluate (__LINE__, __func__, standard, result);
    filter_url_unlink (filename);
  }
  
  {
    OnlineBible_Text onlinebible_text = OnlineBible_Text ();
    onlinebible_text.addNote ();
    onlinebible_text.addText ("Discard this note text because no verse has been given yet.");
    onlinebible_text.closeCurrentNote ();
    onlinebible_text.newVerse (1, 2, 3);
    onlinebible_text.addNote ();
    onlinebible_text.addText ("Output this note text.");
    onlinebible_text.closeCurrentNote ();
    string filename = "/tmp/OLBTextTest2.exp";
    onlinebible_text.save (filename);
    string standard = filter_url_file_get_contents (filter_url_create_root_path ("unittests", "tests", "onlinebible2.exp"));
    string result = filter_url_file_get_contents (filename);
    evaluate (__LINE__, __func__, standard, result);
    filter_url_unlink (filename);
  }

}
