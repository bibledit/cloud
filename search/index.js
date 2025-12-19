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
  var searchloading = document.querySelector("#searchloading");
  searchloading.hidden = true;
  var progress = document.querySelector("progress");
  progress.hidden = true;
  var searchentry = document.querySelector("#searchentry");
  searchentry.focus ();
  searchentry.addEventListener("keypress", function (event) {
    if (event.keyCode == 13) {
      startSearch ();
    }
  });
  document.querySelector("#searchbutton").addEventListener("click", function (event) {
    startSearch ();
  });
});


var query;
var hits = [];
var hitCounter;
var abortController = new AbortController();


function startSearch ()
{
  abortController.abort("");
  abortController = new AbortController();
  var searchentry = document.querySelector("#searchentry");
  query = searchentry.value;
  if (query == "") return;
  var searchloading = document.querySelector("#searchloading");
  searchloading.hidden = false;
  var progress = document.querySelector("progress");
  progress.setAttribute("value", 0);
  progress.hidden = false;
  document.querySelector("#searchresults").innerHTML = "";
  document.querySelector("#hitcount").innerHTML = "";
  document.querySelector("#help").hidden = true;
  hits.length = 0;
  const url = "index?" + new URLSearchParams([ ["q", query], ["b", searchBible] ]).toString();
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
    var searchloading = document.querySelector("#searchloading");
    if (searchloading) {
      searchloading.hidden = true;
    }
    document.querySelector("#hitcount").innerHTML = hits.length;
    document.querySelector("progress").setAttribute ("max", hits.length);
    hitCounter = 0;
    fetchSearchHits ();
  });
}


function fetchSearchHits ()
{
  var progress = document.querySelector("progress");
  progress.setAttribute("value", hitCounter);
  if (hitCounter >= hits.length) {
    if (progress) {
      progress.hidden = true;
    }
    return;
  }
  const url = "index?" + new URLSearchParams([ ["id", hits[hitCounter]], ["q", query], ["b", searchBible] ]).toString();
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
    document.querySelector("#searchresults").insertAdjacentHTML('beforeend', response);
    passageConnectToLast ();
    hitCounter++;
    fetchSearchHits ();
  })
  .catch((error) => {
    console.log(error);
  })
}

