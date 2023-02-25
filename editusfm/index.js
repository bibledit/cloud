/*
Copyright (©) 2003-2023 Teus Benschop.

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


var usfmEditorUniqueID = Math.floor (Math.random() * 100000000);


$(document).ready (function ()
{
  // Listens for bibleselect option tags value change to update the loaded Bible.
  var bibleSelectionElement = document.querySelector ("#bibleselect");
  bibleSelectionElement.addEventListener ('change', () => {
    $.post ("index", { bibleselect: bibleSelectionElement.value })
      .done (function() { window.location.reload () });
  });

  // Make the editor's menu to never scroll out of view.
  var bar = $ ("#editorheader").remove ();
  $ ("#workspacemenu").append (bar);
  
  rangy.init ();
  navigationNewPassage ();
  $ ("#usfmeditor").on ("paste cut keydown", usfmEditorChanged);
  $ (window).on ("unload", usfmEditorUnload);
  $ ("#usfmeditor").on ("paste", function (e) {
    var data = e.originalEvent.clipboardData.getData ('Text');
    e.preventDefault();
    document.execCommand ("insertHTML", false, data);
  });
  usfmIdPollerOn ();
  $ ("#usfmeditor").on ("paste cut click", usfmCaretChanged);
  $ ("#usfmeditor").on ("keydown", usfmHandleKeyDown);
  if (usfmEditorWriteAccess) $ ("#usfmeditor").focus ();
  $ (window).on ("focus", usfmWindowFocused);
  if (swipe_operations) {
    $ ("body").swipe ( {
      swipeLeft:function (event, direction, distance, duration, fingerCount, fingerData) {
        editusfmSwipeLeft (event);
      },
      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
        editusfmSwipeRight (event);
      }
    });
  }
});


var usfmBible;
var usfmBook;
var usfmNavigationBook;
var usfmChapter;
var usfmNavigationChapter;
var usfmNavigationVerse;
var usfmEditorChangedTimeout;
var usfmLoadedText;
var usfmIdChapter = 0;
var usfmCaretTimeout;
var usfmReload = false;
var usfmCaretPosition = 0;
var usfmClarifierTimeout;
var usfmPreviousCaretTop;
var usfmPreviousWidth;
var usfmPreviousHeight;
var usfmEditorTextChanged = false;
var usfmSaveAsync;
var usfmSaving = false;


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
    usfmNavigationBook = navigationBook;
    usfmNavigationChapter = navigationChapter;
    usfmNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    usfmNavigationBook = parent.window.navigationBook;
    usfmNavigationChapter = parent.window.navigationChapter;
    usfmNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }
  usfmEditorSaveChapter ();
  usfmReload = false;
  usfmEditorLoadChapter ();
  if (usfmEditorWriteAccess) {
    usfmPositionCaretViaAjax ();
  } else {
    usfmPositionFocusedVerseViaAjax ();
  }
}


var usfmEditorReloadCount = 0;


function usfmEditorLoadChapter ()
{
  if ((usfmNavigationBook != usfmBook) || (usfmNavigationChapter != usfmChapter) || usfmReload) {
    usfmBible = navigationBible;
    usfmBook = usfmNavigationBook;
    usfmChapter = usfmNavigationChapter;
    usfmIdChapter = 0;
    if (usfmEditorWriteAccess) $ ("#usfmeditor").focus ();
    usfmCaretPosition = usfmGetCaretPosition ();
    $.ajax ({
      url: "load",
      type: "GET",
      data: { bible: usfmBible, book: usfmBook, chapter: usfmChapter, id: usfmEditorUniqueID },
      success: function (response) {
        usfmEditorWriteAccess = checksum_readwrite (response);
        // If this is the second or third or higher editor in the workspace,
        // make the editor read-only.
        if (window.frameElement) {
          iframe = $(window.frameElement);
          var data_editor_number = iframe.attr("data-editor-no");
          if (data_editor_number > 1) {
            usfmEditorWriteAccess = false;
          }
        }
        var contenteditable = ($ ("#usfmeditor").attr('contenteditable') === 'true');
        if (usfmEditorWriteAccess != contenteditable) $ ("#usfmeditor").attr('contenteditable', usfmEditorWriteAccess);
        // Checksumming.
        response = checksum_receive (response, usfmEditorReloadCount);
        if (response !== false) {
          $ ("#usfmeditor").empty ();
          $ ("#usfmeditor").append (response);
          usfmEditorStatus (usfmEditorChapterLoaded);
          usfmLoadedText = response;
          if (usfmEditorWriteAccess) {
            if (usfmReload) {
              usfmPositionCaret (usfmCaretPosition);
            } else {
              usfmPositionCaretViaAjax ();
            }
          }
          // Alert on reload.
          if (usfmReload) {
            if (usfmEditorWriteAccess) usfmEditorReloadAlert (usfmEditorVerseUpdatedLoaded);
          }
          usfmReload = false;
          usfmEditorReloadCount = 0;
        } else {
          // Checksum error: Reload.
          usfmReload = true;
          // Increase the counter, so that eventually it will disable checksumming,
          // so it can then load corrupted text in the USFM editor.
          // https://github.com/bibledit/cloud/issues/482
          usfmEditorReloadCount++
          // Try loading the chapter again after a short timeoout.
          setTimeout (usfmEditorLoadChapter, 100);
        }
        if (!usfmEditorWriteAccess) usfmPositionFocusedVerseViaAjax ();
      },
    });
  }
}


function usfmEditorUnload ()
{
  usfmEditorSaveChapter (true);
}


function usfmEditorSaveChapter (sync)
{
  // Fix for https://github.com/bibledit/cloud/issues/427
  usfmEditorStatus ("");

  if (usfmSaving) {
    usfmEditorChanged ();
    return;
  }
  if (!usfmEditorWriteAccess) return;
  usfmEditorTextChanged = false;
  if (!usfmBible) return;
  if (!usfmBook) return;
  var usfm = $ ("#usfmeditor").text ();
  if (usfm == usfmLoadedText) return;
  if (!usfm) return;
  usfmEditorStatus (usfmEditorChapterSaving);
  usfmLoadedText = usfm;
  usfmIdChapter = 0;
  usfmSaveAsync = true;
  if (sync) usfmSaveAsync = false;
  var encodedUsfm = filter_url_plus_to_tag (usfm);
  var checksum = checksum_get (encodedUsfm);
  usfmSaving = true;
  $.ajax ({
    url: "save",
    type: "POST",
    async: usfmSaveAsync,
    data: { bible: usfmBible, book: usfmBook, chapter: usfmChapter, usfm: encodedUsfm, checksum: checksum, id: usfmEditorUniqueID },
    error: function (jqXHR, textStatus, errorThrown) {
      usfmEditorStatus (usfmEditorChapterRetrying);
      usfmLoadedText = "";
      usfmEditorChanged ();
      if (!usfmSaveAsync) usfmEditorSaveChapter (true);
    },
    success: function (response) {
      usfmEditorStatus (response);
    },
    complete: function (xhr, status) {
      usfmSaveAsync = true;
      usfmSaving = false;
    }
  });
}


function usfmEditorChanged (event)
{
  if (editKeysIgnoreForContentChange (event)) return;
  usfmEditorStatus (usfmEditorWillSave);
  usfmEditorTextChanged = true;
  if (usfmEditorChangedTimeout) {
    clearTimeout (usfmEditorChangedTimeout);
  }
  usfmEditorChangedTimeout = setTimeout (usfmEditorSaveChapter, 1000);
  //restartCaretClarifier ();
}


function usfmEditorStatus (text)
{
  $ ("#usfmstatus").empty ();
  $ ("#usfmstatus").append (text);
  usfmEditorSelectiveNotification (text);
}


function usfmEditorSelectiveNotification (message)
{
  if (message == usfmEditorChapterLoaded) return;
  if (message == usfmEditorWillSave) return;
  if (message == usfmEditorChapterSaving) return;
  if (message == usfmEditorChapterSaved) return;
  if (message == "") return;
  notifyItError (message);
}


var usfmIdTimeout;
var usfmIdAjaxRequest;


function usfmIdPollerOff ()
{
  if (usfmIdTimeout) {
    clearTimeout (usfmIdTimeout);
  }
  if (usfmIdAjaxRequest && usfmIdAjaxRequest.readystate != 4) {
    usfmIdAjaxRequest.abort();
  }
}


function usfmIdPollerOn ()
{
  usfmIdPollerOff ();
  usfmIdTimeout = setTimeout (usfmEditorPollId, 1000);
}


function usfmEditorPollId ()
{
  usfmIdPollerOff ();
  $.ajax ({
    url: "../edit/id",
    type: "GET",
    data: { bible: usfmBible, book: usfmBook, chapter: usfmChapter },
    cache: false,
    success: function (response) {
      if (!usfmSaving) {
        if (usfmIdChapter != 0) {
          if (response != usfmIdChapter) {
            usfmReload = true;
            usfmEditorLoadChapter ();
            usfmIdChapter = 0;
          }
        }
        usfmIdChapter = response;
      }
    },
    complete: function (xhr, status) {
      if (status != "abort") {
        usfmIdPollerOn ();
      }
    }
  });
}


function usfmCaretChanged ()
{
  if (usfmCaretTimeout) {
    clearTimeout (usfmCaretTimeout);
  }
  usfmCaretTimeout = setTimeout (usfmHandleCaret, 1000);
  //restartCaretClarifier ();
}


function usfmHandleKeyDown (event)
{
  if (editKeysIgnoreForCaretChange (event)) return;
  usfmCaretChanged ();
}


function usfmHandleCaret ()
{
  // While the text has changed or is being saved,
  // do not run an operation to set the focused verse,
  // because the focused verse would be inaccurate due to the non-saved text.
  // This solved an inaccurate focused verse resulting from typing quickly at the end of the verse.
  if (usfmEditorTextChanged || usfmSaving) {
    usfmCaretChanged ();
    return;
  }
  if ($ ("#usfmeditor").is (":focus")) {
    var offset = usfmGetCaretPosition ();
    $.ajax ({
      url: "offset",
      type: "GET",
      data: { bible: usfmBible, book: usfmBook, chapter: usfmChapter, offset: offset },
      success: function (response) {
      },
      error: function (jqXHR, textStatus, errorThrown) {
        // On (network) failure, reschedule the update.
        usfmCaretChanged ();
      }
    });
  }
}


function usfmPositionCaretViaAjax ()
{
  // Due to, most likely, network latency,
  // setting the caret at times causes the caret to jump to undesired places.
  // Therefore the caret is to be set only on chapter load.
  if (usfmEditorWriteAccess) $ ("#usfmeditor").focus ();
  $.ajax ({
    url: "focus",
    type: "GET",
    data: { bible: usfmBible, book: usfmBook, chapter: usfmChapter },
    success: function (response) {
      response = response.split (" ");
      var start = parseInt (response [0], 10);
      var end = parseInt (response [1], 10);
      var offset = usfmGetCaretPosition ();
      if ((offset < start) || (offset > end)) {
        // Position caret right after the verse number.
        // That cares for empty verse text too.
        var position = start + 3 + usfmNavigationVerse.toString().length;
        usfmPositionCaret (position);
      }
      restartCaretClarifier ();
    }
  });
}


function usfmGetCaretPosition ()
{
  var position = undefined;
  var editor = $ ("#usfmeditor");
  if (editor.is (":focus")) {
    var element = editor.get (0);
    position = usfmGetCaretCharacterOffsetWithin (element);
  }
  return position;
}


function usfmGetCaretCharacterOffsetWithin (element)
{
  var elementText = "";
  if (typeof window.getSelection != "undefined") {
    var range = window.getSelection().getRangeAt(0);
    var preCaretRange = range.cloneRange();
    preCaretRange.selectNodeContents(element);
    preCaretRange.setEnd(range.endContainer, range.endOffset);
    elementText = preCaretRange.toString();
  } else if (typeof document.selection != "undefined" && document.selection.type != "Control") {
    var textRange = document.selection.createRange();
    var preCaretTextRange = document.body.createTextRange();
    preCaretTextRange.moveToElementText(element);
    preCaretTextRange.setEndPoint("EndToEnd", textRange);
    elementText = preCaretTextRange.text;
  }
  // Trim one space off at the right side.
  // See https://github.com/bibledit/cloud/issues/393.
  var caretOffset = elementText.length;
  var pos = elementText.length - 1;
  if (elementText != "") {
    if (elementText.charCodeAt(pos) == 32) {
      elementText = elementText.substr (0, pos);
    }
  }
  var caretOffset = elementText.length;
  return caretOffset;
}


function usfmPositionCaret (position)
{
  if (!usfmEditorWriteAccess) return;
  $ ("#usfmeditor").focus ();
  var currentPosition = usfmGetCaretPosition ();
  if (currentPosition == undefined) return;
  if (position == undefined) return;
  var selection = rangy.getSelection ();
  selection.move ("character", position - currentPosition);
}


function usfmWindowFocused ()
{
  // usfmPositionCaretViaAjax ();
}


function restartCaretClarifier ()
{
  if (usfmClarifierTimeout) {
    clearTimeout (usfmClarifierTimeout);
  }
  usfmClarifierTimeout = setTimeout (clarifyCaret, 100);
}


function getSelectionCoordinates()
{
  var x = 0, y = 0;
  var sel = document.selection, range;
  if (sel) {
    if (sel.type != "Control") {
      range = sel.createRange();
      range.collapse(true);
      x = range.boundingLeft;
      y = range.boundingTop;
    }
  } else if (window.getSelection) {
    sel = window.getSelection();
    if (sel.rangeCount) {
      range = sel.getRangeAt(0).cloneRange();
      if (range.getClientRects) {
        range.collapse(true);
        var rect = range.getClientRects()[0];
        if (rect) {
          x = rect.left;
          y = rect.top;
        }
      }
    }
  }
  return { x: x, y: y };
}


function clarifyCaret ()
{
  var scrolltop = $ ("#workspacewrapper").scrollTop ();
  var coordinates = getSelectionCoordinates ();
  var caretTop = coordinates.y + scrolltop;
  if (caretTop == usfmPreviousCaretTop) return;
  usfmPreviousCaretTop = caretTop;
  var viewportHeight = $(window).height ();
  $ ("#workspacewrapper").stop (true);
  $ ("#workspacewrapper").animate ({ scrollTop: caretTop - (viewportHeight * verticalCaretPosition / 100) }, 500);
}


/*
 
 Section for swipe navigation.

 */


function editusfmSwipeLeft (event)
{
  if (typeof navigateNextChapter != 'undefined') {
    navigateNextChapter (event);
  } else if (parent.window.navigateNextChapter != 'undefined') {
    parent.window.navigateNextChapter (event);
  }
}


function editusfmSwipeRight (event)
{
  if (typeof navigatePreviousChapter != 'undefined') {
    navigatePreviousChapter (event);
  } else if (parent.window.navigatePreviousChapter != 'undefined') {
    parent.window.navigatePreviousChapter (event);
  }
}

                                                            
/*

Section for positioning focused verse in non-editable USFM editor.

*/



function usfmPositionFocusedVerseViaAjax ()
{
  $.ajax ({
    url: "focus",
    type: "GET",
    data: { bible: usfmBible, book: usfmBook, chapter: usfmChapter },
    success: function (response) {
      response = response.split (" ");
      var start = parseInt (response [0], 10);
      var end = parseInt (response [1], 10);
      var offset = parseInt ((start + end) / 2);
      var cache = $ ("#usfmeditor").html();
      $ ("#usfmeditor").html(function (index, html) {
        return html.slice (0, offset) + '<span id="usfmfocus">' + html.slice(offset, offset + 1) + '</span>' + html.slice(offset + 1);
      });
      var verseTop = $("#usfmfocus").offset().top;
      $ ("#usfmeditor").html(cache);
      var workspaceHeight = $("#workspacewrapper").height();
      var viewportHeight = $(window).height ();
      var scrolltop = $ ("#workspacewrapper").scrollTop ();
      var scrollTo = verseTop - parseInt (workspaceHeight * verticalCaretPosition / 100) + scrolltop;
      $ ("#workspacewrapper").stop (true);
      $ ("#workspacewrapper").animate ({ scrollTop: scrollTo }, 500);
    }
  });
}

                             
/*

Section for reload notifications.

*/


function usfmEditorReloadAlert (message)
{
  // Take action only if the editor has focus and the user can type in it.
  var editor = $ ("#usfmeditor");
  if (editor.is (":focus")) {
    notifyItSuccess (message)
    editor.attr('contenteditable', false);
    setTimeout (usfmEditorReloadAlertTimeout, 3000);
  }
}


function usfmEditorReloadAlertTimeout ()
{
  var editor = $ ("#usfmeditor");
  editor.attr('contenteditable', usfmEditorWriteAccess);
  editor.focus ();
}

                             
// Note that some focus operations were removed for a situation where the workspace has two editors, and the user edits in the visual editor, and then the USFM editor grabs focus, and then the user keeps typing, so he or she was expecting to type in the visual editor but is now typing in the USFM editor.
// Due to removing the focus operations, the caret positioner no longer works when the USFM editor is not focused.
// A new USFM chapter editor could be having multiple editors: One for each line, or rather one for each verse. These editors could be loaded from the server separately, each time it loads a verse line. This system makes placing the caret and scrolling the caret into view easier. Because the javascript just places the caret in a known <div> and scrolls that <div> into view.
