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


var Delta = Quill.import ("delta");

var quill = null;

$(document).ready (function () {
  noteLoadQuill();
  if ($ ("#create").length) $ ("#create").on ("click", noteCreate);
  if ($ ("#cancel").length) $ ("#cancel").on ("click", noteCancel);
  if ($ ("#create2").length) $ ("#create2").on ("click", noteCreate2);
  if ($ ("#cancel2").length) $ ("#cancel2").on ("click", noteCancel2);
});

function noteLoadQuill ()
{
  var quill = new Quill('#body', {
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
      window.location.assign ("index");
    }
  });
}

function noteCancel ()
{
  window.location.assign ("index");
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
      window.location.assign ("note?id=" + noteId + "&temporal=");
    }
  });
}

function noteCancel2 ()
{
  window.location.assign ("note?id=" + noteId);
}
