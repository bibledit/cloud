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
  
  $ ("#oneeditor").on ("paste cut keydown", oneverseEditorChanged);
  
  $ (window).on ("unload", oneverseEditorUnload);
  
  $ ("#oneeditor").on ("paste", function (e) {
    var data = e.originalEvent.clipboardData.getData ('Text');
    e.preventDefault();
    document.execCommand ("insertHTML", false, data);
  });
  
  oneverseIdPoller ();

  oneverseBindUnselectable ();
  
  $ ("#stylebutton").on ("click", oneverseStylesButtonHandler);
  
  $ (window).on ("keydown", oneverseWindowKeyHandler);

  $ ("#oneeditor").on ("click", oneverseCaretChangedByMouse);
  
  $ ("#oneeditor").on ("keydown", oneverseCaretChangedByKeyboard);

  $ ("#oneeditor").focus ();
  
  if (swipe_operations) {
    $ ("body").swipe ( {
      swipeLeft:function (event, direction, distance, duration, fingerCount, fingerData) {
        oneverseSwipeLeft (event);
      },
      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
        oneverseSwipeRight (event);
      }
    });
  }
});


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
var oneverseIdChapter = 0;
var oneverseIdTimeout;
var oneverseReload = false;
var oneverseEditorTextChanged = false;
var oneverseSaveAsync;
var oneverseLoadAjaxRequest;
var oneverseSaving = false;
var oneverseAddedStyle = 0;


/*
 
Section for the new Passage event from the Navigator.
 
*/


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
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
  oneverseEditorSaveVerse ();
  oneverseReload = false;
  oneverseEditorLoadVerse ();
}


/*
 
Section for editor load and save.
 
*/


function oneverseEditorLoadVerse ()
{
  if ((oneverseNavigationBook != oneverseBook) || (oneverseNavigationChapter != oneverseChapter) || (oneverseNavigationVerse != oneverseVerse) || oneverseReload) {
    oneverseBible = navigationBible;
    oneverseBook = oneverseNavigationBook;
    oneverseChapter = oneverseNavigationChapter;
    oneverseVerse = oneverseNavigationVerse;
    oneverseVerseLoading = oneverseNavigationVerse;
    oneverseIdChapter = 0;
    if (oneverseLoadAjaxRequest && oneverseLoadAjaxRequest.readystate != 4) {
      oneverseLoadAjaxRequest.abort();
    }
    oneverseLoadAjaxRequest = $.ajax ({
      url: "load",
      type: "GET",
      data: { bible: oneverseBible, book: oneverseBook, chapter: oneverseChapter, verse: oneverseVerseLoading },
      success: function (response) {
        oneverseEditorWriteAccess = checksum_readwrite (response);
        var contenteditable = ($ ("#oneeditor").attr('contenteditable') === 'true');
        if (oneverseEditorWriteAccess != contenteditable) $ ("#oneeditor").attr('contenteditable', oneverseEditorWriteAccess);
        // Checksumming.
        response = checksum_receive (response);
        // Splitting.
        var bits;
        if (response !== false) {
          bits = response.split ("#_be_#");
          if (bits.length != 4) response == false;
        }
        if (response !== false) {
          oneverseAddedStyle = bits [0];
        }
        if (response !== false) {
          $ ("#oneprefix").empty ();
          $ ("#oneprefix").append (bits [1]);
          $ ("#oneprefix").off ("click");
          $ ("#oneprefix").on ("click", oneVerseHtmlClicked);
        }
        if (response !== false) {
          $ ("#oneeditor").empty ();
          $ ("#oneeditor").append (bits [2]);
          oneverseVerseLoaded = oneverseVerseLoading;
          oneverseEditorStatus (oneverseEditorVerseLoaded);
          // The browser may reformat the loaded html, so take the possible reformatted data for reference.
          oneverseLoadedText = $ ("#oneeditor").html ();
          oneverseReload = false;
          oneverseCaretMovedTimeoutStart ();
        }
        if (response !== false) {
          $ ("#onesuffix").empty ();
          $ ("#onesuffix").append (bits [3]);
          $ ("#onesuffix").off ("click");
          $ ("#onesuffix").on ("click", oneVerseHtmlClicked);
        }
        if (response !== false) {
          oneverseScrollVerseIntoView ();
          oneversePositionCaret ();
        }
        if (response === false) {
          // Checksum or other error: Reload.
          oneverseReload = true;
          oneverseEditorLoadVerse ();
        }
      },
    });
  }
}


function oneverseEditorUnload ()
{
  oneverseEditorSaveVerse (true);
}


function oneverseEditorSaveVerse (sync)
{
  if (oneverseSaving) {
    oneverseEditorChanged ();
    return;
  }
  if (!oneverseEditorWriteAccess) return;
  oneverseEditorTextChanged = false;
  if (!oneverseBible) return;
  if (!oneverseBook) return;
  if (!oneverseVerseLoaded) return;
  var html = $ ("#oneeditor").html ();
  if (html == oneverseLoadedText) return;
  oneverseEditorStatus (oneverseEditorVerseSaving);
  oneverseLoadedText = html;
  oneverseIdChapter = 0;
  oneverseSaveAsync = true;
  if (sync) oneverseSaveAsync = false;
  var encodedHtml = filter_url_plus_to_tag (html);
  var checksum = checksum_get (encodedHtml);
  oneverseSaving = true;
  $.ajax ({
    url: "save",
    type: "POST",
    async: oneverseSaveAsync,
    data: { bible: oneverseBible, book: oneverseBook, chapter: oneverseChapter, verse: oneverseVerseLoaded, html: encodedHtml, checksum: checksum, style: oneverseAddedStyle },
    error: function (jqXHR, textStatus, errorThrown) {
      oneverseEditorStatus (oneverseEditorVerseRetrying);
      oneverseLoadedText = "";
      oneverseEditorChanged ();
      if (!oneverseSaveAsync) oneverseEditorSaveVerse (true);
    },
    success: function (response) {
      oneverseEditorStatus (response);
    },
    complete: function (xhr, status) {
      oneverseSaveAsync = true;
      oneverseSaving = false;
    }
  });
}


/*
 
Portion dealing with triggers for editor's content change.
 
*/


function oneverseEditorChanged (event)
{
  if (!oneverseEditorWriteAccess) return;
  if (editKeysIgnoreForContentChange (event)) return;
  oneverseEditorTextChanged = true;
  if (oneverseEditorChangedTimeout) {
    clearTimeout (oneverseEditorChangedTimeout);
  }
  oneverseEditorChangedTimeout = setTimeout (oneverseEditorSaveVerse, 1000);
}


/*
 
Section for user interface updates and feedback.
 
*/


function oneverseEditorStatus (text)
{
  $ ("#onestatus").empty ();
  $ ("#onestatus").append (text);
  oneverseEditorSelectiveNotification (text);
}


function oneverseActiveStylesFeedback ()
{
  var editor = $ ("#oneeditor");
  if (editor.is (":focus")) {
    var parent = rangy.getSelection().anchorNode.parentElement;
    parent = $ (parent);
    var paragraph = parent.closest ("p");
    var pname = paragraph.attr ('class');
    if (pname == undefined) pname = "...";
    var span = parent.closest ("span");
    var cname = span.attr ("class");
    if (cname == undefined) cname = "";
    var element = $ ("#oneverseactivestyles");
    element.text (pname + " " + cname);
  }
}


function oneverseEditorSelectiveNotification (message)
{
  if (message == oneverseEditorVerseLoaded) return;
  if (message == oneverseEditorVerseSaving) return;
  if (message == oneverseEditorVerseSaved) return;
  notifyItError (message);
}


/*
 
Section for polling the server for updates on the loaded chapter.
 
*/


function oneverseIdPoller ()
{
  if (oneverseIdTimeout) {
    clearTimeout (oneverseIdTimeout);
  }
  oneverseIdTimeout = setTimeout (oneverseEditorPollId, 1000);
}


function oneverseEditorPollId ()
{
  $.ajax ({
    url: "../edit/id",
    type: "GET",
    data: { bible: oneverseBible, book: oneverseBook, chapter: oneverseChapter },
    cache: false,
    success: function (response) {
      if (!oneverseSaving) {
        if (oneverseIdChapter != 0) {
          if (response != oneverseIdChapter) {
            oneverseReload = true;
            oneverseEditorLoadVerse ();
            oneverseIdChapter = 0;
          }
        }
        oneverseIdChapter = response;
      }
    },
    complete: function (xhr, status) {
      oneverseIdPoller ();
    }
  });
}


/*
 
Section for getting and setting the caret position.

*/


function oneversePositionCaret ()
{
  setTimeout (oneversePositionCaretTimeout, 100);
}


function oneversePositionCaretTimeout ()
{
  var selection = rangy.getSelection ();
  var offset = 1 + oneverseVerse.length;
  selection.move ("character", offset);
}


/*

Section for scrolling the editable portion into the center.

*/


function oneverseScrollVerseIntoView ()
{
  $("#workspacewrapper").stop();
  var verseTop = $("#oneeditor").offset().top;
  var workspaceHeight = $("#workspacewrapper").height();
  var currentScrollTop = $("#workspacewrapper").scrollTop();
  var scrollTo = verseTop - (workspaceHeight * verticalCaretPosition / 100) + currentScrollTop;
  var lowerBoundary = currentScrollTop - (workspaceHeight / 10);
  var upperBoundary = currentScrollTop + (workspaceHeight / 10);
  if ((scrollTo < lowerBoundary) || (scrollTo > upperBoundary)) {
    $("#workspacewrapper").animate({ scrollTop: scrollTo }, 500);
  }
}


/*

Section for the styles handling.

*/


function oneverseStylesButtonHandler ()
{
  if (!oneverseEditorWriteAccess) return;
  $.ajax ({
    url: "../edit/styles",
    type: "GET",
    cache: false,
    success: function (response) {
      oneverseShowResponse (response);
      oneverseBindUnselectable ();
      oneverseDynamicClickHandlers ();
    },
  });
  return false;
}


function oneverseBindUnselectable ()
{
  var elements = $ (".unselectable");
  elements.off ("mousedown");
  elements.on ("mousedown", function (event) {
    event.preventDefault();
  });
}


function oneverseShowResponse (response)
{
  if (!oneverseEditorWriteAccess) return;
  $ ("#stylebutton").hide ();
  $ ("#nostyles").hide ();
  var area = $ ("#stylesarea");
  area.empty ();
  area.append (response);
}


function oneverseClearStyles ()
{
  var area = $ ("#stylesarea");
  area.empty ();
  $ ("#stylebutton").show ();
  $ ("#nostyles").show ();
}


function oneverseDynamicClickHandlers ()
{
  var elements = $ ("#stylesarea > a");
  elements.on ("click", function (event) {
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
  
  $ ("#styleslist").on ("change", function (event) {
    var selection = $ ("#styleslist option:selected").text ();
    var style = selection.substring (0, selection.indexOf (" "));
    event.preventDefault();
    oneverseClearStyles ();
    oneverseRequestStyle (style);
  });
}


function oneverseRequestStyle (style)
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
        oneverseApplyParagraphStyle (style);
        oneverseEditorChanged ();
      } else if (action == 'c') {
        oneverseApplyCharacterStyle (style);
        oneverseEditorChanged ();
      } else if (action == 'n') {
        oneverseApplyNotesStyle (style);
        oneverseEditorChanged ();
      } else if (action == "m") {
        oneverseApplyMonoStyle (style);
        oneverseEditorChanged ();
      }
    },
  });
}


function oneverseDisplayAllStyles ()
{
  $.ajax ({
    url: "../edit/styles",
    type: "GET",
    data: { all: "" },
    cache: false,
    success: function (response) {
      oneverseShowResponse (response);
      oneverseBindUnselectable ();
      oneverseDynamicClickHandlers ();
    },
  });
}


function oneverseApplyParagraphStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  $ ("#editor").focus ();
  var parent = rangy.getSelection().anchorNode.parentElement;
  parent = $ (parent);
  var paragraph = parent.closest ("p");
  paragraph.removeClass ();
  paragraph.addClass (style);
}


function oneverseApplyCharacterStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  $ ("#editor").focus ();
  var classApplier = rangy.createClassApplier (style);
  classApplier.toggleSelection ();
}


function oneverseApplyMonoStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  
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
 
Section with window events and their basic handlers.
 
*/


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


/*
 
Section responding to a moved caret.
 
*/


var oneverseCaretMovedTimeoutId;


function oneverseCaretChangedByMouse (event)
{
  oneverseCaretMovedTimeoutStart ();
}


function oneverseCaretChangedByKeyboard (event)
{
  if (editKeysIgnoreForCaretChange (event)) return;
  oneverseCaretMovedTimeoutStart ();
}


function oneverseCaretMovedTimeoutStart ()
{
  if (oneverseCaretMovedTimeoutId) clearTimeout (oneverseCaretMovedTimeoutId);
  oneverseCaretMovedTimeoutId = setTimeout (oneverseActiveStylesFeedback, 200);
}


/*
 
Section for the notes.
 
*/


var oneverseInsertedNotesCount = 0;


function oneverseApplyNotesStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  
  $ ("#oneeditor").focus ();
  
  // Check for / append notes section.
  var notes = $ ("#notes");
  if (notes.length == 0) {
    $ ("#oneeditor").append ('<div id="notes"><hr></div>');
  }
  
  // Get a new node identifier based on the local time.
  var date = new Date();
  var noteId = date.getTime();
  
  var caller = (oneverseInsertedNotesCount % 9) + 1;
  
  // Insert note caller at caret.
  var html = '<a href="#note' + noteId + '" id="citation' + noteId + '" class="superscript">' + caller + '</a>';
  insertHtmlAtCaret (html);
  
  // Append note text to notes section.
  html = '<p class="' + style + '"><a href="#citation' + noteId + '" id="note' + noteId + '">' + caller + '</a><span> </span><span>+ </span><span> </span></p>';
  $ ("#notes").append (html);
  
  oneverseInsertedNotesCount++;
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
 
Section for navigating to another passage.
 
*/


function oneVerseHtmlClicked (event)
{
  // If the user selects text, do nothing.
  var text = "";
  if (window.getSelection) {
    text = window.getSelection().toString();
  } else if (document.selection && document.selection.type != "Control") {
    text = document.selection.createRange().text;
  }
  if (text.length) return;
  
  var verse = "";
  
  var iterations = 0;
  var target = $(event.target);
  var tagName = target.prop("tagName");
  if (tagName == "P") target = $ (target.children ().last ());
  while ((iterations < 10) && (!target.hasClass ("v"))) {
    var previous = $(target.prev ());
    if (previous.length == 0) {
      target = $ (target.parent ().prev ());
      target = $ (target.children ().last ());
    } else {
      target = previous;
    }
    iterations++;
  }
  
  if (target.length == 0) verse = "0";
  
  if (target.hasClass ("v")) {
    verse = target[0].innerText;
  }

  $.ajax ({
    url: "verse",
    type: "GET",
    data: { verse: verse },
    cache: false
  });
}


/*
 
Section for swipe navigation.
 
*/


function oneverseSwipeLeft (event)
{
  if (typeof navigateNextVerse != 'undefined') {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function oneverseSwipeRight (event)
{
  if (typeof navigatePreviousVerse != 'undefined') {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}
