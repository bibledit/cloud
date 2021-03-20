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


var changedTimeout;
var identifier;
var pollerTimeout;
var previousPassages;
var previousTranslations;


$(document).ready (function () {
  $ ("#status").hide ();
  $ ("#passages").focus ();
  $ ("textarea").on ("paste cut keydown", function (event) {
    if (changedTimeout) clearTimeout (changedTimeout);
    changedTimeout = setTimeout (changed, 500);
  });
});


function changed ()
{
  var passages = $('#passages').val();
  var translations = $('#translations').val();
  if ((passages == previousPassages) && (translations == previousTranslations)) return;
  identifier = Math.floor ((Math.random () * 1000000) + 1000000);
  $ ("#status").show ();
  $.ajax ({
    url: "input",
    type: "POST",
    data: { id: identifier, passages: passages, translations: translations },
    success: function (response) {
      $ ("#texts").empty ();
      $ ("#texts").append (response);
      $ ("#status").hide ();
      navigationSetup ();
      passageConnectToAll ();
    },
    complete: function (xhr, status) {
      delayedPoll ();
    }
  });
}


function delayedPoll ()
{
  if (pollerTimeout) {
    clearTimeout (pollerTimeout);
  }
  pollerTimeout = setTimeout (poll, 1000);
}


function poll ()
{
  $.ajax ({
    url: "poll",
    type: "GET",
    data: { id: identifier },
    success: function (response) {
      if (response != "") {
        $ ("#texts").empty ();
        $ ("#texts").append (response);
        passageConnectToAll ();
      }
    },
    complete: function (xhr, status) {
      delayedPoll ();
    }
  });
}

