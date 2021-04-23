/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <unittests/studylight.h>
#include <unittests/utilities.h>
#include <resource/logic.h>
#include <filter/string.h>


void test_studylight () // Todo
{
  trace_unit_tests (__func__);
  
  string resource;
  int book;
  string text;
  
  resource = "Albert Barnes' Notes on the Whole Bible (studylight-eng/bnb)";
  book = 58; // Hebrews.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter_string_html2text (text);
  evaluate (__LINE__, __func__, 15, text.find("The accomplishment of our high priest"));

  
  return; // Todo
  
  resource = "Expository Notes of Dr. Thomas Constable (studylight-eng/dcc)";
  book = 58; // Hebrews.
  //Verses 1-18
  //3. The accomplishment of our high priest10:1-18
  //This section on the high priestly ministry of Christ ( Hebrews 7:1 to  Hebrews 10:18) concludes with this pericope in which the writer emphasized the perfecting effect of Jesus Christ"s sacrifice on New Covenant believers. He wrote this to impress his readers further with the superiority of their condition compared with that of Old Covenant believers.
  //As pointed out previously,  Hebrews 7:1 to  Hebrews 10:18 constitutes an exposition of distinctive features of the high priestly office of the Son. These are its similarity to the priesthood of Melchizedek (ch7), the fact that it involved a single, personal sacrifice for sins (chs8-9), and its achievement of eternal salvation ( Hebrews 10:1-18).
  //". . . in  Hebrews 10:1-18 the writer elaborates the "subjective" effects of Christ"s offering for the community that enjoys the blessings of the new covenant. Christ"s death is considered from the perspective of its efficacy for Christians." [Note: Lane, Hebrews 9-13, p258.]
  //The argument is again chiastic.
  //A  The inadequacy of the Old Covenant: repeated sacrifices were necessary ( Hebrews 10:1-4)
  //B  The one sacrifice of Christ: supersedes the repeated sacrifices ( Hebrews 10:5-10)
  //B"  The priesthood of Christ: supersedes the Levitical priesthood ( Hebrews 10:11-14)
  //A"  The adequacy of the New Covenant: no more sacrifice for sins is necessary ( Hebrews 10:15-18)
  //Verses 11-14
  //The writer stressed the finality of Jesus Christ"s offering further with the contrast in these verses. [Note: See William David Spencer, "Christ"s Sacrifice as Apologetic: An Application of Hebrews 10:1-18," Journal of the Evangelical Theological Society40:2 (June1997):189-97, for a response to resurgent contemporary paganism that encourages self-sacrifice and other types of blood sacrifices.]  The Levitical priests never sat down because they never finished their work, but Jesus Christ sat down beside His Father because He finished His work.
  //"A seated priest is the guarantee of a finished work and an accepted sacrifice." [Note: Bruce, The Epistle ..., p239.]
  //Jesus Christ now awaits the final destruction of His enemies. Those who "are sanctified" (Hebrews 10:14) are those whom Jesus Christ has perfected and are consequently fully acceptable to God (i.e, all believers). [Note: See Kendall, pp180-82.]
  //Jesus Christ"s sacrifice has accomplished three things for us. It has cleansed our consciences from guilt, it has fitted us to approach God as worshippers, and it has fulfilled what the Old Testament promised.
  text = resource_logic_study_light_get (resource, book, 10, 14);
  text = filter_string_html2text (text);
  evaluate (__LINE__, __func__, 15, text.find("The accomplishment of our high priest"));
  evaluate (__LINE__, __func__, 501, text.find("distinctive features of the high priestly office of the Son"));
  evaluate (__LINE__, __func__, 1165, text.find("The one sacrifice of Christ"));
  evaluate (__LINE__, __func__, 1463, text.find("the finality of Jesus Christ\"s offering"));
  evaluate (__LINE__, __func__, 2221, text.find("those whom Jesus Christ has perfected"));
}
