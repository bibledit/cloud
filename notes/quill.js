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


var Delta = Quill.import ("delta");

var quill = null;

var arrayOfAdditionalAddresses = ["", ""];


$(document).ready (function () {
  noteLoadQuill();
  if ($ ("#create").length) $ ("#create").on ("click", noteCreate);
  if ($ ("#cancel").length) $ ("#cancel").on ("click", noteCancel);
  if ($ ("#create2").length) $ ("#create2").on ("click", noteCreate2);
  if ($ ("#cancel2").length) $ ("#cancel2").on ("click", noteCancel2);
  // The following if condition will add a query to supress the topbar
  // when this page is loaded as a workspace item.
  if (window.self !== window.top) {
    arrayOfAdditionalAddresses = ['?topbar=0','&topbar=0'];
  }
  // Upon creating a note, focus the summary line.
  if ($("#summary").length) $("#summary").focus();
  // Upon adding a comment to a note, focus the text area.
  else $(".ql-editor").focus();
  document.body.addEventListener('paste', NoteHandlePaste);
});


function noteLoadQuill ()
{
  quill = new Quill('#body', {
    modules: {
      toolbar: [
        [{ header: [1, 2, false] }],
        ['bold', 'italic', 'underline'],
      ]
    },
    placeholder: 'Write a note...',
    theme: 'snow'
  });

//  quill.clipboard.addMatcher (Node.ELEMENT_NODE, function (node, delta) {
//    var plaintext = $ (node).text ();
//    return new Delta().insert (plaintext);
//  });
}


function noteCreate ()
{
  var bible = $("#bible").val();
  var book = $("#book").val();
  var chapter = $("#chapter").val();
  var verse = $("#verse").val();
  var summary = $("#summary").val();
  var body = $ ("#body > .ql-editor").html ();
  summary = filter_url_plus_to_tag (summary);
  body = filter_url_plus_to_tag (body);
  $.ajax ({
    url: "create",
    type: "POST",
    async: false,
    data: { bible: bible, book: book, chapter: chapter, verse: verse, summary: summary, body: body },
    complete: function (xhr, status) {
      window.location.assign ("index" + arrayOfAdditionalAddresses[0]);
    }
  });
}


function noteCancel ()
{
  window.location.assign ("index" + arrayOfAdditionalAddresses[0]);
}


function noteCreate2 ()
{
  var body = $ ("#body > .ql-editor").html ();
  body = filter_url_plus_to_tag (body);
  $.ajax ({
    url: "comment",
    type: "POST",
    async: false,
    data: { id: noteId, body: body },
    complete: function (xhr, status) {
      window.location.assign ("note?id=" + noteId + "&temporal=" + arrayOfAdditionalAddresses[1]);
    }
  });
}


function noteCancel2 ()
{
  window.location.assign ("note?id=" + noteId + arrayOfAdditionalAddresses[1]);
}


// Pasting text into Quill has a few problems:
// https://github.com/bibledit/cloud/issues/717
// This paste handler aims to fix those.
function NoteHandlePaste (e)
{
  // Stop data from actually being pasted.
  e.stopPropagation();
  e.preventDefault();

  // Get the selected range, if any.
  // If the editor was not focused, the range is not defined:
  // Nothing more to do so bail out.
  var range = quill.getSelection();
  if (!range) return;
  
  // Unselect any selected text.
  if (range.length != 0) {
    quill.setSelection (range.index, range.length);
  }

  // Get pasted data via clipboard API.
  var clipboardData = e.clipboardData || window.clipboardData;
  var pastedData = clipboardData.getData('Text');

  // Inserting the text from the clipboard immediately into the editor does not work.
  // The work-around is to store data about where and what to paste.
  // And then to do the actual insert after a very short delay.
  notePasteIndex = range.index;
  notePasteText = pastedData;
  setTimeout (NoteHandlePasteInsert, 10);
}

var notePasteIndex = 0;
var notePasteText = "";


function NoteHandlePasteInsert()
{
  // Insert the text from the clipboard into the editor.
  quill.insertText(notePasteIndex, notePasteText);
}
