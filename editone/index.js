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


var quill = undefined;
var Delta = Quill.import ("delta");
var verseEditorUniqueID = Math.floor (Math.random() * 100000000);
var touchStartX = 0


document.addEventListener("DOMContentLoaded", function(e) {
  // Make the editor's menu to never scroll out of view.
  var bar = document.querySelector ("#editorheader");
  if (bar) {
    document.querySelector ("#workspacemenu").insertAdjacentHTML('beforeend', bar.outerHTML);
    bar.remove()
  }

  visualVerseEditorInitializeOnce ();
  visualVerseEditorInitializeLoad ();

  navigationNewPassage ();
  
  window.addEventListener("unload", function (event) {
    oneverseEditorForceSaveVerse();
  });

  oneverseBindUnselectable ();
  
  var stylebutton = document.querySelector ("#stylebutton");
  stylebutton.addEventListener("click", oneverseStylesButtonHandler);
  
  window.addEventListener ("keydown", oneverseWindowKeyHandler);

  if (swipe_operations) {
    // The minimum distance to swipe is 10% of the screen width.
    // This is to eliminate small unintended swipes.
    let minSwipeDistance = parseInt(window.screen.width / 10);
    
    document.body.addEventListener('touchstart', event => {
      touchStartX = event.changedTouches[0].screenX;
    });
    
    document.body.addEventListener('touchend', event => {
      let touchEndX = event.changedTouches[0].screenX
      if (touchEndX < touchStartX - minSwipeDistance) {
        oneverseSwipeLeft (event);
      }
      if (touchEndX > touchStartX + minSwipeDistance) {
        oneverseSwipeRight (event);
      }
    })
  }
  
  document.querySelector ("#oneeditor").addEventListener ("click", oneEditorNoteCitationClicked);
  
  setTimeout (oneverseCoordinatingTimeout, 500);

  document.body.addEventListener('paste', HandlePaste);
});


function visualVerseEditorInitializeOnce ()
{
  var Parchment = Quill.import ('parchment');
  
  // Register block formatting class.
  var ParagraphClass = new Parchment.Attributor.Class ('paragraph', 'b', { scope: Parchment.Scope.BLOCK });
  Quill.register (ParagraphClass, true);
  
  // Register inline formatting class.
  var CharacterClass = new Parchment.Attributor.Class ('character', 'i', { scope: Parchment.Scope.INLINE });
  Quill.register (CharacterClass, true);
}


function visualVerseEditorInitializeLoad ()
{
  // Work around https://github.com/quilljs/quill/issues/1116
  // It sets the margins to 0 by adding an overriding class.
  // The Quill editor will remove that class again.
  document.querySelectorAll("#oneeditor > p").forEach((element) => {
    element.classList.add ("nomargins");
  })

  // Instantiate editor.
  quill = new Quill ('#oneeditor', { });

  // Cause it to paste plain text only.
  quill.clipboard.addMatcher (Node.ELEMENT_NODE, function (node, delta) {
    var plaintext = node.innerText;
    return new Delta().insert (plaintext);
  });

  if (oneverseEditorWriteAccess) if (!verseEditorHasFocus ()) quill.focus ();
  
  // Event handlers.
  quill.on ("text-change", visualVerseEditorTextChangeHandler);
  quill.on ("selection-change", visualVerseEditorSelectionChangeHandler);
}


var oneverseBible;
var oneverseBook;
var oneverseNavigationBook;
var oneverseChapter;
var oneverseNavigationChapter;
var oneverseVerse;
var oneverseNavigationVerse;
var oneverseVerseLoading;
var oneverseVerseLoaded;
var oneverseEditorChangedTimeout;
var oneverseLoadedText;
var oneverseChapterId = 0;
var oneverseReloadCozChanged = false;
var oneverseReloadCozError = false;
var oneverseReloadPosition = undefined;
var abortController = new AbortController();
var oneverseEditorWriteAccess = true;


//
//
// Section for the new Passage event from the Navigator.
//
//


function navigationNewPassage ()
{
  if (is_outside_workspace()) {
    oneverseNavigationBook = navigationBook;
    oneverseNavigationChapter = navigationChapter;
    oneverseNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    oneverseNavigationBook = parent.window.navigationBook;
    oneverseNavigationChapter = parent.window.navigationChapter;
    oneverseNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }

  oneverseEditorForceSaveVerse ();
  oneverseReloadCozChanged = false;
  oneverseReloadCozError = false;
  oneverseEditorLoadVerse ();
}


//
//
// Section for editor load and save.
//
//


function oneverseEditorLoadVerse ()
{
  if ((oneverseNavigationBook != oneverseBook) || (oneverseNavigationChapter != oneverseChapter) || (oneverseNavigationVerse != oneverseVerse) || oneverseReloadCozChanged || oneverseReloadCozError ) {
    oneverseBible = navigationBible;
    oneverseBook = oneverseNavigationBook;
    oneverseChapter = oneverseNavigationChapter;
    oneverseVerse = oneverseNavigationVerse;
    oneverseVerseLoading = oneverseNavigationVerse;
    oneverseChapterId = 0;
    if (oneverseReloadCozChanged) {
      oneverseReloadPosition = oneverseCaretPosition ();
    } else {
      oneverseReloadPosition = undefined;
    }
    abortController.abort("");
    abortController = new AbortController();
    const url = "load?" + new URLSearchParams([ ["bible", oneverseBible], ["book", oneverseBook], ["chapter", oneverseChapter], ["verse", oneverseVerseLoading], ["id", verseEditorUniqueID] ]).toString()
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
      // Flag for editor read-write or read-only.
      oneverseEditorWriteAccess = checksum_readwrite (response);
      // If this is the second or third or higher editor in the workspace,
      // make the editor read-only.
      if (window.frameElement) {
        var iframe = window.frameElement;
        var data_editor_number = iframe.getAttribute("data-editor-no");
        if (data_editor_number > 1) {
          oneverseEditorWriteAccess = false;
        }
      }
      // Checksumming.
      response = checksum_receive (response);
      // Splitting.
      var bits;
      if (response !== false) {
        bits = response.split ("#_be_#");
        if (bits.length != 3) response == false;
      }
      if (response !== false) {
        var oneprefix = document.querySelector("#oneprefix");
        oneprefix.innerHTML = "";
        oneprefix.insertAdjacentHTML('beforeend', bits [0]);
        oneprefix.removeEventListener("click", oneVerseHtmlClicked, false);
        oneprefix.addEventListener("click", oneVerseHtmlClicked);
      }
      if (response !== false) {
        // Destroy existing editor.
        if (quill) delete quill;
        // Load the html in the DOM.
        var oneeditor = document.querySelector("#oneeditor");
        oneeditor.innerHTML = "";
        oneeditor.insertAdjacentHTML('beforeend', bits [1]);
        oneverseVerseLoaded = oneverseVerseLoading;
        oneverseEditorStatus (oneverseEditorVerseLoaded);
        // Create the editor based on the DOM's content.
        visualVerseEditorInitializeLoad ();
        quill.enable (oneverseEditorWriteAccess);
        // The browser may reformat the loaded html, so take the possible reformatted data for reference.
        oneverseLoadedText = document.querySelector ("#oneeditor > .ql-editor").innerHTML;
        oneverseCaretMovedTimeoutStart ();
        // Create CSS for embedded styles.
        css4embeddedstyles ();
      }
      if (response !== false) {
        oneverseEditorChangeOffsets = []
        oneverseEditorChangeInserts = []
        oneverseEditorChangeDeletes = []
      }
      if (response !== false) {
        var onesuffix = document.querySelector("#onesuffix");
        onesuffix.innerHTML = "";
        onesuffix.insertAdjacentHTML('beforeend', bits [2]);
        onesuffix.removeEventListener("click", oneVerseHtmlClicked, false);
        onesuffix.addEventListener("click", oneVerseHtmlClicked);
      }
      if (response !== false) {
        oneverseScrollVerseIntoView ();
        oneversePositionCaret ();
        // In case of network error, don't keep showing the notification.
        if (!oneverseReloadCozError) {
          if (oneverseReloadCozChanged) {
            if (oneverseEditorWriteAccess) oneverseReloadAlert (oneverseEditorVerseUpdatedLoaded);
          }
        }
        oneverseReloadCozChanged = false;
        oneverseReloadCozError = false;
      }
      if (response === false) {
        // Checksum or other error: Reload.
        oneverseReloadCozError = true;
        oneverseEditorLoadVerse ();
      }
    })
    .catch((error) => {
      console.log(error);
    })
    .finally(() => {
    });
  }
}


function oneverseEditorForceSaveVerse ()
{
  // If some conditions are not met, do not save.
  if (!oneverseEditorWriteAccess) return;
  if (!oneverseBible) return;
  if (!oneverseBook) return;
  if (!oneverseVerseLoaded) return;
  var html = document.querySelector("#oneeditor > .ql-editor").innerHTML;
  if (html == oneverseLoadedText) return;
  // Set the status as feedback to the user.
  oneverseEditorStatus (oneverseEditorVerseSaving);
  // This force-save cancels a normal save/update that might have been triggered.
  clearTimeout (oneverseEditorChangedTimeout);
  // Prepare the data to save.
  oneverseLoadedText = html;
  var encodedHtml = filter_url_plus_to_tag (html);
  var checksum = checksum_get (encodedHtml);
  // Do the actual save.
  fetch("save", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([ ["bible", oneverseBible], ["book", oneverseBook], ["chapter", oneverseChapter], ["verse", oneverseVerseLoaded], ["html", encodedHtml], ["checksum", checksum], ["id:", verseEditorUniqueID] ]).toString(),
    keepalive: true, // Make call synchronous.
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    oneverseEditorStatus (response);
  })
  .catch((error) => {
    console.log(error);
    oneverseEditorStatus (oneverseEditorVerseRetrying);
    oneverseLoadedText = "";
    oneverseEditorForceSaveVerse ();
  })
  .finally(() => {
  });
}


function oneverseEditorLoadNonEditable ()
{
  oneverseAjaxActive = true;
  const url = "load?" + new URLSearchParams([ ["bible", oneverseBible], ["book", oneverseBook], ["chapter", oneverseChapter], ["verse", oneverseVerseLoading], ["id", verseEditorUniqueID] ]).toString()
  fetch(url, {
    method: "GET",
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    // Remove the flag for editor read-write or read-only.
    checksum_readwrite (response);
    // Checksumming.
    response = checksum_receive (response);
    // Splitting.
    if (response !== false) {
      var bits;
      bits = response.split ("#_be_#");
      if (bits.length != 3) response == false;
      var prefix = document.querySelector("#oneprefix");
      prefix.innerHTML = "";
      prefix.insertAdjacentHTML('beforeend', bits [0]);
      prefix.removeEventListener("click", oneVerseHtmlClicked, false);
      prefix.addEventListener("click", oneVerseHtmlClicked);
      var suffix = document.querySelector("#onesuffix");
      suffix.innerHTML = "";
      suffix.insertAdjacentHTML('beforeend', bits [2]);
      suffix.removeEventListener("click", oneVerseHtmlClicked, false);
      suffix.addEventListener("click", oneVerseHtmlClicked);
    }
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    oneverseAjaxActive = false;
  });
}


//
//
// Portion dealing with triggers for editor's content change.
//
//


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
var oneverseEditorChangeOffsets = [];
var oneverseEditorChangeInserts = [];
var oneverseEditorChangeDeletes = [];


// Arguments: delta: Delta, oldContents: Delta, source: String
function visualVerseEditorTextChangeHandler (delta, oldContents, source)
{
  // Whether a space was typed.
  var space_typed = false;
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
    if (obj.insert) {
      insert = obj.insert.length;
      if (obj.insert.charCodeAt(0) == 160) space_typed = true;
    }
    if (obj.delete) del = obj.delete;
  }
  oneverseEditorChangeOffsets.push(retain);
  oneverseEditorChangeInserts.push(insert);
  oneverseEditorChangeDeletes.push(del);
  // Ensure that it does not delete a chapter number or verse number.
  if (!delta.ops [0].retain) {
    quill.history.undo ();
  }
  // Start save delay timer.
  oneverseEditorChanged (space_typed);
}


function oneverseEditorChanged (space_typed)
{
  if (!oneverseEditorWriteAccess) return;
  oneverseEditorStatus (oneverseEditorWillSave);
  if (oneverseEditorChangedTimeout) {
    clearTimeout (oneverseEditorChangedTimeout);
  }
  // If a space was typed, do not save very soon.
  // This is to fix issue https://github.com/bibledit/cloud/issues/481:
  // "Verse editor refuses to accept a space at the end of a line".
  // The USFM save system does not retain a space at the end of the line.
  // The fix is that when a space is typed, not to save right away, but with a longer delay.
  // That longer delay allows the user to decide what character to type after that space.
  var delay = 1000;
  if (space_typed) delay = 5000;
  oneverseEditorChangedTimeout = setTimeout (oneverseEditorTriggerSave, delay);
}


function oneverseEditorTriggerSave ()
{
  if (!oneverseUpdateTrigger) {
    oneverseUpdateTrigger = true;
  } else {
    if (oneverseEditorChangedTimeout) {
      clearTimeout (oneverseEditorChangedTimeout);
    }
    oneverseEditorChangedTimeout = setTimeout (oneverseEditorTriggerSave, 400);
  }
}


//
//
// Section for user interface updates and feedback.
//
//


function oneverseEditorStatus (text)
{
  var status = document.querySelector("#onestatus");
  if (status)
    status.innerHTML = text;
  oneverseEditorSelectiveNotification (text);
}


function oneverseActiveStylesFeedback ()
{
  var format = quill.getFormat ();
  var paragraph = "...";
  if (format.paragraph) paragraph = format.paragraph;
  var character = "";
  if (format.character) character = format.character;
  if (character.search ("note") >= 0) character = "";
  var pos = character.indexOf ("wla");
  if (pos >= 0) character = character.substring(0, pos);
  character = character.split ("0").join (" ");
  var styles = paragraph + " " + character;
  var element = document.querySelector ("#oneverseactivestyles");
  element.innerHTML = styles;
}


function oneverseEditorSelectiveNotification (message)
{
  if (message == oneverseEditorVerseLoaded) return;
  if (message == oneverseEditorWillSave) return;
  if (message == oneverseEditorVerseSaving) return;
  if (message == oneverseEditorVerseSaved) return;
  if (message == oneverseEditorVerseUpdating) return;
  if (message == oneverseEditorVerseUpdated) return;
  notifyError (message);
}


//
//
// Section for polling the server for updates on the loaded chapter.
//
//


function oneverseEditorPollId ()
{
  oneverseAjaxActive = true;
  const url = "../editor/id?" + new URLSearchParams([ ["bible", oneverseBible], ["book", oneverseBook], ["chapter", oneverseChapter] ]).toString()
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
  .then((response) => {
    if (oneverseChapterId != 0) {
      if (response != oneverseChapterId) {
        // The chapter identifier changed.
        // That means that likely there's updated text on the server.
        // Start the routine to load any possible updates into the editor.
        oneverseUpdateTrigger = true;
        oneverseReloadNonEditableFlag = true;
      }
    }
    oneverseChapterId = response;
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    oneverseAjaxActive = false;
  });
}


//
//
// Section for getting and setting the caret position.
//
//


function oneverseCaretPosition ()
{
  var position = undefined;
  var range = quill.getSelection();
  if (range) position = range.index;
  return position;
}


function oneversePositionCaret ()
{
  setTimeout (oneversePositionCaretTimeout, 100);
}


function oneversePositionCaretTimeout ()
{
  var position;
  if (oneverseReloadPosition != undefined) {
    position = oneverseReloadPosition;
    oneverseReloadPosition = undefined;
  } else {
    position = 1 + oneverseVerse.length;
  }
  quill.setSelection (position, 0, "silent");
}


//
//
// Section for scrolling the editable portion into the center.
//
//


function oneverseScrollVerseIntoView ()
{
  var oneeditor = document.querySelector("#oneeditor");
  oneeditor.scrollIntoView({
    behavior: 'smooth',
    block: 'center',
    inline: 'center'
  });
}


//
//
// Section for the styles handling.
//
//


function oneverseStylesButtonHandler ()
{
  if (!oneverseEditorWriteAccess) return;
  fetch("../editor/style", {
    method: "GET",
    cache: "no-cache"
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    oneverseShowResponse (response);
    oneverseBindUnselectable ();
    oneverseDynamicClickHandlers ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function oneverseBindUnselectable ()
{
  var elements = document.querySelectorAll(".unselectable");
  elements.forEach((element) => {
    element.removeEventListener("mousedown", oneverseUnselectablePreventDefault, false);
    element.addEventListener("mousedown", oneverseUnselectablePreventDefault);
  });
}


function oneverseUnselectablePreventDefault(event)
{
  event.preventDefault();
}


function oneverseShowResponse (response)
{
  if (!oneverseEditorWriteAccess) return;
  document.querySelector ("#stylebutton").hidden = true;
  document.querySelector ("#nostyles").hidden = true;
  var area = document.querySelector ("#stylesarea");
  area.innerHTML = "";
  area.classList.add ('style-of-stylesarea');
  area.insertAdjacentHTML('beforeend', response);
}


function oneverseClearStyles ()
{
  var area = document.querySelector ("#stylesarea");
  area.classList.remove ('style-of-stylesarea');
  area.innerHTML = "";
  document.querySelector ("#stylebutton").hidden = false;
  document.querySelector ("#nostyles").hidden = false;
}


function oneverseDynamicClickHandlers ()
{
  var elements = document.querySelectorAll ("#stylesarea > a");
  elements.forEach((element) => {
    element.addEventListener("click", function(event) {
      event.preventDefault();
      oneverseClearStyles ();
      var href = event.currentTarget.href;
      href = href.substring (href.lastIndexOf ('/') + 1);
      if (href == "cancel") return;
      if (href == "all") {
        oneverseDisplayAllStyles ();
      } else {
        oneverseRequestStyle (href);
      }
    });
  });
  var styleslist = document.querySelector("#styleslist");
  if (styleslist) {
    styleslist.addEventListener("change", function (event) {
      var selection = document.querySelector ("#styleslist").value;
      var style = selection.substring (0, selection.indexOf (" "));
      event.preventDefault();
      oneverseClearStyles ();
      oneverseRequestStyle (style);
    });
  }
}


function oneverseRequestStyle (style)
{
  const url = "../editor/style?style=" + style;
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
  .then((response) => {
    response = response.split ("\n");
    var style = response [0];
    var action = response [1];
    if (action == "p") {
      oneverseApplyParagraphStyle (style);
    } else if (action == 'c') {
      oneverseApplyCharacterStyle (style);
    } else if (action == 'n') {
      oneverseApplyNotesStyle (style);
    } else if (action == "m") {
      oneverseApplyMonoStyle (style);
    }
  })
  .catch((error) => {
    console.log(error);
  })
}


function oneverseDisplayAllStyles ()
{
  const url = "../editor/style?all=";
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
  .then((response) => {
    oneverseShowResponse (response);
    oneverseBindUnselectable ();
    oneverseDynamicClickHandlers ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function oneverseApplyParagraphStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  if (!verseEditorHasFocus ()) quill.focus ();
  quill.format ("paragraph", style, "user");
  oneverseActiveStylesFeedback ();
}


function oneverseApplyCharacterStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  if (!verseEditorHasFocus ()) quill.focus ();
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
  // Deal with embedded character styles.
  post_embedded_style_application (style);
  // Feedback.
  oneverseActiveStylesFeedback ();
}


function oneverseApplyMonoStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  
  quill.focus ();

  var range = quill.getSelection();
  var caret = range.index;

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

  oneverseActiveStylesFeedback ();
}


//
//
// Section with window events and their basic handlers.
//
//


function oneverseWindowKeyHandler (event)
{
  if (!oneverseEditorWriteAccess) return;
  // Ctrl-S: Style.
  if ((event.ctrlKey == true) && (event.keyCode == 83)) {
    oneverseStylesButtonHandler ();
    return false;
  }
  // Escape.
  if (event.keyCode == 27) {
    oneverseClearStyles ();
  }
}


//
//
// Section responding to a moved caret.
//
//


// Responds to a changed selection or caret.
// range: { index: Number, length: Number }
// oldRange: { index: Number, length: Number }
// source: String
function visualVerseEditorSelectionChangeHandler (range, oldRange, source)
{
  // Bail out if editor not focused.
  if (!range) return;

  // Bail out if text was selected.
  if (range.length != 0) return;

  oneverseCaretMovedTimeoutStart ();
}


var oneverseCaretMovedTimeoutId;


function oneverseCaretMovedTimeoutStart ()
{
  if (oneverseCaretMovedTimeoutId) clearTimeout (oneverseCaretMovedTimeoutId);
  oneverseCaretMovedTimeoutId = setTimeout (oneverseActiveStylesFeedback, 200);
}


//
//
// Section for the notes.
//
//


var oneverseInsertedNotesCount = 0;


function oneverseApplyNotesStyle (style)
{
  if (!oneverseEditorWriteAccess) return;

  quill.focus ();

  // Check for and optionally append the gap between text body and notes.
  var notes = document.querySelector (".b-notes");
  if (!notes) {
    var length = quill.getLength ();
    quill.insertText (length, "\n", "paragraph", "notes", "user")
  }
  
  // Get a new node identifier based on the local time.
  var date = new Date();
  var noteId = String (date.getTime());
  
  var caller = String ((oneverseInsertedNotesCount % 9) + 1);

  // Insert note caller at caret.
  var range = quill.getSelection();
  var caret = range.index;
  quill.setSelection (caret, 0);
  quill.insertText (caret, caller, "character", "notecall" + noteId, "user");

  // Append note text to notes section.
  assetsEditorAddNote (quill, style, caller, noteId, oneverseNavigationChapter, verseSeparator, oneverseNavigationVerse);

  oneverseInsertedNotesCount++;
  
  oneverseActiveStylesFeedback ();
}


function oneEditorNoteCitationClicked (event)
{
  var cls = event.target.className;
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
        oneverseActiveStylesFeedback ();
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
        oneverseActiveStylesFeedback ();
        return;
      }
    }
  }
}


//
//
// Section for navigating to another passage.
//
//


function oneVerseHtmlClicked (event)
{
  // If the user has selected text, do nothing.
  var text = "";
  if (window.getSelection) {
    text = window.getSelection().toString();
  } else if (document.selection && document.selection.type != "Control") {
    text = document.selection.createRange().text;
  }
  if (text.length) return;
  
  var verse = "";
  
  var iterations = 0;
  var target = event.target;
  var tagName = target.tagName;
  if (tagName == "P") target = target.lastChild;
  while ((iterations < 10) && (!target.classList.contains ("i-v"))) {
    var previous = target.previousSibling;
    if (previous) {
      target = previous;
    } else {
      target = target.parentElement.previousSibling;
      if (target) target = target.lastChild;
    }
    iterations++;
    if (!target) break;
  }
                                          
  // Too many iterations: Undefined location.
  if (iterations >= 10) return
  
  if (target) {
    if (target.classList.contains ("i-v")) {
      verse = target.innerText;
    }
  } else {
    verse = "0";
  }
  const url = "verse?" + new URLSearchParams([ ["verse", verse], ["focusgroup", focusGroup] ]).toString();
  fetch(url, {
    method: "GET",
    cache: "no-cache"
  })
  .catch((error) => {
    console.log(error);
  })
}


//
//
// Section for swipe navigation.
//
//


function oneverseSwipeLeft (event)
{
  if (is_outside_workspace()) {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function oneverseSwipeRight (event)
{
  if (is_outside_workspace()) {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}


/*

Section for reload notifications.

*/


function oneverseReloadAlert (message)
{
  // Take action only if the editor has focus and the user can type in it.
  if (!verseEditorHasFocus ()) return;
  // Do the notification stuff.
  notifySuccess (message)
  quill.enable (false);
  setTimeout (oneverseReloadAlertTimeout, 3000);
}


function oneverseReloadAlertTimeout ()
{
  quill.enable (oneverseEditorWriteAccess);
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


var oneverseAjaxActive = false;
var oneversePollSelector = 0;
var oneversePollDate = new Date();
var oneverseUpdateTrigger = false;
var oneverseReloadNonEditableFlag = false;


function oneverseCoordinatingTimeout ()
{
  // Handle situation that an AJAX call is ongoing.
  if (oneverseAjaxActive) {
    
  }
  else if (oneverseUpdateTrigger) {
    oneverseUpdateTrigger = false;
    oneverseUpdateExecute ();
  }
  else if (oneverseReloadNonEditableFlag) {
    oneverseReloadNonEditableFlag = false;
    oneverseEditorLoadNonEditable ();
  }
  // Handle situation that no process is ongoing.
  // Now the regular pollers can run again.
  else {
    // There are two regular pollers.
    // Wait 500 ms, then start one of the pollers.
    // So each poller runs once a second.
    var difference = new Date () - oneversePollDate;
    if (difference > 500) {
      oneversePollSelector++;
      if (oneversePollSelector > 1) oneversePollSelector = 0;
      if (oneversePollSelector == 0) {
        oneverseEditorPollId ();
      }
      if (oneversePollSelector == 1) {

      }
      oneversePollDate = new Date();
    }
  }
  setTimeout (oneverseCoordinatingTimeout, 100);
}


/*

Section for the smart editor updating logic.

*/


var editorHtmlAtStartOfUpdate = null;
var useShadowQuill = false;


function oneverseUpdateExecute ()
{
  // Determine whether the conditions for an editor update are all met.
  var goodToGo = true;
  if (!oneverseBible) goodToGo = false;
  if (!oneverseBook) goodToGo = false;
  if (!oneverseVerseLoaded) goodToGo = false;
  if (!goodToGo) {
    return;
  }

  // Clear the editor's edits.
  // The user can continue making changes in the editor.
  // These changes get recorded.
  oneverseEditorChangeOffsets = [];
  oneverseEditorChangeInserts = [];
  oneverseEditorChangeDeletes = [];

  // A snapshot of the text originally loaded in the editor via AJAX.
  var encodedLoadedHtml = filter_url_plus_to_tag (oneverseLoadedText);

  // A snapshot of the current editor text at this point of time.
  editorHtmlAtStartOfUpdate = document.querySelector ("#oneeditor > .ql-editor").innerHTML;

  var encodedEditedHtml = filter_url_plus_to_tag (editorHtmlAtStartOfUpdate);
  
  // The editor "saves..." if there's changes, and "updates..." if there's no changes.
  if (editorHtmlAtStartOfUpdate == oneverseLoadedText) {
    oneverseEditorStatus (oneverseEditorVerseUpdating);
  } else {
    oneverseEditorStatus (oneverseEditorVerseSaving);
  }

  var checksum1 = checksum_get (encodedLoadedHtml);
  var checksum2 = checksum_get (encodedEditedHtml);

  oneverseAjaxActive = true;

  fetch("update", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams([ ["bible", oneverseBible], ["book", oneverseBook], ["chapter", oneverseChapter], ["verse", oneverseVerseLoaded], ["loaded", encodedLoadedHtml], ["edited", encodedEditedHtml], ["checksum1", checksum1], ["checksum2", checksum2], ["id", verseEditorUniqueID] ]).toString(),
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {

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
      useShadowQuill = (oneverseEditorChangeOffsets.length > 0);
      if (useShadowQuill) startShadowQuill (editorHtmlAtStartOfUpdate);

      // Split the response into the separate bits.
      var bits = [];
      bits = response.split ("#_be_#");

      // The first bit is the feedback message to the user.
      oneverseEditorStatus (bits.shift());

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
      oneverseEditorStatus (oneverseEditorVerseRetrying);
    }

    // The browser may reformat the loaded html, so take the possible reformatted data for reference.
    oneverseLoadedText = document.querySelector ("#oneeditor > .ql-editor").innerHTML;
    if (useShadowQuill) oneverseLoadedText = document.querySelector ("#onetemp > .ql-editor").innerHTML;
    document.querySelector ("#onetemp").innerHTML = "";

    // Create CSS for embedded styles.
    css4embeddedstyles ();

  })
  .catch((error) => {
    console.log(error);
    oneverseEditorStatus (oneverseEditorVerseRetrying);
    oneverseEditorChanged (false);
  })
  .finally(() => {
    oneverseAjaxActive = false;
  });
}

                                          
var quill2 = undefined;


function startShadowQuill (html)
{
  if (quill2) delete quill2;
  var onetemp = document.querySelector ("#onetemp");
  onetemp.innerHTML = "";
  onetemp.insertAdjacentHTML('beforeend', html);
  quill2 = new Quill ('#onetemp', { });
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
  for (i = 0; i < oneverseEditorChangeOffsets.length; i++) {
    // Any delete or insert at a lower offset or the same offset
    // modifies the position where to apply the incoming edit from the server/device.
    if (oneverseEditorChangeOffsets[i] <= position) {
      position += oneverseEditorChangeInserts[i];
      position -= oneverseEditorChangeDeletes[i]
    }
    // Any offset higher than the current position gets modified accordingly.
    // If inserting at the current position, increase that offset.
    // If deleting at the current position, decrease that offset.
    if (oneverseEditorChangeOffsets[i] > position) {
      if (ins_op) oneverseEditorChangeOffsets[i] += size;
      if (del_op) oneverseEditorChangeOffsets[i] -= size;
    }
  }

  return position
}



//
//
// Section for the focus.
//
//


function verseEditorHasFocus ()
{
  var focus = getActiveElement();
  var focused = (focus.className == "ql-editor");
  if (focused) focused = quill.hasFocus ();
  return focused;
}


//
//
// Clipboard paste handling.
//
//


// Pasting text into Quill has a few problems:
// https://github.com/bibledit/cloud/issues/717
// This paste handler aims to fix those.
function HandlePaste (e)
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
