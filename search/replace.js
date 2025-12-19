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
  document.querySelector ("#searchloading").hidden = true;
  document.querySelector ("progress").hidden = true;
  document.querySelector ("#applybutton").hidden = true;
  document.querySelector ("#searchentry").focus ();
  document.querySelector ("#searchentry").addEventListener ("keypress", function (event) {
    if (event.keyCode == 13) {
      fetchIdentifiers ();
    }
  });
  document.querySelector ("#replaceentry").addEventListener ("keypress", function (event) {
    if (event.keyCode == 13) {
      fetchIdentifiers ();
    }
  });
  document.querySelector ("#previewbutton").addEventListener ("click", fetchIdentifiers);
  document.querySelector ("#applybutton").addEventListener ("click", function (event) {
    replacingAll = true;
    replaceCounter = 0;
    document.querySelector ("progress").hidden = false;
    document.querySelector ("#applybutton").hidden = false;
    replaceAll ();
  });
  document.querySelector ("#searchresults").addEventListener ("click", function (event) {
    handleClick (event);
  });
});


var searchfor;
var replacewith;
var casesensitive;
var hits = [];
var hitCount = 0;
var hitCounter;
var abortController = new AbortController();
var replacingAll = false;
var replaceCounter = 0;


function fetchIdentifiers ()
{
  abortController.abort("");
  abortController = new AbortController();
  searchfor = document.querySelector ("#searchentry").value;
  replacewith = document.querySelector ("#replaceentry").value;
  casesensitive = document.querySelector ("#casesensitive").checked;
  if (searchfor == "") return;
  document.querySelector ("#searchloading").hidden = false;
  document.querySelector ("progress").setAttribute ("value", 0);
  document.querySelector ("progress").hidden = false;
  document.querySelector ("#searchresults").innerHTML = "";
  document.querySelector ("#hitcount").innerHTML = "";
  document.querySelector ("#applybutton").hidden = true;
  document.querySelector ("#help").hidden = true;
  hits.length = 0;
  replacingAll = false;
  const url = "getids?" + new URLSearchParams([ ["b", searchBible], ["q", searchfor], ["c", casesensitive] ]).toString();
  fetch(url, {
    method: "GET",
    signal: abortController.signal
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    var ids = response.split ("\n");
    for (var i = 0; i < ids.length; i++) {
      var id = ids [i];
      if (id != "") {
        hits.push (id);
      }
    }
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    document.querySelector ("#searchloading").hidden = true;
    document.querySelector ("#hitcount").innerHTML = hits.length;
    document.querySelector ("progress").setAttribute ("max", hits.length);
    hitCounter = 0;
    fetchPreviews ();
  });
}


function fetchPreviews ()
{
  document.querySelector ("progress").setAttribute ("value", hitCounter);
  if (hitCounter >= hits.length) {
    document.querySelector ("progress").hidden = true;
    hitCount = hits.length;
    hits.length = 0;
    if (hitCount > 0) {
      document.querySelector ("#applybutton").hidden = false;
    }
    return;
  }
  const url = "replacepre?" + new URLSearchParams([ ["q", searchfor], ["c", casesensitive], ["r", replacewith], ["id", hits[hitCounter]] ]).toString();
  fetch(url, {
    method: "GET",
    signal: abortController.signal
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    document.querySelector ("#searchresults").insertAdjacentHTML('beforeend', response);
    passageConnectToLast ();
    hitCounter++;
    fetchPreviews ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function handleClick (event)
{
  var href = event.target.getAttribute ("href");
  var parent = event.target.parentElement;
  var divparent = parent.parentElement;
  if (href == "replace") {
    event.preventDefault ();
    var id = divparent.getAttribute ("id");
    doReplace (id);
  }
  if (href == "delete") {
    event.preventDefault ();
    divparent.remove ();
    hitCount--;
    document.querySelector ("#hitcount").innerHTML = hitCount;
  }
}


var replaceIdentifier = 0;

function doReplace (identifier)
{
  replaceIdentifier = identifier;
  const url = "replacego?" + new URLSearchParams([ ["id", identifier], ["q", searchfor], ["c", casesensitive], ["r", replacewith] ]).toString();
  fetch(url, {
    method: "GET",
    signal: abortController.signal,
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    var element = document.getElementById (String(replaceIdentifier));
    element.outerHTML = response;
    passageConnectToAll ();
    if (replacingAll) replaceAll ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function replaceAll ()
{
  replaceCounter++;
  document.querySelector ("progress").setAttribute ("value", replaceCounter);
  var ids = document.querySelectorAll ("#searchresults > div");
  if (ids.length) {
    var id = ids[0].getAttribute("id");
    doReplace (id);
  } else {
    replacingAll = false;
    document.querySelector ("progress").hidden = true;
  }
}
