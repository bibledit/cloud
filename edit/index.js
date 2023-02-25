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


var quill = undefined;
var Delta = Quill.import ("delta");
var chapterEditorUniqueID = Math.floor (Math.random() * 100000000);


$ (document).ready (function ()
{
  // Listens for bibleselect option tags value change to update the loaded Bible.
  var bibleSelectionElement = document.querySelector ("#bibleselect");
  bibleSelectionElement.addEventListener ('change', () => {
    var value = bibleSelectionElement.value;
    $.post ("index", { bibleselect: bibleSelectionElement.value })
      .done (function() { window.location.reload () });
  });

  // Reposition the editor's menu so it never scrolls out of view.
  var bar = $ ("#editorheader").remove ();
  $ ("#workspacemenu").append (bar);

  editorInitializeOnce ();
  editorInitialize ();

  navigationNewPassage ();
  
  $ (window).on ("unload", edit2SaveChapter);

  editorBindUnselectable ();
  $ ("#stylebutton").on ("click", editorStylesButtonHandler);
  $ (window).on ("keydown", editorWindowKeyHandler);
  
  edit2PositionCaretViaAjaxStart ();

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
  
  $ ("#editor").bind ("paste", editorClipboardPasteHandler);

  setTimeout (edit2CoordinatingTimeout, 500);

  document.body.addEventListener('paste', ChapterEditorHandlePaste);

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

  if (editorWriteAccess) if (!quill.hasFocus ()) quill.focus ();
  
  // Event handlers.
  quill.on ("text-change", editorTextChangeHandler);
  quill.on ("selection-change", editorSelectionChangeHandler);
  
  // Clear undo/redo stack.
  quill.history.clear();
}


// This fixes an issue where upon pasting text in the editor,
// the editor scrolls to the very top of the text.
// If the text is long, then the focused verse is thrown off the screen.
// Here it fixes that.
// https://github.com/bibledit/cloud/issues/428
function editorClipboardPasteHandler (event)
{
  var currentScrollTop = $("#workspacewrapper").scrollTop();
  $("#workspacewrapper").animate({ scrollTop: currentScrollTop }, 100);
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
    edit2SaveChapter ();
    editorLoadChapter ();
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
var edit2CaretPosition = 0;
var editorSaving = false;
var editorWriteAccess = false;


function editorLoadChapter ()
{
  editorLoadedBible = navigationBible;
  editorLoadedBook = editorNavigationBook;
  editorLoadedChapter = editorNavigationChapter;
  editorChapterIdOnServer = 0;
  edit2CaretPosition = getCaretPosition ();
  edit2CaretInitialized = false;
  $.ajax ({
    url: "load",
    type: "GET",
    data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter, id: chapterEditorUniqueID },
    success: function (response) {
      // Set the editor read-write or read-only.
      editorWriteAccess = checksum_readwrite (response);
      // If this is the second or third or higher editor in the workspace,
      // make the editor read-only.
      if (window.frameElement) {
        iframe = $(window.frameElement);
        var data_editor_number = iframe.attr("data-editor-no");
        if (data_editor_number > 1) {
          editorWriteAccess = false;
        }
      }
      // Checksumming.
      response = checksum_receive (response);
      if (response !== false) {
        // Only load new text when it is different.
        // Extract the plain text from the html and compare that.
        // https://github.com/bibledit/cloud/issues/449
        var responseText = $(response).text();
        var editorText = $(editorGetHtml ()).text();
        if (responseText != editorText) {
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
        // Reference for comparison at save or update time.
        editorReferenceText = editorGetHtml ();
        // Position caret.
        editorScheduleCaretPositioning ();
      } else {
        // Checksum error: Reload.
        editorLoadChapter ();
      }
      edit2CaretInitialized = false;
    },
  });
}


function edit2SaveChapter ()
{
  editorStatus ("");
  if (editorSaving) {
    edit2ContentChanged ();
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
  var encodedHtml = filter_url_plus_to_tag (html);
  var checksum = checksum_get (encodedHtml);
  editorSaving = true;
  $.ajax ({
    url: "save",
    type: "POST",
    async: false,
    data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter, html: encodedHtml, checksum: checksum, id: chapterEditorUniqueID },
    success: function (response) {
      editorStatus (response);
    },
    error: function (jqXHR, textStatus, errorThrown) {
      editorStatus (editorChapterRetrying);
      editorReferenceText = "";
      edit2ContentChanged ();
      edit2SaveChapter ();
    },
    complete: function (xhr, status) {
      editorSaving = false;
    },
  });
}


function editorGetHtml ()
{
  var html = $ ("#editor > .ql-editor").html ();
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

Portion dealing with triggers for when the editor's content changes.

*/


// Three combined lists store information about edits made in the editor,
// during the time span between
// 1. the moment the changes are sent to the server/device,
// when there is a delay in the network, and
// 2. the moment the updates for the editor come back from the server/device.
// Storage as follows:
// 1. Offsets at which...
// 2. The given number of characters were inserted, and...
// 3. The given number of characters were deleted.
// With 2-byte UTF-16 characters, one character is given as a "1" in the lists.
// With 4-byte UTF-16 characters, one characters is represented by a "2" in the lists.
var edit2EditorChangeOffsets = [];
var edit2EditorChangeInserts = [];
var edit2EditorChangeDeletes = [];


// Arguments: delta: Delta, oldContents: Delta, source: String
function editorTextChangeHandler (delta, oldContents, source)
{
  // Record the change.
  // It gives 4-byte UTF-16 characters as length value 2 instead of 1.
  var retain = 0;
  var insert = 0;
  var del = 0;
  for (let i = 0; i < delta.ops.length; i++) {
    let obj = delta.ops[i];
    if (obj.retain) retain = obj.retain;
    // For Unicode handling, see:
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/length
    if (obj.insert) insert = obj.insert.length;
    if (obj.delete) del = obj.delete;
  }
  edit2EditorChangeOffsets.push(retain);
  edit2EditorChangeInserts.push(insert);
  edit2EditorChangeDeletes.push(del);
  // Ensure that it does not delete a chapter number or verse number.
  if (!delta.ops [0].retain) {
    quill.history.undo ();
  }
  // Start save delay timer.
  edit2ContentChanged ();
  // Set a flag for the undo workaround.
  edit2UndoChanged = true;
}


var edit2ContentChangedTimeoutId;


function edit2ContentChanged ()
{
  if (!editorWriteAccess) return;
  editorTextChanged = true;
  editorStatus (editorWillSave);
  if (edit2ContentChangedTimeoutId) {
    clearTimeout (edit2ContentChangedTimeoutId);
  }
  edit2ContentChangedTimeoutId = setTimeout (edit2EditorTriggerSave, 1000);
}


function edit2EditorTriggerSave ()
{
  if (!edit2UpdateTrigger) {
    edit2UpdateTrigger = true;
  } else {
    if (edit2ContentChangedTimeoutId) {
      clearTimeout (edit2ContentChangedTimeoutId);
    }
    edit2ContentChangedTimeoutId = setTimeout (edit2EditorTriggerSave, 400);
  }
}


/*

Section for polling the server for updates on the loaded chapter.

*/


var editorChapterIdOnServer = 0;


function edit2EditorPollId ()
{
  edit2AjaxActive = true;
  $.ajax ({
    url: "../editor/id",
    type: "GET",
    data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter },
    success: function (response) {
      if (editorChapterIdOnServer != 0) {
        if (response != editorChapterIdOnServer) {
          // The chapter identifier changed.
          // That means that likely there's updated text on the server.
          // Start the routine to load any possible updates into the editor.
          edit2UpdateTrigger = true;
        }
      }
      editorChapterIdOnServer = response;
    },
    complete: function (xhr, status) {
      edit2AjaxActive = false;
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
  
  edit2CaretMovedTimeoutStart ();
}


var edit2CaretInitialized = false;
var edit2CaretMovedTimeoutId;


function edit2CaretMovedTimeoutStart ()
{
  if (edit2CaretMovedTimeoutId) clearTimeout (edit2CaretMovedTimeoutId);
  edit2CaretMovedTimeoutId = setTimeout (edit2CaretMovedTimeoutFinish, 200);
}


function edit2CaretMovedTimeoutFinish ()
{
  // Since the user moved the caret by hand,
  // cancel any scheduled caret positioning.
  edit2PositionCaretViaAjaxStop ();
  // Trigger the caret moved handler.
  edit2CaretMovedTrigger = true;
}


function edit2HandleCaretMoved ()
{
  // If the caret has not yet been positioned, postpone the action.
  if (!edit2CaretInitialized) {
    edit2CaretMovedTimeoutStart ();
    edit2PositionCaretViaAjaxStart ();
    return;
  }

  if (quill.hasFocus ()) {
    // Initiate a new ajax request.
    edit2AjaxActive = true;
    $.ajax ({
      url: "navigate",
      type: "GET",
      data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter, offset: getCaretPosition () },
      success: function (response) {
        if (response != "") {
          // Set the focused verse immediately,
          // rather than waiting on the Navigator signal that likely will come later.
          // This fixes a clicking / scrolling problem.
          editorNavigationVerse = response;
          editorScheduleWindowScrolling ();
          editorHighlightVerseNumber ();
        }
      },
      error: function (jqXHR, textStatus, errorThrown) {
        // On (network) failure, reschedule the update.
        edit2CaretMovedTimeoutStart ();
      },
      complete: function (xhr, status) {
        edit2AjaxActive = false;
      }
    });
  } else {
    edit2CaretMovedTimeoutStart ();
  }

  editorActiveStylesFeedback ();
}


/*

Section with window events and their basic handlers.

*/

var edit2UndoChanged = false;


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
  // Undo.
  if ((event.metaKey == true) && (event.shiftKey == false) && (event.key == 'z')) {
    edit2UndoChanged = false;
    //var position = getCaretPosition ();
    quill.history.undo();
    if (edit2UndoChanged == false) {
      // This should fix an issue with the Quill editor.
      // When doing an undo, and the history stack is empty,
      // the Quill editor places the caret at the end of the text block.
      // The solution should have been to reposition the caret.
      // But repositioning the caret appears not to work.
      // After repositioning the caret, the caret remains at the end of the text block.
      //positionCaret (position);
      //positionCaretDelayed (position)
      //quill.setSelection (position, 0, "silent");
    }
    return false;
  }
  // Redo.
  if ((event.metaKey == true) && (event.shiftKey == true) && (event.key == 'z')) {
    // Checking on 2020-11-26 the .redo() did not work.
    quill.history.redo();
    return false;
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
  if (message == editorChapterUpdating) return;
  if (message == editorChapterUpdated) return;
  if (message == editorWillSave) return;
  if (message == editorChapterSaving) return;
  if (message == editorChapterSaved) return;
  if (message == "") return;
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


var edit2PositionCaretDelayedTimerId;
var edit2PositionCaretValue;


function positionCaretDelayed (position)
{
  if (edit2PositionCaretDelayedTimerId) {
    clearTimeout (edit2PositionCaretDelayedTimerId);
  }
  edit2PositionCaretValue = position;
  edit2PositionCaretDelayedTimerId = setTimeout (positionCaretDelayedTimeout, 100);
}


function positionCaretDelayedTimeout ()
{
  quill.setSelection (edit2PositionCaretValue, 0, "silent");
  //positionCaret (edit2PositionCaretValue);
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


var edit2PositionCaretViaAjaxTimerId;


function edit2PositionCaretViaAjaxStop ()
{
  if (edit2PositionCaretViaAjaxTimerId) {
    clearTimeout (edit2PositionCaretViaAjaxTimerId);
  }
}

function edit2PositionCaretViaAjaxStart ()
{
  edit2PositionCaretViaAjaxStop ();
  // Very frequent positioning calls have been seen in some browsers, so they are filtered here.
  edit2PositionCaretViaAjaxTimerId = setTimeout (edit2PositionCaretViaAjaxStartExecute, 100);
}


function edit2PositionCaretViaAjaxStartExecute ()
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
        edit2CaretInitialized = true;
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
  var scrollTo = bounds.top - (workspaceHeight * verticalCaretPosition / 100);
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
    url: "../editor/style",
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
  area.addClass ('style-of-stylesarea');
  area.append (response);
}


function editorClearStyles ()
{
  var area = $ ("#stylesarea");
  area.removeClass ('style-of-stylesarea');
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
    url: "../editor/style",
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
        edit2ContentChanged ();
      } else if (action == "m") {
        applyMonoStyle (style);
        edit2ContentChanged ();
      }
    },
  });
}


function displayAllStyles ()
{
  $.ajax ({
    url: "../editor/style",
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

  // Append note text to notes section.
  assetsEditorAddNote (quill, style, caller, noteId, editorNavigationChapter, verseSeparator, editorNavigationVerse);

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
    edit2PositionCaretViaAjaxStart ();
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


/*

Section for reload notifications.

*/


function editorReloadAlert (message)
{
  // Take action only if the editor has focus and the user can type in it.
  if (quill.hasFocus ()) {
    notifyItSuccess (message)
    quill.enable (false);
    setTimeout (editorReloadAlertTimeout, 3000);
  }
}


function editorReloadAlertTimeout ()
{
  quill.enable (editorWriteAccess);
  quill.focus ();
}



/*

Section for the coordinating timer.
This deals with the various events.
It monitors the ongoing AJAX actions for loading and updating and saving.
It decides which action to take.
It ensures that no two actions overlap or interfere with one another.
It also handles network latency,
by ensuring that the next call to the server
only occurs after the first call has been completed.
https://github.com/bibledit/cloud/issues/424

*/


var edit2UpdateTrigger = false;
var edit2CaretMovedTrigger = false;
var edit2AjaxActive = false;
var edit2PollSelector = 0;
var edit2PollDate = new Date();


function edit2CoordinatingTimeout ()
{
  // Handle situation that an AJAX call is ongoing.
  if (edit2AjaxActive) {
    
  }
  else if (edit2CaretMovedTrigger) {
    edit2CaretMovedTrigger = false;
    edit2HandleCaretMoved ();
  }
  else if (edit2UpdateTrigger) {
    edit2UpdateTrigger = false;
    edit2UpdateExecute ();
  }
  // Handle situation that no process is ongoing.
  // Now the regular pollers can run again.
  else {
    // There are two regular pollers.
    // Wait 500 ms, then start one of the pollers.
    // So each poller runs once a second.
    var difference = new Date () - edit2PollDate;
    if (difference > 500) {
      edit2PollSelector++;
      if (edit2PollSelector > 1) edit2PollSelector = 0;
      if (edit2PollSelector == 0) {
        edit2EditorPollId ();
      }
      if (edit2PollSelector == 1) {

      }
      edit2PollDate = new Date();
    }
  }
  setTimeout (edit2CoordinatingTimeout, 100);
}



/*

Section for the smart editor updating logic.

*/


var editorHtmlAtStartOfUpdate = null;
var useShadowQuill = false;


function edit2UpdateExecute ()
{
  // Determine whether the conditions for an editor update are all met.
  var goodToGo = true;
  if (!editorLoadedBible) goodToGo = false;
  if (!editorLoadedBook) goodToGo = false;
  if (!editorLoadedChapter) goodToGo = false;
  if (!goodToGo) {
    return;
  }

  // Clear the editor's edits.
  // The user can continue making changes in the editor.
  // These changes get recorded.
  edit2EditorChangeOffsets = [];
  edit2EditorChangeInserts = [];
  edit2EditorChangeDeletes = [];

  // A snapshot of the text originally loaded in the editor via AJAX.
  var encodedLoadedHtml = filter_url_plus_to_tag (editorReferenceText);

  // A snapshot of the current editor text at this point of time.
  editorHtmlAtStartOfUpdate = editorGetHtml();
  var encodedEditedHtml = filter_url_plus_to_tag (editorHtmlAtStartOfUpdate);

  // The editor "saves..." if there's changes, and "updates..." if there's no changes.
  if (editorHtmlAtStartOfUpdate == editorReferenceText) {
    editorStatus (editorChapterUpdating);
  } else {
    editorStatus (editorChapterSaving);
  }

  var checksum1 = checksum_get (encodedLoadedHtml);
  var checksum2 = checksum_get (encodedEditedHtml);

  edit2AjaxActive = true;

  $.ajax ({
    url: "update",
    type: "POST",
    async: true,
    data: { bible: editorLoadedBible, book: editorLoadedBook, chapter: editorLoadedChapter, loaded: encodedLoadedHtml, edited: encodedEditedHtml, checksum1: checksum1, checksum2: checksum2, id: chapterEditorUniqueID },
    error: function (jqXHR, textStatus, errorThrown) {
      editorStatus (editorChapterRetrying);
      edit2ContentChanged ();
    },
    success: function (response) {

      // Flag for editor read-write or read-only.
      // Do not set the read-only status of the editor here.
      // This is already set at text-load.
      // It is also dependent on the frame number the editor is in.
      // To not make it more complex than needed, leave read-only out.
      var readwrite = checksum_readwrite (response);

      // Checksumming.
      response = checksum_receive (response);
      if (response !== false) {
        
        // Use a shadow copy of the Quill editor in case the user made edits
        // between the moment the update routine was initiated,
        // and the moment the updates from the server/device are being applied.
        // This shadow copy will be updated with the uncorrected changes from the server/device.
        // The html from this editor will then server as the "loaded text".
        useShadowQuill = (edit2EditorChangeOffsets.length > 0);
        if (useShadowQuill) startShadowQuill (editorHtmlAtStartOfUpdate);

        // Split the response into the separate bits.
        var bits = [];
        bits = response.split ("#_be_#");

        // The first bit is the feedback message to the user.
        editorStatus (bits.shift());

        // The next bit is the new chapter identifier.
        oneverseChapterId = bits.shift();

        // Apply the remaining data, the differences, to the editor.
        while (bits.length > 0) {
          var position = parseInt (bits.shift ());
          var operator = bits.shift();
          // Position 0 in the incoming changes always refers to the initial new line in the editor.
          // Do not insert or delete that new line, but just apply any formatting there.
          if (position == 0) {
            if (operator == "p") {
              var style = bits.shift ();
              quill.formatLine (0, 0, {"paragraph": style}, "silent");
              if (useShadowQuill) quill2.formatLine (0, 0, {"paragraph": style}, "silent");
            }
          } else {
            // The initial new line is not counted in Quill.
            position--;
            // The position for the shadow copy of Quill, if it's there.
            var position2 = position;
            // Handle the insert operation.
            if (operator == "i") {
              // Get the information.
              var text = bits.shift ();
              var style = bits.shift ();
              var size = parseInt (bits.shift());
              // Correct the position
              // and the positions stored during the update procedure's network latency.
              position = oneverseUpdateIntermediateEdits (position, size, true, false);
              // Handle the insert operation.
              if (text == "\n") {
                // New line.
                quill.insertText (position, text, {}, "silent");
                if (useShadowQuill) quill2.insertText (position2, text, {}, "silent");
                quill.formatLine (position + 1, 1, {"paragraph": style}, "silent");
                if (useShadowQuill) quill2.formatLine (position2 + 1, 1, {"paragraph": style}, "silent");
              } else {
                // Ordinary character: Insert formatted text.
                quill.insertText (position, text, {"character": style}, "silent");
                if (useShadowQuill) quill2.insertText (position2, text, {"character": style}, "silent");
              }
            }
            // Handle delete operator.
            else if (operator == "d") {
              // Get the bits of information.
              var size = parseInt (bits.shift());
              // Correct the position and the positions
              // stored during the update procedure's network latency.
              position = oneverseUpdateIntermediateEdits (position, size, false, true);
              // Do the delete operation.
              quill.deleteText (position, size, "silent");
              if (useShadowQuill) quill2.deleteText (position2, size, "silent");
            }
            // Handle format paragraph operator.
            else if (operator == "p") {
              var style = bits.shift ();
              quill.formatLine (position + 1, 1, {"paragraph": style}, "silent");
              if (useShadowQuill) quill2.formatLine (position2 + 1, 1, {"paragraph": style}, "silent");
            }
            // Handle format character operator.
            else if (operator == "c") {
              var style = bits.shift ();
            }
          }
        }
        
      } else {
        // If the checksum is not valid, the response will become false.
        // Checksum error.
        editorStatus (editorChapterRetrying);
      }

      // The browser may reformat the loaded html, so take the possible reformatted data for reference.
      editorReferenceText = editorGetHtml ();
      if (useShadowQuill) editorReferenceText = $ ("#edittemp > .ql-editor").html ();
      $ ("#edittemp").empty ();
      
      // Create CSS for embedded styles.
      css4embeddedstyles ();
    },
    complete: function (xhr, status) {
      edit2AjaxActive = false;
    }
  });

}

                                          
var quill2 = undefined;


function startShadowQuill (html)
{
  if (quill2) delete quill2;
  $ ("#edittemp").empty ();
  $ ("#edittemp").append (html);
  quill2 = new Quill ('#edittemp', { });
}


function oneverseUpdateIntermediateEdits (position, size, ins_op, del_op)
{
  // The offsets of the changes from the server/device are going to be corrected
  // with the offsets of the edits made in the editor.
  // This is to handle continued user-typing while the editor is being updated,
  // Update, and get updated by, the edits made since the update operation began.
  // UTF-16 characters 4 bytes long have a size of 2 in Javascript.
  // So the routine takes care of that too.
  var i;
  for (i = 0; i < edit2EditorChangeOffsets.length; i++) {
    // Any delete or insert at a lower offset or the same offset
    // modifies the position where to apply the incoming edit from the server/device.
    if (edit2EditorChangeOffsets[i] <= position) {
      position += edit2EditorChangeInserts[i];
      position -= edit2EditorChangeDeletes[i]
    }
    // Any offset higher than the current position gets modified accordingly.
    // If inserting at the current position, increase that offset.
    // If deleting at the current position, decrease that offset.
    if (edit2EditorChangeOffsets[i] > position) {
      if (ins_op) edit2EditorChangeOffsets[i] += size;
      if (del_op) edit2EditorChangeOffsets[i] -= size;
    }
  }

  return position
}


//
//
// Clipboard paste handling.
//
//


// Pasting text into Quill has a few problems:
// https://github.com/bibledit/cloud/issues/717
// This paste handler aims to fix those.
function ChapterEditorHandlePaste (e)
{
  // Stop data from actually being pasted.
  e.stopPropagation();
  e.preventDefault();

  // Get the selected range, if any.
  // If the editor was not focused, the range is not defined:
  // Nothing more to do so bail out.
  var range = quill.getSelection();
  if (!range) return;
  
  // Unselect any selected text.
  if (range.length != 0) {
    quill.setSelection (range.index, range.length);
  }

  // Get pasted data via clipboard API.
  var clipboardData = e.clipboardData || window.clipboardData;
  var pastedData = clipboardData.getData('Text');

  // Inserting the text from the clipboard immediately into the editor does not work.
  // The work-around is to store data about where and what to paste.
  // And then to do the actual insert after a very short delay.
  pasteIndex = range.index;
  pasteText = pastedData;
  setTimeout (HandlePasteInsert, 10);
}

var pasteIndex = 0;
var pasteText = "";


function HandlePasteInsert()
{
  // Insert the text from the clipboard into the editor.
  quill.insertText(pasteIndex, pasteText);
}
