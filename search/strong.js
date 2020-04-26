/*
Copyright (©) 2003-2020 Teus Benschop.

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
  $ ("#loadbutton").focus ();
  $ ("#loadbutton").on ("keypress", function (event) {
    if (event.keyCode == 13) {
      startLoad ();
    }
  });
  $ ("#loadbutton").on ("click", function (event) {
    startLoad ();
  });
  $ ("#strongslinks").on ("click", function (event) {
    startSearch (event);
  });
});


var hits = [];
var hitCounter;
var ajaxRequest;


function startLoad ()
{
  initPage ();

  ajaxRequest = $.ajax ({
    url: "strong",
    type: "GET",
    data: { b: searchBible, load: true },
    success: function (response) {
      $ ("#strongslinks").append (response);
      $ ("#searchbutton").removeAttr ("disabled");
    },
  });
}


function startSearch (event)
{
  initPage ();
  event.preventDefault ();
  var strong = event.target.textContent;

  $ ("#searchloading").show ();
  $ ("progress").attr ("value", 0);
  $ ("progress").show ();
  
  ajaxRequest = $.ajax ({
    url: "strong",
    type: "GET",
    data: { b: searchBible, strong: strong },
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
    url: "strong",
    type: "GET",
    data: { b: searchBible, id: hits[hitCounter] },
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


function initPage ()
{
  try {
    ajaxRequest.abort ();
  } catch (err) {
  }

  $ ("#searchloading").hide ();

  $ ("progress").hide ();

  $ ("#hitcount").empty ();
  $ ("#hitcount").text ("0");
  hits.length = 0;

  if ($ ("#searchoriginals").prop ("checked")) target = 1;
  if ($ ("#searchtreasury").prop ("checked")) target = 2;

  $ ("#strongslinks").empty ();

  $ ("#searchresults").empty ();
}

