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
  $ ("#applybutton").hide ();
  $ ("#searchentry").focus ();
  $ ("#searchentry").on ("keypress", function (event) {
    if (event.keyCode == 13) {
      fetchIdentifiers ();
    }
  });
  $ ("#replaceentry").on ("keypress", function (event) {
    if (event.keyCode == 13) {
      fetchIdentifiers ();
    }
  });
  $ ("#previewbutton").on ("click", function (event) {
    fetchIdentifiers ();
  });
  $ ("#applybutton").on ("click", function (event) {
    replacingAll = true;
    replaceCounter = 0;
    $ ("progress").show ();
    $ ("#applybutton").hide ();
    replaceAll ();
  });
  $ ("#searchresults").on ("click", function (event) {
    handleClick (event);
  });
});


var searchfor;
var replacewith;
var casesensitive;
var hits = [];
var hitCount = 0;
var hitCounter;
var ajaxRequest;
var replacingAll = false;
var replaceCounter = 0;


function fetchIdentifiers ()
{
  try {
    ajaxRequest.abort ();
  } catch (err) {
  }
  searchfor = $ ("#searchentry").val ();
  replacewith = $ ("#replaceentry").val ();
  casesensitive = $ ("#casesensitive").prop ("checked");
  if (searchfor == "") return;
  $ ("#searchloading").show ();
  $ ("progress").attr ("value", 0);
  $ ("progress").show ();
  $ ("#searchresults").empty ();
  $ ("#hitcount").empty ();
  $ ("#applybutton").hide ();
  $ ("#help").hide ();
  hits.length = 0;
  replacingAll = false;
  ajaxRequest = $.ajax ({
    url: "getids",
    type: "GET",
    data: { b: searchBible, q: searchfor, c: casesensitive },
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
      fetchPreviews ();
    }
  });
}


function fetchPreviews ()
{
  $ ("progress").attr ("value", hitCounter);
  if (hitCounter >= hits.length) {
    $ ("progress").hide (1000);
    hitCount = hits.length;
    hits.length = 0;
    if (hitCount > 0) {
      $ ("#applybutton").show (1000);
    }
    return;
  }
  ajaxRequest = $.ajax ({
    url: "replacepre",
    type: "GET",
    data: { q: searchfor, c: casesensitive, r: replacewith, id: hits[hitCounter] },
    success: function (response) {
      $ ("#searchresults").append (response);
      passageConnectToLast ();
      hitCounter++;
      fetchPreviews ();
    },
    complete: function (xhr, status) {
    }
  });
}


function handleClick (event)
{
  var target = $ (event.target);
  var href = target.attr ("href");
  var parent = target.parent ();
  var divparent = parent.parent ();
  if (href == "replace") {
    event.preventDefault ();
    var id = divparent.attr ("id");
    doReplace (id);
  }
  if (href == "delete") {
    event.preventDefault ();
    divparent.remove ();
    hitCount--;
    $ ("#hitcount").text (hitCount);
  }
}


function doReplace (identifier)
{
  ajaxRequest = $.ajax ({
    url: "replacego",
    type: "GET",
    context: identifier,
    data: { id: identifier, q: searchfor, c: casesensitive, r: replacewith  },
    success: function (response) {
      var element = $ ("#" + this);
      element.replaceWith (response);
      passageConnectToAll ();
      if (replacingAll) replaceAll ();
    },
    complete: function (xhr, status) {
    }
  });
}


function replaceAll ()
{
  replaceCounter++;
  $ ("progress").attr ("value", replaceCounter);
  var id = $ ("#searchresults > div").first ().attr ("id");
  if (id != undefined) {
    doReplace (id);
  } else {
    replacingAll = false;
    $ ("progress").hide (1000);
  }
}


