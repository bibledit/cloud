/*
Copyright (©) 2003-2024 Teus Benschop.

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
var verseReaderUniqueID = Math.floor (Math.random() * 100000000);


$ (document).ready (function ()
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

  visualVerseReaderInitializeOnce ();
  visualVerseReaderInitializeLoad ();

  navigationNewPassage ();
  
  if (swipe_operations) {
    $ ("body").swipe ( {
      swipeLeft:function (event, direction, distance, duration, fingerCount, fingerData) {
        readchooseSwipeLeft (event);
      },
      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
        readchooseSwipeRight (event);
      }
    });
  }
  
  setTimeout (readverseCoordinatingTimeout, 500);

});


function visualVerseReaderInitializeOnce ()
{
  var Parchment = Quill.import ('parchment');
  
  // Register block formatting class.
  var ParagraphClass = new Parchment.Attributor.Class ('paragraph', 'b', { scope: Parchment.Scope.BLOCK });
  Quill.register (ParagraphClass, true);
  
  // Register inline formatting class.
  var CharacterClass = new Parchment.Attributor.Class ('character', 'i', { scope: Parchment.Scope.INLINE });
  Quill.register (CharacterClass, true);
}


function visualVerseReaderInitializeLoad ()
{
  // Work around https://github.com/quilljs/quill/issues/1116
  // It sets the margins to 0 by adding an overriding class.
  // The Quill editor will remove that class again.
  $ ("#oneeditor > p").each (function (index) {
    var element = $(this);
    element.addClass ("nomargins");
  });
  
  // Instantiate editor.
  quill = new Quill ('#oneeditor', { });
}


var readchooseBible;
var readchooseBook;
var readchooseNavigationBook;
var readchooseChapter;
var readchooseNavigationChapter;
var readchooseVerse;
var readchooseNavigationVerse;
var readchooseVerseLoading;
var readchooseVerseLoaded;
var readchooseChapterId = 0;
var readchooseLoadAjaxRequest;


//
//
// Section for the new Passage event from the Navigator.
//
//


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
    readchooseNavigationBook = navigationBook;
    readchooseNavigationChapter = navigationChapter;
    readchooseNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    readchooseNavigationBook = parent.window.navigationBook;
    readchooseNavigationChapter = parent.window.navigationChapter;
    readchooseNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }
  readchooseEditorLoadVerse ();
}


//
//
// Section for editor load and save.
//
//


function readchooseEditorLoadVerse ()
{
  if ((readchooseNavigationBook != readchooseBook) || (readchooseNavigationChapter != readchooseChapter) || (readchooseNavigationVerse != readchooseVerse) ) {
    publicFeedbackLoadNotesInRead ();
    readchooseBible = navigationBible;
    readchooseBook = readchooseNavigationBook;
    readchooseChapter = readchooseNavigationChapter;
    readchooseVerse = readchooseNavigationVerse;
    readchooseVerseLoading = readchooseNavigationVerse;
    readchooseChapterId = 0;
    if (readchooseLoadAjaxRequest && readchooseLoadAjaxRequest.readystate != 4) {
      readchooseLoadAjaxRequest.abort();
    }
    readchooseLoadAjaxRequest = $.ajax ({
      url: "load",
      type: "GET",
      data: { bible: readchooseBible, book: readchooseBook, chapter: readchooseChapter, verse: readchooseVerseLoading, id: verseReaderUniqueID },
      success: function (response) {
        checksum_readwrite (response);
        // Checksumming.
        response = checksum_receive (response);
        // Splitting.
        var bits;
        if (response !== false) {
          bits = response.split ("#_be_#");
          if (bits.length != 3) response == false;
        }
        if (response !== false) {
          $ ("#oneprefix").empty ();
          $ ("#oneprefix").append (bits [0]);
          $ ("#oneprefix").off ("click");
          $ ("#oneprefix").on ("click", readchooseHtmlClicked);
        }
        if (response !== false) {
          // Destroy existing editor.
          if (quill) delete quill;
          // Load the html in the DOM.
          $ ("#oneeditor").empty ();
          $ ("#oneeditor").append (bits [1]);
          readchooseVerseLoaded = readchooseVerseLoading;
          readchooseEditorStatus (readchooseEditorVerseLoaded);
          // Create the editor based on the DOM's content.
          visualVerseReaderInitializeLoad ();
          quill.enable (false);
          // Create CSS for embedded styles.
          css4embeddedstyles ();
        }
        if (response !== false) {
          $ ("#onesuffix").empty ();
          $ ("#onesuffix").append (bits [2]);
          $ ("#onesuffix").off ("click");
          $ ("#onesuffix").on ("click", readchooseHtmlClicked);
        }
        if (response !== false) {
          readchooseScrollVerseIntoView ();
        }
        if (response === false) {
          // Checksum or other error: Reload.
          readchooseEditorLoadVerse ();
        }
      },
    });
  }
}


function readchooseEditorLoadNonEditable ()
{
  readchooseAjaxActive = true;
  $.ajax ({
    url: "load",
    type: "GET",
    data: { bible: readchooseBible, book: readchooseBook, chapter: readchooseChapter, verse: readchooseVerseLoading, id: verseReaderUniqueID },
    success: function (response) {
      // Remove the flag for editor read-write or read-only.
      checksum_readwrite (response);
      // Checksumming.
      response = checksum_receive (response);
      // Splitting.
      if (response !== false) {
        var bits;
        bits = response.split ("#_be_#");
        if (bits.length != 3) response == false;
        $ ("#oneprefix").empty ();
        $ ("#oneprefix").append (bits [0]);
        $ ("#oneprefix").off ("click");
        $ ("#oneprefix").on ("click", readchooseHtmlClicked);
        $ ("#onesuffix").empty ();
        $ ("#onesuffix").append (bits [2]);
        $ ("#onesuffix").off ("click");
        $ ("#onesuffix").on ("click", readchooseHtmlClicked);
      }
    },
    complete: function (xhr, status) {
      readchooseAjaxActive = false;
    }
  });
}


//
//
// Section for user interface updates and feedback.
//
//


function readchooseEditorStatus (text)
{
  if (document.body.contains(document.getElementById("onestatus"))) {
    $ ("#onestatus").empty ();
    $ ("#onestatus").append (text);
  }
}


//
//
// Section for polling the server for updates on the loaded chapter.
//
//


var readchooseIdAjaxRequest;


function readchooseEditorPollId ()
{
  readchooseAjaxActive = true;
  readchooseIdAjaxRequest = $.ajax ({
    url: "../editor/id",
    type: "GET",
    data: { bible: readchooseBible, book: readchooseBook, chapter: readchooseChapter },
    cache: false,
    success: function (response) {
      if (readchooseChapterId != 0) {
        if (response != readchooseChapterId) {
          // The chapter identifier changed.
          // That means that likely there's updated text on the server.
          // Start the routine to load any possible updates into the editor.
          readchooseUpdateTrigger = true;
          readchooseReloadNonEditableFlag = true;
          
        }
      }
      readchooseChapterId = response;
    },
    error: function (jqXHR, textStatus, errorThrown) {
    },
    complete: function (xhr, status) {
      readchooseAjaxActive = false;
    }
  });
}

//
//
// Section for scrolling the editable portion into the center.
//
//


function readchooseScrollVerseIntoView ()
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


//
//
// Section for navigating to another passage.
//
//


function readchooseHtmlClicked (event)
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
  while ((iterations < 10) && (!target.hasClass ("i-v"))) {
    var previous = $(target.prev ());
    if (previous.length == 0) {
      target = $ (target.parent ().prev ());
      target = $ (target.children ().last ());
    } else {
      target = previous;
    }
    iterations++;
  }
                                          
  // Too many iterations: Undefined location.
  if (iterations >= 10) return
  
  if (target.length == 0) verse = "0";
  
  if (target.hasClass ("i-v")) {
    verse = target[0].innerText;
  }

  $.ajax ({
    url: "verse",
    type: "GET",
    data: { verse: verse },
    cache: false
  });

  // It used to go to the Resources, but it was disabled later.
  // https://github.com/bibledit/cloud/issues/508
  // setTimeout (readChooseGotoResources, 1000);
}


function readChooseGotoResources ()
{
  window.location.href = '../resource/index';
}


//
//
// Section for swipe navigation.
//
//


function readchooseSwipeLeft (event)
{
  if (typeof navigateNextVerse != 'undefined') {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function readchooseSwipeRight (event)
{
  if (typeof navigatePreviousVerse != 'undefined') {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
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


var readchooseAjaxActive = false;
var readchoosePollSelector = 0;
var readchoosePollDate = new Date();
var readchooseUpdateTrigger = false;
var readchooseReloadNonEditableFlag = false;


function readverseCoordinatingTimeout ()
{
  // Handle situation that an AJAX call is ongoing.
  if (readchooseAjaxActive) {
    
  }
  else if (readchooseUpdateTrigger) {
    readchooseUpdateTrigger = false;
    readchooseUpdateExecute ();
  }
  else if (readchooseReloadNonEditableFlag) {
    readchooseReloadNonEditableFlag = false;
    readchooseEditorLoadNonEditable ();
  }
  // Handle situation that no process is ongoing.
  // Now the regular pollers can run again.
  else {
    // There are two regular pollers.
    // Wait 500 ms, then start one of the pollers.
    // So each poller runs once a second.
    var difference = new Date () - readchoosePollDate;
    if (difference > 500) {
      readchoosePollSelector++;
      if (readchoosePollSelector > 1) readchoosePollSelector = 0;
      if (readchoosePollSelector == 0) {
        readchooseEditorPollId ();
      }
      if (readchoosePollSelector == 1) {

      }
      readchoosePollDate = new Date();
    }
  }
  setTimeout (readverseCoordinatingTimeout, 100);
}


//
//
// Indonesian Cloud Free
// Section for handling public feedbacks.
//
//


function publicFeedbackLoadNotesInRead ()
{
  $.ajax ({
    url: "/public/notes",
    type: "GET",
    data: { bible: readchooseBible, book: readchooseBook, chapter: readchooseChapter },
    success: function (response) {
      console.log(response);
      $ ("#publicnotesinread").empty ();
      $ ("#publicnotesinread").append (response);
    },
  });
}
