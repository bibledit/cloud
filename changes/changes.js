/*
Copyright (©) 2003-2017 Teus Benschop.

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


$(document).ready(function() {
  changesFocusTimerId = 0;
  updateIdCount ();
  $("body").on ("keydown", keyDown);
  $ ("#ids").on ("click", handleClick);
  if (swipe_operations) {
    var entries = $ ("#ids");
    entries.swipe ( {
      swipeLeft:function (event, direction, distance, duration, fingerCount, fingerData) {
        handleSwipeAway (event);
      },
      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
        handleSwipeExpand (event);
      }
    });
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
    passageConnectToAll ();
    return;
  }
  ajaxRequest = $.ajax ({
    url: "changes",
    type: "GET",
    data: { load: pendingidentifiers[0] },
    success: function (response) {
      $ ("#ids").append (response);
      if (!firstNotificationSelected) {
        setTimeout (initiallySelectFirstNotification, 100);
        firstNotificationSelected = true;
      }
      updateIdCount ();
      pendingidentifiers.shift ();
    },
    complete: function (xhr, status) {
      setTimeout (fetchChangeNotifications, 10);
    }
  });
}


function initiallySelectFirstNotification ()
{
  var entries = $ (notificationEntriesSelector);
  selectEntry (entries.first ());

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

  var entry = $(event.target).closest (notificationEntriesSelector);

  selectEntry (entry);

  var href = $(event.target).attr ("href");
  if (!href) {
    return;
  }

  var identifier = entry.attr ("id").substring (5, 100);

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
  var current = $(".selected");
  if (!current) return undefined;
  var next = current.next ("div");
  if (next.length) return next;
  return current;
}


function getPreviousEntry () {
  var current = $(".selected");
  if (!current) return undefined;
  var prev = current.prev ("div");
  if (prev.length) return prev;
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
  var current = $(".selected");
  var attribute = current.attr ("id");
  if (!attribute) return 0;
  var identifier = attribute.substring (5, 100);
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
  var newEntry = getEntryAfterDelete ();
  removeEntry ();
  selectEntry (newEntry);
}


function handleSwipeExpand (event)
{
  var entry = $(event.target).closest (notificationEntriesSelector);
  selectEntry (entry);
  toggleEntry ();
}
