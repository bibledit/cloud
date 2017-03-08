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


$ (document).ready (function () 
{
  navigationNewPassage ();
});


var publicFeedbackNavigationBible;
var publicFeedbackNavigationBook;
var publicFeedbackNavigationChapter;
var publicFeedbackNavigationVerse;


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
    publicFeedbackNavigationBible = navigationBible;
    publicFeedbackNavigationBook = navigationBook;
    publicFeedbackNavigationChapter = navigationChapter;
    publicFeedbackNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    publicFeedbackNavigationBible = parent.window.navigationBible;
    publicFeedbackNavigationBook = parent.window.navigationBook;
    publicFeedbackNavigationChapter = parent.window.navigationChapter;
    publicFeedbackNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }

  publicFeedbackLoadVerse ();
}


function publicFeedbackLoadVerse ()
{
  $.ajax ({
    url: "new",
    type: "GET",
    data: { bible: publicFeedbackNavigationBible, book: publicFeedbackNavigationBook, chapter: publicFeedbackNavigationChapter, verse: publicFeedbackNavigationVerse },
    success: function (response) {
      $ ("#versetext").empty ();
      $ ("#versetext").append (response);
    },
  });
}
