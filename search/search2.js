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


$(document).ready (function () {
  $ ("#searchloading").hide ();
  $ ("progress").hide ();
  $ ("#searchentry").focus ();
  $ ("#searchentry").on ("keypress", function (event) {
    if (event.keyCode == 13) {
      startSearch ();
    }
  });
  $ ("#searchbutton").on ("click", function (event) {
    startSearch ();
  });
  identifier = Math.floor ((Math.random () * 1'000'000) + 1'000'000);
});


var identifier;
var hits = [];
var hitCounter;
var ajaxRequest;


function startSearch ()
{
  try {
    ajaxRequest.abort ();
  } catch (err) {
  }
  var query = $ ("#searchentry").val ();
  if (query == "") return;
  var casesensitive = $ ("#casesensitive").prop ("checked");
  var searchplain = $ ("#searchplain").prop ("checked");
  var currentbook = $ ("#currentbook").prop ("checked");
  var sharing = "load";
  if ($ ("#add").prop ("checked")) sharing = "add";
  if ($ ("#remove").prop ("checked")) sharing = "remove";
  if ($ ("#intersect").prop ("checked")) sharing = "intersect";
  $ ("#searchloading").show ();
  $ ("progress").attr ("value", 0);
  $ ("progress").show ();
  $ ("#searchresults").empty ();
  $ ("#hitcount").empty ();
  hits.length = 0;
  ajaxRequest = $.ajax ({
    url: "search2",
    type: "GET",
    data: { bible: searchBible, i: identifier, q: query, c: casesensitive, p: searchplain, b: currentbook, s: sharing },
    success: function (response) {
      var ids = response.split ("\n");
      for (var i = 0; i < ids.length; i++) {
        var id = ids [i];
        if (id != "") {
          hits.push (id);
        }
      }
    },
    complete: function (xhr, status) {
      $ ("#searchloading").hide ();
      $ ("#hitcount").text (hits.length);
      $ ("progress").attr ("max", hits.length);
      hitCounter = 0;
      fetchSearchHits ();
    }
  });
}


function fetchSearchHits ()
{
  $ ("progress").attr ("value", hitCounter);
  if (hitCounter >= hits.length) {
    $ ("progress").hide (1000);
    return;
  }
  ajaxRequest = $.ajax ({
    url: "search2",
    type: "GET",
    data: { i: identifier, h: hits[hitCounter] },
    success: function (response) {
      $ ("#searchresults").append (response);
      passageConnectToLast ();
      hitCounter++;
      fetchSearchHits ();
    },
    complete: function (xhr, status) {
    }
  });
}

