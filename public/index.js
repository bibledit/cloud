/*
Copyright (©) 2003-2016 Teus Benschop.

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
var publicFeedbackLoadedBook;
var publicFeedbackLoadedChapter;


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

  if ((publicFeedbackNavigationBook != publicFeedbackLoadedBook) || (publicFeedbackNavigationChapter != publicFeedbackLoadedChapter)) {
    publicFeedbackLoadChapter ();
    publicFeedbackLoadNotes ();
  }
}


function publicFeedbackLoadChapter ()
{
  publicFeedbackLoadedBook = publicFeedbackNavigationBook;
  publicFeedbackLoadedChapter = publicFeedbackNavigationChapter;
  $.ajax ({
    url: "chapter",
    type: "GET",
    data: { bible: publicFeedbackNavigationBible, book: publicFeedbackLoadedBook, chapter: publicFeedbackLoadedChapter },
    success: function (response) {
      $ ("#publicchapter").empty ();
      $ ("#publicchapter").append (response);
    },
  });
}


function publicFeedbackLoadNotes ()
{
  $.ajax ({
    url: "notes",
    type: "GET",
    data: { bible: publicFeedbackNavigationBible, book: publicFeedbackLoadedBook, chapter: publicFeedbackLoadedChapter },
    success: function (response) {
      $ ("#publicnotes").empty ();
      $ ("#publicnotes").append (response);
    },
  });
}
