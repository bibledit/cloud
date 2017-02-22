/*
Copyright (©) 2003-2016 Teus Benschop.

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


$(document).ready (function () {
  navigationNewPassage ();
  $ ("#usfmverseeditor").on ("paste cut keydown", verseEditorChanged);
  $ (window).on ("unload", verseEditorUnload);
  $ ("#usfmverseeditor").on ("paste", function (e) {
    var data = e.originalEvent.clipboardData.getData ('Text');
    e.preventDefault();
    document.execCommand ("insertHTML", false, data);
  });
  verseIdPoller ();
  if (swipe_operations) {
    $ ("body").swipe ( {
      swipeLeft:function (event, direction, distance, duration, fingerCount, fingerData) {
        editverseSwipeLeft (event);
      },
      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
        editverseSwipeRight (event);
      }
    });
  }
  if (unsentBibleDataTimeoutWarning) notifyItError (unsentBibleDataTimeoutWarning);
});


var verseBible;
var verseBook;
var verseNavigationBook;
var verseChapter;
var verseNavigationChapter;
var verseVerse;
var verseNavigationVerse;
var verseVerseLoading;
var verseVerseLoaded;
var verseEditorChangedTimeout;
var verseLoadedText;
var verseIdChapter = 0;
var verseIdTimeout;
var verseReload = false;
var verseEditorTextChanged = false;
var verseSaveAsync;
var verseLoadAjaxRequest;
var verseSaving = false;


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
    verseNavigationBook = navigationBook;
    verseNavigationChapter = navigationChapter;
    verseNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    verseNavigationBook = parent.window.navigationBook;
    verseNavigationChapter = parent.window.navigationChapter;
    verseNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }
  verseEditorSaveChapter ();
  verseReload = false;
  verseEditorLoadChapter ();
}


function verseEditorLoadChapter ()
{
  if ((verseNavigationBook != verseBook) || (verseNavigationChapter != verseChapter) || (verseNavigationVerse != verseVerse) || verseReload) {
    verseBible = navigationBible;
    verseBook = verseNavigationBook;
    verseChapter = verseNavigationChapter;
    verseVerse = verseNavigationVerse;
    verseVerseLoading = verseNavigationVerse;
    verseIdChapter = 0;
    if (verseEditorFocused) verseFocus ();
    if (verseLoadAjaxRequest && verseLoadAjaxRequest.readystate != 4) {
      verseLoadAjaxRequest.abort();
    }
    verseEditorFocused = verseFocused ();
    verseLoadAjaxRequest = $.ajax ({
      url: "load",
      type: "GET",
      data: { bible: verseBible, book: verseBook, chapter: verseChapter, verse: verseVerseLoading },
      success: function (response) {
        verseEditorWriteAccess = checksum_readwrite (response);
        // Checksumming.
        response = checksum_receive (response);
        if (response !== false) {
          if (response != verseLoadedText) {
            $ ("#usfmverseeditor").empty ();
            $ ("#usfmverseeditor").append (response);
            verseEditorStatus (verseEditorVerseLoaded);
            verseLoadedText = response;
            verseVerseLoaded = verseVerseLoading;
            verseReload = false;
            if (verseEditorFocused || verseInitialFocus) verseFocus ();
            verseInitialFocus = false;
          }
        } else {
          // Checksum error: Reload.
          verseReload = true;
          verseEditorLoadChapter ();
        }
      },
    });
  }
}


function verseEditorUnload ()
{
  verseEditorSaveChapter (true);
}


function verseEditorSaveChapter (sync)
{
  if (verseSaving) {
    verseEditorChanged ();
    return;
  }
  if (!verseEditorWriteAccess) return;
  verseEditorTextChanged = false;
  if (!verseBible) return;
  if (!verseBook) return;
  if (!verseVerseLoaded) return;
  var usfm = $ ("#usfmverseeditor").html ();
  if (usfm == verseLoadedText) return;
  if (!usfm) return;
  verseEditorStatus (verseEditorVerseSaving);
  verseLoadedText = usfm;
  verseIdChapter = 0;
  verseSaveAsync = true;
  if (sync) verseSaveAsync = false;
  var encodedUsfm = filter_url_plus_to_tag (usfm);
  var checksum = checksum_get (encodedUsfm);
  verseSaving = true;
  $.ajax ({
    url: "save",
    type: "POST",
    async: verseSaveAsync,
    data: { bible: verseBible, book: verseBook, chapter: verseChapter, verse: verseVerseLoaded, usfm: encodedUsfm, checksum: checksum },
    error: function (jqXHR, textStatus, errorThrown) {
      verseEditorStatus (verseEditorChapterRetrying);
      verseLoadedText = "";
      verseEditorChanged ();
      if (!verseSaveAsync) verseEditorSaveChapter (true);
    },
    success: function (response) {
      verseEditorStatus (response);
    },
    complete: function (xhr, status) {
      verseSaveAsync = true;
      verseSaving = false;
    }
  });
}


function verseEditorChanged (event)
{
  if (editKeysIgnoreForContentChange (event)) return;
  verseEditorTextChanged = true;
  if (verseEditorChangedTimeout) {
    clearTimeout (verseEditorChangedTimeout);
  }
  verseEditorChangedTimeout = setTimeout (verseEditorSaveChapter, 1000);
}


function verseEditorStatus (text)
{
  $ ("#usfmstatus").empty ();
  $ ("#usfmstatus").append (text);
  verseEditorSelectiveNotification (text);
}


function verseEditorSelectiveNotification (message)
{
  if (message == verseEditorVerseLoaded) return;
  if (message == verseEditorVerseSaving) return;
  if (message == verseEditorVerseSaved) return;
  notifyItError (message);
}


function verseIdPoller ()
{
  if (verseIdTimeout) {
    clearTimeout (verseIdTimeout);
  }
  verseIdTimeout = setTimeout (verseEditorPollId, 1000);
}


function verseEditorPollId ()
{
  $.ajax ({
    url: "../edit/id",
    type: "GET",
    data: { bible: verseBible, book: verseBook, chapter: verseChapter },
    cache: false,
    success: function (response) {
      if (!verseSaving) {
        if (verseIdChapter != 0) {
          if (response != verseIdChapter) {
            verseReload = true;
            verseEditorLoadChapter ();
            verseIdChapter = 0;
          }
        }
        verseIdChapter = response;
      }
    },
    complete: function (xhr, status) {
      verseIdPoller ();
    }
  });
}


var verseInitialFocus = true;
var verseEditorFocused = true;

function verseFocused ()
{
  return $ ("span[contenteditable]").is (":focus");
}

function verseFocus ()
{
  $ ("span[contenteditable]").focus ();
}


function editverseSwipeLeft (event)
{
  if (typeof navigateNextVerse != 'undefined') {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function editverseSwipeRight (event)
{
  if (typeof navigatePreviousVerse != 'undefined') {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}
