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


var Delta = Quill.import ("delta");

var quill = null;

var arrayOfAdditionalAddresses = ["", ""];


document.addEventListener("DOMContentLoaded", function(e) {
  noteLoadQuill();
  var create = document.querySelector("#create");
  if (create) {
    create.addEventListener("click", noteCreate);
  }
  var cancel = document.querySelector("#cancel");
  if (cancel) {
    cancel.addEventListener("click", noteCancel);
  }
  var create2 = document.querySelector("#create2");
  if (create2) {
    create2.addEventListener("click", noteCreate2);
  }
  var cancel2 = document.querySelector("#cancel2");
  if (cancel2) {
    cancel2.addEventListener("click", noteCancel2);
  }
  // The following if condition will add a query to suppress the topbar
  // when this page is loaded as a workspace item.
  if (window.self !== window.top) {
    arrayOfAdditionalAddresses = ['?topbar=0','&topbar=0'];
  }
  // Upon creating a note, focus the summary line.
  // Upon adding a comment to a note, focus the text area.
  var summary = document.querySelector("#summary");
  if (summary)
    summary.focus();
  else
    document.querySelector(".ql-editor").focus();
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
}


function noteCreate ()
{
  var bible = document.querySelector("#bible").value;
  var book = document.querySelector("#book").value;
  var chapter = document.querySelector("#chapter").value;
  var verse = document.querySelector("#verse").value;
  var summary = document.querySelector("#summary").value;
  var body = document.querySelector("#body > .ql-editor").innerHTML;
  summary = filter_url_plus_to_tag (summary);
  body = filter_url_plus_to_tag (body);
  fetch("create", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([ ["bible", bible], ["book", book], ["chapter", chapter], ["verse", verse], ["summary", summary], ["body", body] ]).toString(),
    keepalive: true,
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    window.location.assign ("index" + arrayOfAdditionalAddresses[0]);
  });
}


function noteCancel ()
{
  window.location.assign ("index" + arrayOfAdditionalAddresses[0]);
}


function noteCreate2 ()
{
  var body = document.querySelector ("#body > .ql-editor").innerHTML;
  body = filter_url_plus_to_tag (body);
  fetch("comment", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([ ["id", noteId], ["body", body] ]).toString(),
    keepalive: true,
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    window.location.assign ("note?id=" + noteId + "&temporal=" + arrayOfAdditionalAddresses[1]);
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
