/*
Copyright (©) 2003-2026 Teus Benschop.

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
  var loadbutton = document.querySelector("#loadbutton");
  loadbutton.focus ();
  loadbutton.addEventListener("keypress", function (event) {
    if (event.keyCode == 13) {
      startLoad ();
    }
  });
  loadbutton.addEventListener ("click", startLoad);
  document.querySelector ("#searchentry").addEventListener ("keypress", function (event) {
    if (event.keyCode == 13) {
      startSearch ();
    }
  });
  var searchbutton = document.querySelector("#searchbutton");
  searchbutton.addEventListener ("keypress", function (event) {
    if (event.keyCode == 13) {
      startSearch ();
    }
  });
  searchbutton.addEventListener ("click", startSearch);
});


var hits = [];
var hitCounter;
var abortController = new AbortController();
var classs = "";


function startLoad ()
{
  initPage ();

  const url = "originals?" + new URLSearchParams([ ["b", searchBible], ["load", true] ]).toString();
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
    if (classs != "") document.querySelector ("#searchentry").classList.remove (classs);
    var lines = response.split ("\n");
    classs = lines.splice (0, 1) [0];
    response = lines.join ("\n");
    var searchentry = document.querySelector("#searchentry");
    searchentry.value = response;
    searchentry.classList.add (classs);
    searchentry.focus ();
    document.querySelector ("#searchbutton").removeAttribute ("disabled");
  })
  .catch((error) => {
    console.log(error);
  })
}


function startSearch ()
{
  initPage ();

  document.querySelector ("#searchloading").hidden = false;
  document.querySelector ("progress").setAttribute ("value", 0);
  document.querySelector ("progress").hidden = false;
  
  var words = document.querySelector ("#searchentry").value;

  const url = "originals?" + new URLSearchParams([ ["b", searchBible], ["words", words] ]).toString();
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
    fetchSearchHits ();
  });
}


function fetchSearchHits ()
{
  document.querySelector ("progress").setAttribute ("value", hitCounter);
  if (hitCounter >= hits.length) {
    document.querySelector ("progress").hidden = true;
    return;
  }
  const url = "originals?" + new URLSearchParams([ ["b", searchBible], ["id", hits[hitCounter]] ]).toString();
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
    fetchSearchHits ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function initPage ()
{
  abortController.abort("");
  abortController = new AbortController();

  document.querySelector ("#searchloading").hidden = true;

  document.querySelector ("progress").hidden = true;

  document.querySelector ("#hitcount").innerHTML = "0";
  hits.length = 0;

  document.querySelector ("#searchresults").innerHTML = "";
}

