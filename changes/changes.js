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

  if (swipe_operations) { // Todo working here.
//    var entries = $ ("#ids");
//    entries.swipe ( {
//      swipeLeft:function (event, direction, distance, duration, fingerCount, fingerData) {
//        handleSwipeAway (event);
//      },
//      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
//        handleSwipeExpand (event);
//      }
//    });
    // The minimum distance to swipe is 10% of the screen width.
    // This is to eliminate small unintended swipes.
    let minSwipeDistance = parseInt(window.screen.width / 10);

    // Operate on the main container that has all change notification entries.
    var entries2 = document.querySelector("#ids");

    entries2.addEventListener('touchstart', event => {
      touchStartX = event.changedTouches[0].screenX;
    });

    entries2.addEventListener('touchend', event => {
      let touchEndX = event.changedTouches[0].screenX
      if (touchEndX < touchStartX - minSwipeDistance) {
        console.log(event);
      console.log(event.changedTouches[0].target);
      console.log(event.changedTouches[0].target.id);
        console.log("swipe left v2");
      }
      if (touchEndX > touchStartX + minSwipeDistance) {
        console.log(event);
        console.log("swipe right v2");
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
    selectEntry2 (entries[0]);
  }
}


function keyDown (event) {
  // Down arrow: Go to next entry.
  if (event.keyCode == 40) {
    event.preventDefault ();
    selectEntry2 (getNextEntry ());
  }
  // Up arrow: Go to previous entry.
  if (event.keyCode == 38) {
    event.preventDefault ();
    selectEntry2 (getPreviousEntry ());
  }
  // Delete the entry. // Todo working here.
  if (event.keyCode == 46) {
    var newEntry = getEntryAfterDelete2 ();
    removeEntry ();
    selectEntry2 (newEntry);
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


function handleClick (event) { // Todo working here.

  var entry = event.target.closest (notificationEntriesSelector);
  selectEntry2 (entry);

  var href = $(event.target).attr ("href");
  if (!href) {
    return;
  }

  var identifier = entry.id.substring (5, 100);
  console.log (identifier);

  if (href == "remove") {
    var newEntry = getEntryAfterDelete2 ();
    removeEntry ();
    selectEntry2 (newEntry);
    event.preventDefault ();
    return;
  }

  if (href == "expand") {
    toggleEntry ();
    event.preventDefault ();
    return;
  }
  
  if (href.substring (0, 11) == ("unsubscribe")) {
    $.post ("change", { unsubscribe:href });
    $ (event.target).fadeOut ();
    event.preventDefault ();
    return;
  }
  
  if (href.substring (0, 8) == ("unassign")) {
    $.post ("change", { unassign:href });
    $ (event.target).fadeOut ();
    event.preventDefault ();
    return;
  }
  
  if (href.substring (0, 6) == ("delete")) {
    $.post ("change", { delete:href });
    $ (event.target).parent ().parent ().fadeOut ();
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
  var current = $(".selected");
  if (!current) return undefined;
  var entry = current.next ("div");
  if (entry.length) return entry;
  entry = current.prev ("div");
  if (entry.length) return entry;
  return undefined;  
}


function getEntryAfterDelete2 () { // Todo writing this.
  var current = document.querySelector(".selected");
//  var current = $(".selected");
  if (!current) return undefined;
  var next = current.nextElementSibling;
  if (next) return next;

  var previous = current.previousElementSibling;
  if (previous) return previous;



//  var entry = current.next ("div");
//  if (entry.length) return entry;
//  entry = current.prev ("div");
//  if (entry.length) return entry;
  return undefined;
}


function selectEntry (entry)
{
  if (entry) {
    $(".selected").removeClass("selected");
    entry.addClass("selected");
    var entryOffset = entry.offset();
    if (entryOffset) {
      var elementOffset = entryOffset.top;
      var currentScrollTop = $("#workspacewrapper").scrollTop();
      var workspaceHeight = $("#workspacewrapper").height();
      $("#workspacewrapper").scrollTop(elementOffset + (entry.height() / 2) - (workspaceHeight / 2) + currentScrollTop);
      changesFocusTimerStart();
    }
  }
}


function selectEntry2 (entry)
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
  $.post ("changes", { remove:identifier });
  $(".selected").remove ();
  updateIdCount ();
}


function updateIdCount () {
  var idCount = $(notificationEntriesSelector).length;
  $("#count").html (idCount);
}


function expandEntry () {
  // Bail out if nothing has been selected.
  var current = $(".selected");
  if (!current) return;
  // Bail out if the entry is already expanded.
  if ($(".selected > div").length > 0) return;
  // Get the selected identifier.
  var identifier = getSelectedIdentifier ();
  // Get extra information through AJAX calls.
  $(".selected").append ($ ("<div>" + loading + "</div>"));
  $.ajax ({
    url: "change",
    type: "GET",
    data: { get: identifier },
    cache: false,
    success: function (response) {
      $(".selected > div").remove ();
      var extraInfo = $ ("<div>" + response + "</div>");
      $(".selected").append (extraInfo);
      noteClickSetup ();
      var scrollTop = $("#workspacewrapper").scrollTop();
      var elementTop = $(".selected").offset().top;
      var workspaceTop = $("#workspacewrapper").offset().top;
      var location = scrollTop + elementTop - workspaceTop;
      $("#workspacewrapper").animate({ scrollTop: location }, 500);
    },
  });
}


function collapseEntry () {
  $(".selected > div").remove ();
  selectEntry ($(".selected"));
}


function toggleEntry () {
  if ($(".selected > div").length > 0) {
    collapseEntry ();
  } else {
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
  $.post ("changes", { navigate: identifier });
}


function handleSwipeAway (event)
{
  var entry = $(event.target).closest (notificationEntriesSelector);
  selectEntry (entry);
  var newEntry = getEntryAfterDelete2 ();
  removeEntry ();
  selectEntry2 (newEntry);
}


function handleSwipeExpand (event)
{
  var entry = $(event.target).closest (notificationEntriesSelector);
  selectEntry (entry);
  toggleEntry ();
}
