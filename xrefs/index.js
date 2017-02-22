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


$(document).ready (function () {
  navigationNewPassage ();
  $ (window).on ("keydown", function (event) {
    xrefHandleKeyDown (event);
  });
});


var xrefNavigationBook;
var xrefNavigationChapter;
var xrefNavigationVerse;
var xrefBook;
var xrefChapter;
var xrefVerse;
var xrefFocusedLink;
var xrefMaxLinks;


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
    xrefNavigationBook = navigationBook;
    xrefNavigationChapter = navigationChapter;
    xrefNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    xrefNavigationBook = parent.window.navigationBook;
    xrefNavigationChapter = parent.window.navigationChapter;
    xrefNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }
  xrefLoadSourceTarget ();
}


function xrefLoadSourceTarget ()
{
  if ((xrefNavigationBook != xrefBook) || (xrefNavigationChapter != xrefChapter) || (xrefNavigationVerse != xrefVerse)) {
    xrefBook = xrefNavigationBook;
    xrefChapter = xrefNavigationChapter;
    xrefVerse = xrefNavigationVerse;
    xrefLoadSource ();
    xrefLoadTarget ();
    xrefFocusedLink = 1;
  }
}


function xrefLoadSource ()
{
  $.ajax ({
    url: "source",
    type: "GET",
    data: { book: xrefBook, chapter: xrefChapter, verse: xrefVerse },
    success: function (response) {
      $ ("#sourcetext").empty ();
      $ ("#sourcetext").append (response);
      xrefHighlight ();
    },
  });
}


function xrefLoadTarget ()
{
  $.ajax ({
    url: "target",
    type: "GET",
    data: { book: xrefBook, chapter: xrefChapter, verse: xrefVerse },
    success: function (response) {
      $ ("#targettext").empty ();
      $ ("#targettext").append (response);
      $ ("#targettext").focus ();
      xrefMaxLinks = $ ("#targettext a").length / 2;
      xrefHighlight ();
    },
  });
}


function xrefHandleKeyDown (event)
{
  if (event.altKey) return;
  if (event.ctrlKey) return;
  if (event.shiftKey) return;
  if (event.keyCode == 37) {
    // Left arrow.
    event.preventDefault ();
    xrefMove (-1);
  }
  if (event.keyCode == 38) {
    // Up arrow.
    event.preventDefault ();
    if (xrefFocusedLink <= 1) return;
    xrefFocusedLink--;
    xrefHighlight ();
  }
  if (event.keyCode == 39) {
    // Right arrow.
    event.preventDefault ();
    xrefMove (1);
  }
  if (event.keyCode == 40) {
    // Down arrow.
    event.preventDefault ();
    if (xrefFocusedLink >= xrefMaxLinks) return;
    xrefFocusedLink++;
    xrefHighlight ();
  }
}


function xrefHighlight ()
{
  $ (".focus").removeClass ("focus");
  $ ("#citation" + (xrefFocusedLink + 1000)).addClass ("focus");
  var link = $ ("#citation" + xrefFocusedLink);
  link.addClass ("focus");
}


function xrefMove (move)
{
  $.ajax ({
    url: "move",
    type: "GET",
    data: { book: xrefBook, chapter: xrefChapter, verse: xrefVerse, focus: xrefFocusedLink, move: move },
    success: function (response) {
      xrefLoadTarget ();
    },
  });
}
