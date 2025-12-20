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
  document.querySelector ("#loadbutton").focus ();
  document.querySelector ("#loadbutton").addEventListener ("keypress", function (event) {
    if (event.keyCode == 13) {
      startLoad ();
    }
  });
  document.querySelector ("#loadbutton").addEventListener ("click", function (event) {
    startLoad ();
  });
  document.querySelector ("#strongslinks").addEventListener ("click", function (event) {
    startSearch (event);
  });
});


var hits = [];
var hitCounter;
var abortController = new AbortController();


function startLoad ()
{
  initPage ();

  const url = "strong?" + new URLSearchParams([ ["b", searchBible], ["load", true] ]).toString();
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
    document.querySelector ("#strongslinks").insertAdjacentHTML('beforeend', response);
    document.querySelector ("#searchbutton").removeAttribute ("disabled");
  })
  .catch((error) => {
    console.log(error);
  })
}


function startSearch (event)
{
  initPage ();
  event.preventDefault ();
  var strong = event.target.textContent;

  document.querySelector ("#searchloading").hidden = false;
  document.querySelector ("progress").setAttribute ("value", 0);
  document.querySelector ("progress").hidden = false;
  
  const url = "strong?" + new URLSearchParams([ ["b", searchBible], ["strong", strong] ]).toString();
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
  const url = "strong?" + new URLSearchParams([ ["b", searchBible], ["id", hits[hitCounter]] ]).toString();
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

  var searchoriginals = document.querySelector ("#searchoriginals");
  if (searchoriginals) if (searchoriginals.checked) target = 1;
  var searchtreasury = document.querySelector ("#searchtreasury");
  if (searchtreasury) if (searchtreasury.checked) target = 2;

  document.querySelector ("#strongslinks").innerHTML = "";

  document.querySelector ("#searchresults").innerHTML = "";
}
