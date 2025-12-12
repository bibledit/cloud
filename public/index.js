/*
Copyright (©) 2003-2025 Teus Benschop.

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


document.addEventListener("DOMContentLoaded", function(e) {
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
  if (is_outside_workspace()) {
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
  const url = "chapter?" + new URLSearchParams([ ["bible", publicFeedbackNavigationBible], ["book", publicFeedbackLoadedBook], ["chapter", publicFeedbackLoadedChapter] ]).toString();
  fetch(url, {
    method: "GET",
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    var publicchapter = document.querySelector("#publicchapter");
    publicchapter.innerHTML = response;
  })
  .catch((error) => {
    console.log(error);
  })
}


function publicFeedbackLoadNotes ()
{
  const url = "notes?" + new URLSearchParams([ ["bible", publicFeedbackNavigationBible], ["book", publicFeedbackLoadedBook], ["chapter", publicFeedbackLoadedChapter] ]).toString();
  fetch(url, {
    method: "GET",
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    var publicnotes = document.querySelector("#publicnotes");
    publicnotes.innerHTML = response;
  })
  .catch((error) => {
    console.log(error);
  })
}
