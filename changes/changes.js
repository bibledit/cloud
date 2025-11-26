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


var touchStartX = 0

document.addEventListener("DOMContentLoaded", function(e) {
  changesFocusTimerId = 0;
  updateIdCount ();
  document.body.addEventListener("keydown", keyDown);
  document.querySelector("#ids").addEventListener("click", handleClick);

  if (swipe_operations) {
    // The minimum distance to swipe is 10% of the screen width.
    // This is to eliminate small unintended swipes.
    let minSwipeDistance = parseInt(window.screen.width / 10);

    // Operate on the main container that has all change notification entries.
    var entries = document.querySelector("#ids");

    entries.addEventListener('touchstart', event => {
      touchStartX = event.changedTouches[0].screenX;
    });

    entries.addEventListener('touchend', event => {
      let touchEndX = event.changedTouches[0].screenX
      if (touchEndX < touchStartX - minSwipeDistance) {
        handleSwipeAway (event);
      }
      if (touchEndX > touchStartX + minSwipeDistance) {
        handleSwipeExpand (event);
      }
    })
  }
  navigationSetup ();
  if (pendingidentifiers) pendingidentifiers = pendingidentifiers.split (" ");
  else pendingidentifiers = [];
  fetchChangeNotifications ();
});


var notificationEntriesSelector = "div[id^='entry']";
var firstNotificationSelected = false;


function fetchChangeNotifications ()
{
  if (pendingidentifiers.length == 0) {
    return;
  }
  let url = "changes?load=" + pendingidentifiers[0];
  fetch(url)
  .then(response => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then(text => {
    document.querySelector("#ids").insertAdjacentHTML('beforeend', text);
    // Activate the passage link straightaway.
    // https://github.com/bibledit/cloud/issues/273
    passageConnectToLast ();
    if (!firstNotificationSelected) {
      setTimeout (initiallySelectFirstNotification, 100);
      firstNotificationSelected = true;
    }
    updateIdCount ();
    pendingidentifiers.shift ();
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    setTimeout (fetchChangeNotifications, 10);
  });
}


function initiallySelectFirstNotification ()
{
  let entries = document.querySelectorAll(notificationEntriesSelector);
  if (entries.length > 0) {
    selectEntry (entries[0]);
  }
}


function keyDown (event) {
  // Down arrow: Go to next entry.
  if (event.keyCode == 40) {
    event.preventDefault ();
    selectEntry (getNextEntry ());
  }
  // Up arrow: Go to previous entry.
  if (event.keyCode == 38) {
    event.preventDefault ();
    selectEntry (getPreviousEntry ());
  }
  // Delete the entry.
  if (event.keyCode == 46) {
    var newEntry = getEntryAfterDelete ();
    removeEntry ();
    selectEntry (newEntry);
  }
  // Right arrow: Expand entry.
  if (event.keyCode == 39) {
    event.preventDefault ();
    expandEntry ();
  }
  // Left arrow: Collapse entry.
  if (event.keyCode == 37) {
    event.preventDefault ();
    collapseEntry ();
  }
}


function handleClick (event) {

  const entry = event.target.closest (notificationEntriesSelector);
  selectEntry (entry);

  const href = event.target.getAttribute('href');
  if (!href) {
    return;
  }

  var identifier = entry.id.substring (5, 100);

  if (href == "remove") {
    var newEntry = getEntryAfterDelete ();
    removeEntry ();
    selectEntry (newEntry);
    event.preventDefault ();
    return;
  }

  if (href == "expand") {
    toggleEntry ();
    event.preventDefault ();
    return;
  }
  
  if (href.substring (0, 11) == ("unsubscribe")) {
    fetch("change", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: new URLSearchParams([["unsubscribe", href]]).toString(),
    })
    event.target.remove();
    event.preventDefault ();
    return;
  }
  
  if (href.substring (0, 8) == ("unassign")) {
    fetch("change", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: new URLSearchParams([["unassign", href]]).toString(),
    })
    event.target.remove();
    event.preventDefault ();
    return;
  }
  
  if (href.substring (0, 6) == ("delete")) {
    fetch("change", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: new URLSearchParams([["delete", href]]).toString(),
    })
    event.target.parentElement.parentElement.remove()
    event.preventDefault ();
    return;
  }
}


function getNextEntry () {
  var current = document.querySelector(".selected");
  if (!current) return undefined;
  var next = current.nextElementSibling;
  if (next) return next;
  return current;
}


function getPreviousEntry () {
  var current = document.querySelector(".selected");
  if (!current) return undefined;
  var previous = current.previousElementSibling;
  if (previous) return previous;
  return current;
}


function getEntryAfterDelete () {
  var current = document.querySelector(".selected");
  if (!current) return undefined;
  var next = current.nextElementSibling;
  if (next) return next;
  var previous = current.previousElementSibling;
  if (previous) return previous;
  return undefined;
}


function selectEntry (entry)
{
  if (entry) {
    var selected = document.querySelector(".selected");
    if (selected) {
      selected.classList.remove("selected");
    }
    entry.classList.add("selected");
    entry.scrollIntoView({
      behavior: 'smooth',
      block: 'center',
      inline: 'center'
    });
    changesFocusTimerStart();
  }
}


function removeEntry () {
  var identifier = getSelectedIdentifier ();
  if (identifier == 0) return;
  fetch("changes", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([["remove", identifier]]).toString(),
  })
  document.querySelector(".selected").remove();
  updateIdCount ();
}


function updateIdCount () {
  var idCount = document.querySelectorAll(notificationEntriesSelector).length
  document.querySelector("#count").innerHTML = idCount
}


function expandEntry () {
  // Bail out if nothing has been selected.
  var current = document.querySelector(".selected");
  if (!current) return;
  // Bail out if the entry is already expanded.
  if (document.querySelectorAll(".selected > div").length > 0) return;
  // Get the selected identifier.
  var identifier = getSelectedIdentifier ();
  // Get extra information through AJAX calls.
  document.querySelector(".selected").insertAdjacentHTML('beforeend', "<div>" + loading + "</div>");
  const url = "change?get=" + identifier;
  fetch(url, {
    method: "GET",
    cache: "no-cache"
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((text) => {
    document.querySelector(".selected > div").remove ();
    var extraInfo = "<div>" + text + "</div>";
    let selected = document.querySelector(".selected");
    selected.insertAdjacentHTML('beforeend', extraInfo);
    noteClickSetup ();
    selected.scrollIntoView({
      behavior: 'smooth',
      block: 'center',
      inline: 'center'
    });
  })
  .catch((error) => {
  })
  .finally(() => {
  });
}


function collapseEntry () {
  selected = document.querySelector(".selected > div");
  if (selected)
    selected.remove ();
  selectEntry (document.querySelector(".selected"));
}


function toggleEntry () {
  var selectedDiv = document.querySelectorAll(".selected > div");
  if (selectedDiv) {
    if (selectedDiv.length > 0)
      collapseEntry ();
    else
      expandEntry ();
  }
}


function getSelectedIdentifier () {
  let selected = document.querySelector(".selected");
  if (!selected)
    return 0;
  let id = selected.id
  if (!id)
    return 0;
  let identifier = id.substring (5, 100);
  return identifier;
}


var changesFocusTimerId;


function changesFocusTimerStart ()
{
  if (changesFocusTimerId) clearTimeout (changesFocusTimerId);
  changesFocusTimerId = setTimeout (changesFocusTimeout, 300);
}


function changesFocusTimeout ()
{
  // Navigate to the passage of the entry.
  var identifier = getSelectedIdentifier ();
  fetch("changes", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([["navigate", identifier]]).toString(),
  })
}


function handleSwipeAway (event)
{
  var entry = event.target.closest (notificationEntriesSelector);
  if (entry) {
    selectEntry (entry);
    var newEntry = getEntryAfterDelete ();
    removeEntry ();
    selectEntry (newEntry);
  }
}


function handleSwipeExpand (event)
{
  var entry = event.target.closest (notificationEntriesSelector);
  if (entry) {
    selectEntry (entry);
    toggleEntry ();
  }
}
