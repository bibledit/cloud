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


$ (document).ready (function () 
{
  // Make the editor's menu to never scroll out of view.
  var bar = $ ("#editorheader").remove ();
  $ ("#workspacemenu").append (bar);
  
  rangy.init ();

  navigationNewPassage ();

  $ ("#editor").on ("paste cut keydown", editorContentChanged);
  $ (window).on ("unload", editorUnload);

  $ ("#editor").on ("focus", editorWindowFocused);

  $ ("#editor").on ("paste", editorPaste);

  editorIdPollerTimeoutStart ();

  $ ("#editor").on ("click", editorCaretChangedByMouse);
  $ ("#editor").on ("keydown", editorCaretChangedByKeyboard);

  $ ("#editor").focus ();

  editorBindUnselectable ();
  $ ("#stylebutton").on ("click", editorStylesButtonHandler);
  $ (window).on ("keydown", editorWindowKeyHandler);

  positionCaretViaAjax ();
  
  if (swipe_operations) {
    $ ("body").swipe ( {
      swipeLeft:function (event, direction, distance, duration, fingerCount, fingerData) {
        editorSwipeLeft (event);
      },
      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
        editorSwipeRight (event);
      }
    });
  }
});


/*

Section for the new Passage event from the Navigator.

*/


var editorNavigationBook;
var editorNavigationChapter;
var editorNavigationVerse;


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
    editorNavigationBook = navigationBook;
    editorNavigationChapter = navigationChapter;
    editorNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    editorNavigationBook = parent.window.navigationBook;
    editorNavigationChapter = parent.window.navigationChapter;
    editorNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }

  if ((editorNavigationBook != editorLoadedBook) || (editorNavigationChapter != editorLoadedChapter)) {
    editorSaveChapter ();
    editorLoadChapter (false);
  } else {
    editorScheduleCaretPositioning ();
  }
}


/*

Section for editor load and save.

Notes:
* It remembers the Bible, book, and chapter loaded.
  The reason for remembering these is, among others, that the active Bible / book / chapter
  on the server may change due to user actions, so when saving this chapter to the server,
  it passes the correct Bible / book / chapter to the server along with the updated text.
* While loading the chapter, do not set the "contenteditable" to false, then to true,
  because Google Chrome gets confused then. This was seen on version 33.
  Other types of browsers and other versions were not tested on this behaviour.

*/


var editorLoadedBible;
var editorLoadedBook;
var editorLoadedChapter;
var editorReferenceText;
var editorTextChanged = false;
var editorCaretPosition = 0;
var editorSaveAsync;
var editorSaving = false;


function editorLoadChapter (reload)
{
  editorLoadedBible = navigationBible;
  editorLoadedBook = editorNavigationBook;
  editorLoadedChapter = editorNavigationChapter;
  editorChapterIdOnServer = 0;
  editorCaretPosition = getCaretPosition ();
  editorCaretInitialized = false;
  $.ajax ({
    url: "load",
    type: "GET",
    data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter },
    success: function (response) {
      editorWriteAccess = checksum_readwrite (response);
      var contenteditable = ($ ("#editor").attr('contenteditable') === 'true');
      if (editorWriteAccess != contenteditable) $ ("#editor").attr('contenteditable', editorWriteAccess);
      // Checksumming.
      response = checksum_receive (response);
      if (response !== false) {
        // Only load new text when it is different.
        if (response != editorGetHtml ()) {
          $ ("#editor").empty ();
          $ ("#editor").append (response);
          editorStatus (editorChapterLoaded);
        }
        editorReferenceText = response;
        if (reload) {
          positionCaret (editorCaretPosition);
        } else {
        }
        editorScheduleCaretPositioning ();
      } else {
        // Checksum error: Reload.
        editorLoadChapter (false);
      }
      editorCaretInitialized = false;
    },
  });
}


function editorSaveChapter (sync)
{
  if (editorSaving) {
    editorContentChangedTimeoutStart ();
    return;
  }
  if (!editorWriteAccess) return;
  editorTextChanged = false;
  if (!editorLoadedBible) return;
  if (!editorLoadedBook) return;
  var html = editorGetHtml ();
  if (html == editorReferenceText) return;
  editorStatus (editorChapterSaving);
  editorReferenceText = html;
  editorChapterIdOnServer = 0;
  editorIdPollerTimeoutStop ();
  editorSaveAsync = true;
  if (sync) editorSaveAsync = false;
  var encodedHtml = filter_url_plus_to_tag (html);
  var checksum = checksum_get (encodedHtml);
  editorSaving = true;
  $.ajax ({
    url: "save",
    type: "POST",
    async: editorSaveAsync,
    data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter, html: encodedHtml, checksum: checksum },
    success: function (response) {
      editorStatus (response);
      if (response == editorChapterReformat) {
        editorLoadChapter (true);
      }
    },
    error: function (jqXHR, textStatus, errorThrown) {
      editorStatus (editorChapterRetrying);
      editorReferenceText = "";
      editorContentChanged ();
      if (!editorSaveAsync) editorSaveChapter (true);
    },
    complete: function (xhr, status) {
      editorIdPollerTimeoutStart ();
      editorSaveAsync = true;
      editorSaving = false;
    },
  });
}


function editorGetHtml ()
{
  var html = $ ("#editor").html ();
  // Remove verse focus class name, if it is:
  // * the only class name.
  html = html.split (' class="focusedverse"').join ('');
  // * between two more class names.
  html = html.split (' focusedverse ').join ('');
  // * the first class name.
  html = html.split ('focusedverse ').join ('');
  // * the last class name.
  html = html.split (' focusedverse').join ('');
  return html;
}


/*

Portion dealing with triggers for editor's content change.

*/


var editorContentChangedTimeoutId;


function editorContentChanged (event)
{
  if (!editorWriteAccess) return;
  if (editKeysIgnoreForContentChange (event)) return;
  editorTextChanged = true;
  editorContentChangedTimeoutStart ();
}


function editorContentChangedTimeoutStart ()
{
  if (editorContentChangedTimeoutId) clearTimeout (editorContentChangedTimeoutId);
  editorContentChangedTimeoutId = setTimeout (editorSaveChapter, 1000);
}


function editorUnload ()
{
  editorSaveChapter (true);
}


/*

Section for polling the server for updates on the loaded chapter.

*/


var editorChapterIdOnServer = 0;
var editorChapterIdPollerTimeoutId;


function editorIdPollerTimeoutStart ()
{
  editorIdPollerTimeoutStop ();
  editorChapterIdPollerTimeoutId = setTimeout (editorEditorPollId, 1000);
}


function editorIdPollerTimeoutStop ()
{
  if (editorChapterIdPollerTimeoutId) clearTimeout (editorChapterIdPollerTimeoutId);
}


function editorEditorPollId ()
{
  $.ajax ({
    url: "../edit/id",
    type: "GET",
    data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter },
    success: function (response) {
      if (!editorSaving) {
        if (editorChapterIdOnServer != 0) {
          if (response != editorChapterIdOnServer) {
            editorLoadChapter (true);
            editorChapterIdOnServer = 0;
          }
        }
        editorChapterIdOnServer = response;
      }
    },
    complete: function (xhr, status) {
      editorIdPollerTimeoutStart ();
    }
  });
}


/*

Section responding to a moved caret.

*/


var editorCaretMovedTimeoutId;
var editorCaretMovedAjaxRequest;
var editorCaretMovedAjaxOffset;
var editorCaretInitialized = false;


function editorCaretChangedByMouse (event)
{
  if (editorPositionCaretProgrammatically) return;
  editorCaretMovedTimeoutStart ();
}


function editorCaretChangedByKeyboard (event)
{
  if (editKeysIgnoreForCaretChange (event)) return;
  if (editorPositionCaretProgrammatically) return;
  editorCaretMovedTimeoutStart ();
}


function editorCaretMovedTimeoutStart ()
{
  if (editorCaretMovedTimeoutId) clearTimeout (editorCaretMovedTimeoutId);
  editorCaretMovedTimeoutId = setTimeout (editorHandleCaretMoved, 200);
}


function editorHandleCaretMoved ()
{
  // If the text in the editor has been changed, and therefore not saved,
  // or is being saved now, postpone handling the moved caret.
  // It is important to also delay the handler "while" the text is being saved,
  // because there have been situations that javascript initiated the save operation,
  // shortly after that the moved caret handler via AJAX,
  // and that the server completed processing the caret handler before it had completed the save operations,
  // which led to inaccurate caret position calculations, which caused symptoms like a "jumping caret".
  if (editorTextChanged || editorSaving) {
    editorCaretMovedTimeoutStart ();
    return;
  }
  
  // If the caret has not yet been positioned, postpone the action.
  if (!editorCaretInitialized) {
    editorCaretMovedTimeoutStart ();
    positionCaretViaAjax ();
    return;
  }

  if ($ ("#editor").is (":focus")) {
    // Cancel any previous ajax request that might still be incompleted.
    // This is to avoid the caret jumping on slower or unstable connections.
    if (editorCaretMovedAjaxRequest && editorCaretMovedAjaxRequest.readystate != 4) {
      editorCaretMovedAjaxRequest.abort();
    }
    // Record the offset of the caret at the start of the ajax request.
    editorCaretMovedAjaxOffset = getCaretPosition ();
    // Initiate a new ajax request.
    editorCaretMovedAjaxRequest = $.ajax ({
      url: "offset",
      type: "GET",
      data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter, offset: editorCaretMovedAjaxOffset },
      success: function (response) {
        if (response != "") {
          var offset = getCaretPosition ();
          // Take action only when the caret is still at the same position as it was when this ajax request was initiated.
          if (offset == editorCaretMovedAjaxOffset) {
            // Set the verse correct immediately, rather than waiting on the Navigator signal that likely will come later.
            // This fixes a clicking / scrolling problem.
            editorNavigationVerse = response;
            editorScheduleWindowScrolling ();
          } else {
            // Caret was moved during this AJAX operation: Reschedule it.
            editorCaretMovedTimeoutStart ();
          }
        }
      },
      error: function (jqXHR, textStatus, errorThrown) {
        // On (network) failure, reschedule the update.
        editorCaretMovedTimeoutStart ();
      }
    });
  } else {
    editorCaretMovedTimeoutStart ();
  }

  editorActiveStylesFeedback ();
}


/*

Section with window events and their basic handlers.

*/


function editorWindowKeyHandler (event)
{
  if (!editorWriteAccess) return;
  // Ctrl-S: Style.
  if ((event.ctrlKey == true) && (event.keyCode == 83)) {
    editorStylesButtonHandler ();
    return false;
  }
  // Escape.
  if (event.keyCode == 27) {
    editorClearStyles ();
  }
}


function editorWindowFocused ()
{
  editorCaretMovedTimeoutStart ();
}


/*

Section for user interface updates and feedback.

*/


function editorStatus (text)
{
  $ ("#editorstatus").empty ();
  $ ("#editorstatus").append (text);
  editorSelectiveNotification (text);
}


function editorActiveStylesFeedback ()
{
  var editor = $ ("#editor");
  if (editor.is (":focus")) {
    var parent = rangy.getSelection().anchorNode.parentElement;
    parent = $ (parent);
    var paragraph = parent.closest ("p");
    var pname = paragraph.attr ('class');
    var span = parent.closest ("span");
    var cname = span.attr ("class");
    if (cname == undefined) cname = "";
    cname = cname.replace ("focusedverse", "");
    var element = $ ("#activestyles");
    element.text (pname + " " + cname);
  }
}


function editorSelectiveNotification (message)
{
  if (message == editorChapterLoaded) return;
  if (message == editorChapterSaving) return;
  if (message == editorChapterSaved) return;
  if (message == editorChapterReformat) return;
  notifyItError (message);
}


/*

Section for getting and setting the caret position.

*/


function getCaretPosition ()
{
  var position = undefined;
  var editor = $ ("#editor");
  if (editor.is (":focus")) {
    var element = editor.get (0);
    position = getCaretCharacterOffsetWithin (element);
  }
  return position;
}


function getCaretCharacterOffsetWithin (element)
{
  var caretOffset = 0;
  if (typeof window.getSelection != "undefined") {
    var range = window.getSelection().getRangeAt(0);
    var preCaretRange = range.cloneRange();
    preCaretRange.selectNodeContents(element);
    preCaretRange.setEnd(range.endContainer, range.endOffset);
    caretOffset = preCaretRange.toString().length;
  } else if (typeof document.selection != "undefined" && document.selection.type != "Control") {
    var textRange = document.selection.createRange();
    var preCaretTextRange = document.body.createTextRange();
    preCaretTextRange.moveToElementText(element);
    preCaretTextRange.setEndPoint("EndToEnd", textRange);
    caretOffset = preCaretTextRange.text.length;
  }
  return caretOffset;
}


function positionCaret (position)
{
  if (position == undefined) return;
  editorPositionCaretCount = 3;
  editorPositionCaretOffset = position;
  editorPositionCaretExecute ();
}

var editorPositionCaretCount = 0;
var editorPositionCaretOffset = 0;
var editorPositionCaretProgrammatically = false;

function editorPositionCaretExecute ()
{
  // Positioning often is not accurate the first time, so do it several times if needed.
  if (editorPositionCaretCount > 0) {
    var currentPosition = getCaretPosition ();
    if (currentPosition == undefined) return;
    editorPositionCaretProgrammatically = true;
    var selection = rangy.getSelection ();
    var desiredPosition = parseInt (editorPositionCaretOffset);
    // Fix bug that it jumps to the previous verse through positioning the cursor slightly off-location.
    if (editorPositionCaretCount == 3) desiredPosition += 5;
    selection.move ("character", desiredPosition - currentPosition);
    currentPosition = getCaretPosition ();
    editorPositionCaretProgrammatically = false;
    if (editorPositionCaretOffset == currentPosition) return;
    editorPositionCaretCount--;
    setTimeout (editorPositionCaretExecute, 10);
  }
}


var editorPositionCaretViaAjaxTimerId;


function positionCaretViaAjax ()
{
  // Very frequent focus calls have been seen in some browsers, so they are filtered here.
  if (editorPositionCaretViaAjaxTimerId) clearTimeout (editorPositionCaretViaAjaxTimerId);
  editorPositionCaretViaAjaxTimerId = setTimeout (positionCaretViaAjaxExecute, 100);
}


function positionCaretViaAjaxExecute ()
{
  if (isNoVerseBook (editorLoadedBook)) return;
  $.ajax ({
    url: "focus",
    type: "GET",
    data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter },
    success: function (response) {
      if (response != "") {
        response = response.split ("\n");
        var start = response [0];
        var end = response [1];
        var offset = getCaretPosition ();
        if ((offset < start) || (offset > end)) {
          positionCaret (start);
        }
        editorCaretInitialized = true;
      }
      editorScheduleWindowScrolling ();
    },
    error: function (jqXHR, textStatus, errorThrown) {
      // Network error: Reschedule.
      editorScheduleCaretPositioning ();
    },
    complete: function (xhr, status) {
    }
  });
}


/*

Section for scrolling the caret into view.

*/


function editorScrollVerseIntoView ()
{
  if (isNoVerseBook (editorLoadedBook)) return;

  var verses = [0];
  var navigated = false;
  $ ("#editor > p span").each (function (index) {
    var element = $(this);
    if (element.hasClass ("v")) {
      verses = usfm_get_verse_numbers (element[0].textContent);
      if (verses.indexOf (parseInt (editorNavigationVerse)) >= 0) {
        if (navigated == false) {
          var verseTop = element.offset().top;
          var workspaceHeight = $("#workspacewrapper").height();
          var currentScrollTop = $("#workspacewrapper").scrollTop();
          var scrollTo = verseTop - (workspaceHeight * verticalCaretPosition / 100) + currentScrollTop;
          var lowerBoundary = currentScrollTop - (workspaceHeight / 10);
          var upperBoundary = currentScrollTop + (workspaceHeight / 10);
          if ((scrollTo < lowerBoundary) || (scrollTo > upperBoundary)) {
            $("#workspacewrapper").animate({ scrollTop: scrollTo }, 500);
          }
          navigated = true;
        }
      }
    }
    if (verses.indexOf (parseInt (editorNavigationVerse)) >= 0) element.addClass ("focusedverse");
    else element.removeClass ("focusedverse");

  });
  if (editorNavigationVerse == 0) {
    $ ("#workspacewrapper").animate ({ scrollTop: 0 }, 500);
  }
}


/*

Section for the styles handling.

*/


function editorStylesButtonHandler ()
{
  if (!editorWriteAccess) return;
  $.ajax ({
    url: "../edit/styles",
    type: "GET",
    cache: false,
    success: function (response) {
      editorShowResponse (response);
      editorBindUnselectable ();
      dynamicClickHandlers ();
    },
  });
  return false;
}


function editorBindUnselectable ()
{
  var elements = $ (".unselectable");
  elements.off ("mousedown");
  elements.on ("mousedown", function (event) {
    event.preventDefault();
  });
}


function editorShowResponse (response)
{
  if (!editorWriteAccess) return;
  $ ("#stylebutton").hide ();
  $ ("#nostyles").hide ();
  var area = $ ("#stylesarea");
  area.empty ();
  area.append (response);
}


function editorClearStyles ()
{
  var area = $ ("#stylesarea");
  area.empty ();
  $ ("#stylebutton").show ();
  $ ("#nostyles").show ();
}


function dynamicClickHandlers ()
{
  var elements = $ ("#stylesarea > a");
  elements.on ("click", function (event) {
    event.preventDefault();
    editorClearStyles ();
    $ ("#editor").focus ();
    var href = event.currentTarget.href;
    href = href.substring (href.lastIndexOf ('/') + 1);
    if (href == "cancel") return;
    if (href == "all") {
      displayAllStyles ();
    } else {
      requestStyle (href);
    }
  });

  $ ("#styleslist").on ("change", function (event) {
    var selection = $ ("#styleslist option:selected").text ();
    var style = selection.substring (0, selection.indexOf (" "));
    event.preventDefault();
    editorClearStyles ();
    $ ("#editor").focus ();
    requestStyle (style);
  });
}


function requestStyle (style)
{
  $.ajax ({
    url: "../edit/styles",
    type: "GET",
    data: { style: style },
    cache: false,
    success: function (response) {
      response = response.split ("\n");
      var style = response [0];
      var action = response [1];
      if (action == "p") {
        applyParagraphStyle (style);
        editorContentChanged ();
      } else if (action == 'c') {
        applyCharacterStyle (style);
        editorContentChanged ();
      } else if (action == 'n') {
        applyNotesStyle (style);
        editorContentChanged ();
      } else if (action == "m") {
        applyMonoStyle (style);
        editorContentChanged ();
      }
    },
  });
}


function displayAllStyles ()
{
  $.ajax ({
    url: "../edit/styles",
    type: "GET",
    data: { all: "" },
    cache: false,
    success: function (response) {
      editorShowResponse (response);
      editorBindUnselectable ();
      dynamicClickHandlers ();
    },
  });
}


function applyParagraphStyle (style)
{
  if (!editorWriteAccess) return;
  $ ("#editor").focus ();
  var parent = rangy.getSelection().anchorNode.parentElement;
  parent = $ (parent);
  var paragraph = parent.closest ("p");
  paragraph.removeClass ();
  paragraph.addClass (style);
}


function applyCharacterStyle (style)
{
  if (!editorWriteAccess) return;
  $ ("#editor").focus ();
  var classApplier = rangy.createClassApplier (style);
  classApplier.toggleSelection ();
}


function applyMonoStyle (style)
{
  if (!editorWriteAccess) return;

  $ ("#editor").focus ();

  var parent = rangy.getSelection().anchorNode.parentElement;
  parent = $ (parent);
  var paragraph = parent.closest ("p");

  paragraph.removeClass ();
  paragraph.addClass ("mono");

  var text = paragraph.text ();

  var char = text.substring (0, 1);
  if (char == "\\") {
    text = text.substring (1, 10000);
    var pos = text.indexOf (' ');
    text = text.substring (pos + 1, 10000);
  }
  text = "\\" + style + " " + text;
  paragraph.text (text);
}


/*

Section for the notes.

*/


var editorInsertedNotesCount = 0;


function applyNotesStyle (style)
{
  if (!editorWriteAccess) return;

  $ ("#editor").focus ();

  // Check for / append notes section.
  var notes = $ ("#notes");
  if (notes.length == 0) {
    $ ("#editor").append ('<div id="notes"><hr></div>');
  }

  // Get a new node identifier based on the local time.
  var date = new Date();
  var noteId = date.getTime();

  var caller = (editorInsertedNotesCount % 9) + 1;
  
  // Insert note caller at caret.
  var html = '<a href="#note' + noteId + '" id="citation' + noteId + '" class="superscript">' + caller + '</a>';
  insertHtmlAtCaret (html);
  
  // Append note text to notes section.
  html = '<p class="' + style + '"><a href="#citation' + noteId + '" id="note' + noteId + '">' + caller + '</a><span> </span><span>+ </span><span> </span></p>';
  $ ("#notes").append (html);
  
  editorInsertedNotesCount++;
}


function insertHtmlAtCaret (html) 
{
  var sel, range;
  if (window.getSelection) {
    // IE9 and non-IE
    sel = window.getSelection();
    if (sel.getRangeAt && sel.rangeCount) {
      range = sel.getRangeAt(0);
      range.deleteContents();

      // Range.createContextualFragment() would be useful here but is
      // only relatively recently standardized and is not supported in
      // some browsers (IE9, for one)
      var el = document.createElement("div");
      el.innerHTML = html;
      var frag = document.createDocumentFragment(), node, lastNode;
      while ( (node = el.firstChild) ) {
        lastNode = frag.appendChild(node);
      }
      var firstNode = frag.firstChild;
      range.insertNode(frag);
      
      // Preserve the selection
      if (lastNode) {
        range = range.cloneRange();
        range.setStartAfter(lastNode);
        range.collapse(true);
        sel.removeAllRanges();
        sel.addRange(range);
      }
    }
  } else if ((sel = document.selection) && sel.type != "Control") {
    // IE < 9
    var originalRange = sel.createRange();
    originalRange.collapse(true);
    sel.createRange().pasteHTML(html);
  }
}


/*

Section for the clipboard.

*/


function editorPaste (event)
{
  var data = event.originalEvent.clipboardData.getData ('Text');
  event.preventDefault();
  document.execCommand ("insertHTML", false, data);
}


/*

Caret positioning and subsequent window scrolling.

The purpose of this section is to coordinate the two events,
so that caret positioning is done first, 
and window scrolling last.

*/


var editorPositioningTimerId;
var editorPendingCaretPositioning = false;
var editorPendingWindowScrolling = false;


function editorScheduleCaretPositioning ()
{
  editorPendingCaretPositioning = true;
  editorPendingWindowScrolling = false;
  editorPositioningTimerStart ();
}


function editorScheduleWindowScrolling ()
{
  editorPendingWindowScrolling = true;
  editorPositioningTimerStart ();
}


function editorPositioningTimerStart ()
{
  if (editorPositioningTimerId) clearTimeout (editorPositioningTimerId);
  editorPositioningTimerId = setTimeout (editorPositioningRun, 200);
}


function editorPositioningRun ()
{
  if (editorPendingCaretPositioning) {
    positionCaretViaAjax ();
    editorPendingCaretPositioning = false;
    editorPendingWindowScrolling = false;
  }
  if (editorPendingWindowScrolling) {
    editorPendingWindowScrolling = false;
    editorScrollVerseIntoView ();
  }
}


/*
 
 Section for swipe navigation.
 
 */


function editorSwipeLeft (event)
{
  if (typeof navigateNextChapter != 'undefined') {
    navigateNextChapter (event);
  } else if (parent.window.navigateNextChapter != 'undefined') {
    parent.window.navigateNextChapter (event);
  }
}


function editorSwipeRight (event)
{
  if (typeof navigatePreviousChapter != 'undefined') {
    navigatePreviousChapter (event);
  } else if (parent.window.navigatePreviousChapter != 'undefined') {
    parent.window.navigatePreviousChapter (event);
  }
}
