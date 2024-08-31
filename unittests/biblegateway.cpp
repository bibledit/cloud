/*
Copyright (©) 2003-2024 Teus Benschop.

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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <resource/logic.h>


TEST (biblegateway, basic)
{
  std::string resource;
  int book;
  std::string text;
  
  resource = "Библия, ревизирано издание (BPB)";
  book = 1; // Genesis.
  text = resource_logic_bible_gateway_get (resource, book, 1, 1);
  EXPECT_EQ ("НАЧАЛО НА СВЕТА И ЧОВЕЧЕСТВОТОСътворението 1 В началото Бог сътвори небето и земята.", text);

  resource = "21st Century King James Version (KJ21)";
  book = 34; // Nahum.
  text = resource_logic_bible_gateway_get (resource, book, 1, 0);
  EXPECT_EQ ("", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 1);
  EXPECT_EQ ("1 The burden of Nineveh. The book of the vision of Nahum the Elkoshite.", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 15);
  EXPECT_EQ ("Behold upon the mountains the feet of Him that bringeth good tidings, that publisheth peace! O Judah, keep thy solemn feasts, perform thy vows; for the wicked shall no more pass through thee; he is utterly cut off.", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 16);
  EXPECT_EQ ("", text);

  book = 64; // 3 John.
  text = resource_logic_bible_gateway_get (resource, book, 1, 0);
  EXPECT_EQ ("", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 1);
  EXPECT_EQ ("The Elder, Unto the well-beloved Gaius, whom I love in the truth:", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 14);
  EXPECT_EQ ("but I trust I shall shortly see thee, and we shall speak face to face. Peace be to thee. Our friends salute thee. Greet the friends by name.", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 15);
  EXPECT_EQ ("", text);
  
  resource = "Expanded Bible (EXB)";
  book = 34; // Nahum.
  text = resource_logic_bible_gateway_get (resource, book, 1, 0);
  EXPECT_EQ ("", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 1);
  EXPECT_EQ ("1 This is the ·message [oracle; burden] for the city of Nineveh [C the capital of the Assyrian empire]. This is the book of the vision of Nahum, ·who was from the town of Elkosh [L the Elkoshite; C of uncertain location].", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 2);
  EXPECT_EQ ("The Lord Is Angry with Nineveh The Lord is a ·jealous [zealous] God [Ex. 20:5; 34:14; Deut. 4:24; 5:9; Josh. 24:19] who ·punishes [avenges]; the Lord ·punishes [avenges] and is filled with ·anger [wrath]. The Lord ·punishes [takes vengeance on] ·those who are against him [his adversaries/enemies], and he ·stays angry with [or vents his wrath against] his enemies.", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 11);
  EXPECT_EQ ("Someone has come from ·Nineveh [L you] who ·makes evil plans [plots evil] against the Lord and gives wicked ·advice [counsel; strategy].", text);
  text = resource_logic_bible_gateway_get (resource, book, 1, 12);
  EXPECT_EQ ("This is what the Lord says: “Although Assyria ·is strong [or has allies] and has many people, it will be ·defeated [destroyed] and ·brought to an end [pass away]. Although I have ·made you suffer, Judah [L afflicted you], I will ·make you suffer [afflict you] no more.", text);
  
  resource = "J.B. Phillips New Testament (PHILLIPS)";
  book = 63; // 2 John.
  text = resource_logic_bible_gateway_get (resource, book, 1, 3);
  EXPECT_EQ ("This letter comes from the Elder to a certain Christian lady and her children, held in the highest affection not only by me but by all who know the truth. For the truth’s sake (which even now we know and which will be our companion for ever) I wish you, in all love and sincerity, grace, mercy and peace from God the Father and the Lord Jesus Christ, the Father’s Son.", text);
  
  resource = "Living Bible (TLB)";
  book = 40; // Matthew.
  text = resource_logic_bible_gateway_get (resource, book, 1, 17);
  EXPECT_EQ ("These are fourteen[a] of the generations from Abraham to King David; and fourteen from King David’s time to the exile; and fourteen from the exile to Christ.<br>Note: These are fourteen, literally, “So all the generations from Abraham unto David are fourteen.”", text);

  resource = "New American Standard Bible (NASB)";
  book = 60; // 1 Peter.
  text = resource_logic_bible_gateway_get (resource, book, 2, 5);
  EXPECT_EQ ("you also, as living stones, [a]are being built up as a spiritual house for a holy priesthood, to offer spiritual sacrifices that are acceptable to God through Jesus Christ.<br>Note: Or allow yourselves to be built up; or build yourselves up", text);

  resource = "English Standard Version (ESV)";
  book = 60; // 1 Peter.
  text = resource_logic_bible_gateway_get (resource, book, 3, 7);
  EXPECT_EQ ("Likewise, husbands, live with your wives in an understanding way, showing honor to the woman as the weaker vessel, since they are heirs with you[a] of the grace of life, so that your prayers may not be hindered.<br>Note: Some manuscripts since you are joint heirs", text);

  // Test poetry in verses.
  resource = "New American Standard Bible (NASB)";
  book = 54; // 1 Timothy.
  text = resource_logic_bible_gateway_get (resource, book, 3, 16);
  EXPECT_EQ ("Beyond question, great is the mystery of godliness: He who was revealed in the flesh, Was [a]vindicated [b]in the Spirit, Seen by angels, Proclaimed among the nations, Believed on in the world, Taken up in glory.<br>Note: Or justified<br>Note: Or by", text);

}

#endif

