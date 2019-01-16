/*
Copyright (©) 2003-2018 Teus Benschop.

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


var quill = undefined;
var Delta = Quill.import ("delta");


$ (document).ready (function ()
{
  // Reposition the editor's menu so it never scrolls out of view.
  var bar = $ ("#editorheader").remove ();
  $ ("#workspacemenu").append (bar);

  editorInitializeOnce ();
  editorInitialize ();

  navigationNewPassage ();
  
  $ (window).on ("unload", editorUnload);

  editorIdPollerTimeoutStart ();

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

  $ ("#editor").on ("click", editorNoteCitationClicked);
});


function editorInitializeOnce ()
{
  var Parchment = Quill.import ('parchment');
  
  // Register block formatting class.
  var ParagraphClass = new Parchment.Attributor.Class ('paragraph', 'b', { scope: Parchment.Scope.BLOCK });
  Quill.register (ParagraphClass, true);
  
  // Register inline formatting class.
  var CharacterClass = new Parchment.Attributor.Class ('character', 'i', { scope: Parchment.Scope.INLINE });
  Quill.register (CharacterClass, true);
}


function editorInitialize ()
{
  // Work around https://github.com/quilljs/quill/issues/1116
  // It sets the margins to 0 by adding an overriding class.
  // The Quill editor will remove that class again.
  $ ("#editor > p").each (function (index) {
    var element = $(this);
    element.addClass ("nomargins");
  });
  
  // Instantiate editor.
  quill = new Quill ('#editor', { });

  // Cause it to paste plain text only.
  quill.clipboard.addMatcher (Node.ELEMENT_NODE, function (node, delta) {
    var plaintext = $ (node).text ();
    return new Delta().insert (plaintext);
  });

  if (!quill.hasFocus ()) quill.focus ();
  
  // Event handlers.
  quill.on ("text-change", editorTextChangeHandler);
  quill.on ("selection-change", editorSelectionChangeHandler);
}


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
  editorHighlightVerseNumber ();
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
var editorWriteAccess = false;


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
      // Set the editor read-write or read-only.
      editorWriteAccess = checksum_readwrite (response);
      // Checksumming.
      response = checksum_receive (response);
      if (response !== false) {
        // Only load new text when it is different.
        if (response != editorGetHtml ()) {
          // Destroy existing editor.
          if (quill) delete quill;
          // Load the html in the DOM.
          $ ("#editor").empty ();
          $ ("#editor").append (response);
          // Create the editor based on the DOM's content.
          editorInitialize ();
          quill.enable (editorWriteAccess);
          // CSS for embedded styles.
          css4embeddedstyles ();
          // Feedback.
          editorStatus (editorChapterLoaded);
        }
        // Reference for comparison at save time.
        editorReferenceText = editorGetHtml ();
        // Position caret straightaway.
        if (reload) {
          positionCaret (editorCaretPosition);
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
  var html = $ (".ql-editor").html ();
  // Remove verse focus class name, if it is:
  // * the only class name.
  html = html.split (' class="versebeam"').join ('');
  // * between two more class names.
  html = html.split (' versebeam ').join ('');
  // * the first class name.
  html = html.split ('versebeam ').join ('');
  // * the last class name.
  html = html.split (' versebeam').join ('');
  // Remove class applied to caret.
  html = html.split ('<span class="ql-cursor">' + String.fromCharCode(65279) + '</span>').join ('');
  // Done.
  return html;
}


/*

Portion dealing with triggers for editor's content change.

*/


var editorContentChangedTimeoutId;


// Arguments: delta: Delta, oldContents: Delta, source: String
function editorTextChangeHandler (delta, oldContents, source)
{
  editorContentChanged ();
}


function editorContentChanged ()
{
  if (!editorWriteAccess) return;
  editorTextChanged = true;
  editorStatus (editorWillSave);
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
    url: "id",
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


// Responds to a changed selection or caret.
// range: { index: Number, length: Number }
// oldRange: { index: Number, length: Number }
// source: String
function editorSelectionChangeHandler (range, oldRange, source)
{
  editorHighlightVerseNumber ();
  
  // Bail out if editor not focused.
  if (!range) return;
  
  // Bail out if text was selected.
  if (range.length != 0) return;
  
  editorCaretMovedTimeoutStart ();
}


var editorCaretMovedTimeoutId;
var editorCaretMovedAjaxRequest;
var editorCaretMovedAjaxOffset;
var editorCaretInitialized = false;


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

  if (quill.hasFocus ()) {
    // Cancel any previous ajax request that might still be incompleted.
    // This is to avoid the caret jumping on slower or unstable connections.
    if (editorCaretMovedAjaxRequest && editorCaretMovedAjaxRequest.readystate != 4) {
      editorCaretMovedAjaxRequest.abort();
    }
    // Record the offset of the caret at the start of the ajax request.
    editorCaretMovedAjaxOffset = getCaretPosition ();
    // Initiate a new ajax request.
    editorCaretMovedAjaxRequest = $.ajax ({
      url: "navigate",
      type: "GET",
      data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter, offset: editorCaretMovedAjaxOffset },
      success: function (response) {
        if (response != "") {
          var offset = getCaretPosition ();
          // Take action only when the caret is still at the same position as it was when this ajax request was initiated.
          if (offset == editorCaretMovedAjaxOffset) {
            // Set the focused verse immediately,
            // rather than waiting on the Navigator signal that likely will come later.
            // This fixes a clicking / scrolling problem.
            editorNavigationVerse = response;
            editorScheduleWindowScrolling ();
            editorHighlightVerseNumber ();
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
  var caret = getCaretPosition ();
  if (caret == undefined) return;
  var format = quill.getFormat ();
  var paragraph = "";
  if (format.paragraph) paragraph = format.paragraph;
  var character = "";
  if (format.character) character = format.character;
  if (character.search ("note") >= 0) character = "";
  character = character.split ("0").join (" ");
  var styles = paragraph + " " + character;
  styles = styles.replace ("versebeam", "");
  var element = $ ("#activestyles");
  element.text (styles);
}


function editorSelectiveNotification (message)
{
  if (message == editorChapterLoaded) return;
  if (message == editorWillSave) return;
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
  var range = quill.getSelection();
  if (range) position = range.index;
  return position;
}


function positionCaret (position)
{
  if (position == undefined) return;
  var currentPosition = getCaretPosition ();
  if (currentPosition == undefined) return;
  position = parseInt (position);
  if (position == currentPosition) return;
  quill.setSelection (position, 0, "silent");
  editorActiveStylesFeedback ();
}


var editorPositionCaretViaAjaxTimerId;


function positionCaretViaAjax ()
{
  // Very frequent positioning calls have been seen in some browsers, so they are filtered here.
  if (editorPositionCaretViaAjaxTimerId) clearTimeout (editorPositionCaretViaAjaxTimerId);
  editorPositionCaretViaAjaxTimerId = setTimeout (positionCaretViaAjaxExecute, 100);
}


function positionCaretViaAjaxExecute ()
{
  if (isNoVerseBook (editorLoadedBook)) return;
  $.ajax ({
    url: "position",
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
    }
  });
}


/*
 
 Section for highlighting the focused verse.
 
*/


var editorHighlightVerseNumberTimeoutId;


function editorHighlightVerseNumber ()
{
  if (editorHighlightVerseNumberTimeoutId) clearTimeout (editorHighlightVerseNumberTimeoutId);
  editorHighlightVerseNumberTimeoutId = setTimeout (editorHighlightVerseNumberExecute, 200);
}


function editorHighlightVerseNumberExecute ()
{
  $ (".i-v").each (function (index) {
    var element = $(this);
    var verseNumber = element.text ();
    if (verseNumber == editorNavigationVerse) {
      element.addClass ("versebeam");
    } else {
      element.removeClass ("versebeam");
    }
  });
}


/*

Section for scrolling the caret into view.

*/


function editorScrollVerseIntoView ()
{
  if (isNoVerseBook (editorLoadedBook)) return;
  var position = getCaretPosition ();
  if (position == undefined) return;
  var bounds = quill.getBounds (position);
  var workspaceHeight = $("#workspacewrapper").height();
  var currentScrollTop = $("#workspacewrapper").scrollTop();
  var scrollTo = bounds.top - (workspaceHeight /  2);
  scrollTo = parseInt (scrollTo);
  var lowerBoundary = currentScrollTop - (workspaceHeight / 10);
  var upperBoundary = currentScrollTop + (workspaceHeight / 10);
  if ((scrollTo < lowerBoundary) || (scrollTo > upperBoundary)) {
    $("#workspacewrapper").animate({ scrollTop: scrollTo }, 500);
  }
}


/*

Section for the styles handling.

*/


function editorStylesButtonHandler ()
{
  if (!editorWriteAccess) return;
  $.ajax ({
    url: "styles",
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
    //$ ("#editor").focus ();
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
    //$ ("#editor").focus ();
    requestStyle (style);
  });
}


function requestStyle (style)
{
  $.ajax ({
    url: "styles",
    type: "GET",
    data: { style: style },
    cache: false,
    success: function (response) {
      response = response.split ("\n");
      var style = response [0];
      var action = response [1];
      if (action == "p") {
        applyParagraphStyle (style);
      } else if (action == 'c') {
        applyCharacterStyle (style);
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
    url: "styles",
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
  if (!quill.hasFocus ()) quill.focus ();
  quill.format ("paragraph", style, "user");
  editorActiveStylesFeedback ();
}


function applyCharacterStyle (style)
{
  if (!editorWriteAccess) return;
  if (!quill.hasFocus ()) quill.focus ();
  // No formatting of a verse.
  var range = quill.getSelection();
  if (range) {
    for (i = range.index; i < range.index + range.length; i++) {
      var format = quill.getFormat (i);
      if (format.character && format.character == "v") {
        return;
      }
    }
  }
  // Apply style.
  var format = quill.getFormat ();
  style = editor_determine_character_style (format.character, style)
  quill.format ("character", style, "user");
  // Check on embedded style applied.
  post_embedded_style_application (style);
  // Feedback.
  editorActiveStylesFeedback ();
}


function applyMonoStyle (style)
{
  if (!editorWriteAccess) return;

  quill.focus ();

  var caret = getCaretPosition ();
  
  var start = caret;
  var text = quill.getText (start, 1);
  if (text == "\n") caret--;
  for (i = caret; i >= 0; i--) {
    var text = quill.getText (i, 1);
    if (text == "\n") break;
    start = i;
  }

  var end = caret;
  for (i = caret; true; i++) {
    end = i;
    var text = quill.getText (i, 1);
    if (text == "\n") break;
  }

  text = quill.getText (start, end - start);
  var char = text.substring (0, 1);
  if (char == "\\") {
    text = text.substring (1);
    var pos = text.indexOf (' ');
    text = text.substring (pos + 1);
  }
  text = "\\" + style + " " + text;

  quill.deleteText (start, end - start);
  quill.insertText (start, text, "paragraph", "mono");
  
  editorActiveStylesFeedback ();
}


/*

Section for the notes.

*/


var editorInsertedNotesCount = 0;


function applyNotesStyle (style)
{
  if (!editorWriteAccess) return;

  quill.focus ();

  // Check for and optionally append the gap between text body and notes.
  var notes = $ (".b-notes");
  if (notes.length == 0) {
    var length = quill.getLength ();
    quill.insertText (length, "\n", "paragraph", "notes", "user")
  }
  
  // Get a new note identifier.
  var date = new Date();
  var noteId = String (date.getTime());

  var caller = String ((editorInsertedNotesCount % 9) + 1);
  
  // Insert note caller at caret.
  var caret = getCaretPosition ();
  quill.setSelection (caret, 0);
  quill.insertText (caret, caller, "character", "notecall" + noteId, "user");

  // Append note text to notes section:
  // <p class="b-f"><span class="i-notebody1">1</span> + .</p>
  var length = quill.getLength ();
  quill.insertText (length, "\n", "paragraph", style, "user");
  quill.insertText (length, caller, "character", "notebody" + noteId, "user");
  length++;
  quill.insertText (length, " + .", "character", "", "user");
  
  editorInsertedNotesCount++;
  
  editorActiveStylesFeedback ();
}


function editorNoteCitationClicked (event)
{
  var target = $(event.target);
  var cls = target.attr ("class");
  if (cls.substr (0, 6) != "i-note") return;
  cls = cls.substr (2);
  var length = quill.getLength ();
  if (cls.search ("call") > 0) {
    cls = cls.replace ("call", "body");
    // Start searching for note bodies at the end.
    for (i = length; i >= 0; i--) {
      var format = quill.getFormat (i, 1);
      if (format.character && format.character == cls) {
        quill.setSelection (i + 4, 0);
        editorScrollVerseIntoView ();
        editorActiveStylesFeedback ();
        return;
      }
    }
  }
  if (cls.search ("body") > 0) {
    cls = cls.replace ("body", "call");
    // Start searching for note callers at the start.
    for (i = 0; i < length; i++) {
      var format = quill.getFormat (i, 1);
      if (format.character && format.character == cls) {
        quill.setSelection (i + 1, 0);
        editorScrollVerseIntoView ();
        editorActiveStylesFeedback ();
        return;
      }
    }
  }
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



// Debug



function editorLog (msg)
{
  var date = new Date();
  var seconds = date.getSeconds();
  var milliseconds = date.getMilliseconds();
  console.log (seconds + " " + milliseconds + ": " + msg);
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


