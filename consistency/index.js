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


var changedTimeout;
var identifier;
var pollerTimeout;
var previousPassages;
var previousTranslations;


document.addEventListener("DOMContentLoaded", function(e) {
  document.querySelector("#status").hidden = true;
  document.querySelector("#passages").focus ();
  var textareas = document.querySelectorAll("textarea");
  textareas.forEach((textarea) => {
    textarea.addEventListener("paste", (event) => {
      delayChanged();
    });
    textarea.addEventListener("cut", (event) => {
      delayChanged();
    });
    textarea.addEventListener("keydown", (event) => {
      delayChanged();
    });
  });
});


function delayChanged() {
  if (changedTimeout) clearTimeout (changedTimeout);
  changedTimeout = setTimeout (changed, 500);
}

function changed ()
{
  var passages = document.querySelector('#passages').value
  var translations = document.querySelector('#translations').value;
  if ((passages == previousPassages) && (translations == previousTranslations)) return;
  identifier = Math.floor ((Math.random () * 1000000) + 1000000);
  document.querySelector("#status").hidden = false;
  fetch("input", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([["id", identifier], ["passages", passages], ["translations", translations]]).toString(),
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((text) => {
    var texts= document.querySelector("#texts");
    texts.innerHTML = "";
    texts.insertAdjacentHTML('beforeend', text);
    document.querySelector("#status").hidden = true;
    navigationSetup ();
    passageConnectToAll ();
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    delayedPoll ();
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
  const url = "poll?id=" + identifier;
  fetch(url, { 
    method: "GET"
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((text) => {
    if (text != "") {
      var texts= document.querySelector("#texts");
      texts.innerHTML = "";
      texts.insertAdjacentHTML('beforeend', text);
      passageConnectToAll ();
    }
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    delayedPoll ();
  });
}
