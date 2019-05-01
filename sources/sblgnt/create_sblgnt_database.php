<?php
/*
Copyright (©) 2003-2019 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a wacopy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

echo "Copying Greek data for book ";

$output = __DIR__ . "/../databases/sblgnt.sqlite";
@unlink ($output);

$db = new PDO ("sqlite:$output");

$db->exec ("PRAGMA temp_store = MEMORY;");
$db->exec ("PRAGMA synchronous = OFF;");
$db->exec ("PRAGMA journal_mode = OFF;");

$sql = <<<'EOD'
CREATE TABLE sblgnt (
  book integer,
  chapter integer,
  verse integer,
  greek text
);
EOD;
$db->exec ($sql);

$input = __DIR__ . "/sblgnt.xml";

$book = 40 - 1;
$chapter = 0;
$verse = 0;
$word = false;
$greek = "";

$xml = new XMLReader();

$xml->open ($input);

while ($xml->read ()) {

  $nodeType = $xml->nodeType;

  $name = $xml->name;

  if ($nodeType == XMLReader::ELEMENT) {

    if ($name == "book") {
      $book++;
      $chapter = 0;
      $verse = 0;
      echo "$book ";
      $type = $xml->getAttribute ("type");
      if ($type == "book") {
      }
    }

    if ($name == "verse-number") {
      $id = $xml->getAttribute ("id");
      $id = explode (":", $id);
      $chapter = substr ($id[0], -2, 2);
      $chapter = intval ($chapter);
      $verse = intval ($id[1]);
    }

    if ($name == "w") {
      $word = true;
    }

  }

  if ($nodeType == XMLReader::TEXT) {
    if ($word) {
      $greek = $xml->value;
      $greek = filter_string_trim ($greek);
      $greek = filter_string_str_replace ("'", "''", $greek);
      $sql = "INSERT INTO sblgnt (book, chapter, verse, greek) VALUES (book, chapter, verse, '$greek');";
      $db->exec ($sql);
    }
  }

  if ($nodeType == XMLReader::END_ELEMENT) {
    if ($name == "w") {
      $word = false;
    }
  }

}

$xml->close ();

$output = realpath ($output);
echo "\n";
echo "SQLite database has been created at:\n";
echo "$output\n";

?>
