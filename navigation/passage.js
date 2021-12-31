/*
Copyright (©) 2003-2022 Teus Benschop.

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


var navigationBible;
var navigationBook;
var navigationChapter;
var navigationVerse;


var navigatorContainer;
var navigatorTimeout;


$(document).ready (function () {
  navigatorContainer = $ ("#versepickerwrapper");
  buildMouseNavigator ();
  navigationPollPassage ();
  $ ("body").on('keydown', "#keyboard", keyboardNavigatorEnter);
});


function buildKeyboardNavigator () {
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, keyboard: "" },
    cache: false,
    success: function (response) {
      navigatorContainer = $ ("#versepickerwrapper");
      if (navigatorContainer.length == 0) {
        navigatorContainer = $(parent.document).find ("#versepickerwrapper");
      }
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      var keyboard = $ ("#keyboard");
      if (keyboard.length == 0) keyboard = $(parent.document).find ("#keyboard");
      keyboard.focus ();
    },
  });
}


function keyboardNavigatorEnter (event) {
  if (event.keyCode == 13) {
    passage = $ ("#keyboard").val();
    $.ajax ({
      url: "/navigation/update",
      type: "GET",
      data: { bible: navigationBible, passage: passage },
      cache: false,
      success: function (response) {
        navigatorContainer.empty ();
        navigatorContainer.append (response);
        bindClickHandlers ();
        navigationPollPassage ();
      },
    });
    return false;
  }
}


function buildMouseNavigator () {
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      bindClickHandlers ();
    },
  });
}


function bindClickHandlers () {
  $("#navigateback").on ("click", function (event) {
    navigateBack (event);
  });
  $("#navigateback").longpress (function (event) {
    historyBack (event);
  });
  $("#navigateback").on ("contextmenu", function (event) {
    event.preventDefault();
  });
  $("#navigateforward").on ("click", function (event) {
    navigateForward (event);
  });
  $("#navigateforward").longpress (function (event) {
    historyForward (event);
  });
  $("#navigateforward").on ("contextmenu", function (event) {
    event.preventDefault();
  });
  $("#selectbook").on ("click", function (event) {
    $ (".fadeout").hide ();
    $ ('#topbar').addClass('wrap-active');
    displayBooks (event);
  });
  $("#selectchapter").on ("click", function (event) {
    $ (".fadeout").hide ();
    displayChapters (event);
  });
  $("#previousverse").on ("click", function (event) {
    navigatePreviousVerse (event);
  });
  $("#selectverse").on ("click", function (event) {
    $ (".fadeout").hide ();
    displayVerses (event);
  });
  $("#nextverse").on ("click", function (event) {
    navigateNextVerse (event);
  });
}



var navigateBackSkip = false;

function navigateBack (event) {
  if (navigateBackSkip) {
    navigateBackSkip = false;
    return;
  }
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, goback: "" },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      bindClickHandlers ();
      navigationPollPassage ();
    },
  });
}


var navigateForwardSkip = false;

function navigateForward (event) {
  if (navigateForwardSkip) {
    navigateForwardSkip = false;
    return;
  }
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, goforward: "" },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      bindClickHandlers ();
      navigationPollPassage ();
    },
  });
}


function displayBooks (event) {
  event.preventDefault ();
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, getbooks: true },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      $("#applybook").on ("click", function (event) {
        $ ('#topbar').removeClass('wrap-active');
        applyBook (event);
      });
    },
  });
}


function applyBook (event) {
  event.preventDefault ();
  if (event.target.localName == "a") {
    $.ajax ({
      url: "/navigation/update",
      type: "GET",
      data: { bible: navigationBible, applybook: event.target.id },
      cache: false,
      success: function (response) {
        navigatorContainer.empty ();
        navigatorContainer.append (response);
        bindClickHandlers ();
        navigationPollPassage ();
      },
    });
  }
}


function displayChapters (event) {
  event.preventDefault ();
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, book: navigationBook, getchapters: true, chapter: navigationChapter },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      $("#applychapter").on ("click", function (event) {
        applyChapter (event);
      });
    },
  });
}


function applyChapter (event) {
  event.preventDefault ();
  if (event.target.localName == "a") {
    $.ajax ({
      url: "/navigation/update",
      type: "GET",
      data: { bible: navigationBible, applychapter: event.target.id },
      cache: false,
      success: function (response) {
        navigatorContainer.empty ();
        navigatorContainer.append (response);
        bindClickHandlers ();
        navigationPollPassage ();
      },
    });
  }
}


function displayVerses (event) {
  event.preventDefault ();
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, book: navigationBook, chapter: navigationChapter, verse: navigationVerse, getverses: true },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      $("#applyverse").on ("click", function (event) {
        applyVerse (event);
      });
    },
  });
}


function applyVerse (event) {
  event.preventDefault ();
  if (event.target.localName == "a") {
    $.ajax ({
      url: "/navigation/update",
      type: "GET",
      data: { bible: navigationBible, applyverse: event.target.id },
      cache: false,
      success: function (response) {
        navigatorContainer.empty ();
        navigatorContainer.append (response);
        bindClickHandlers ();
        navigationPollPassage ();
      },
    });
  }
}


function navigatePreviousVerse (event) {
  event.preventDefault ();
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, previousverse: "" },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      bindClickHandlers ();
      navigationPollPassage ();
    },
  });
}


function navigateNextVerse (event) {
  event.preventDefault ();
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, nextverse: "" },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      bindClickHandlers ();
      navigationPollPassage ();
    },
  });
}


function navigatePreviousChapter (event) {
  event.preventDefault ();
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, previouschapter: "" },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      bindClickHandlers ();
      navigationPollPassage ();
    },
  });
}


function navigateNextChapter (event) {
  event.preventDefault ();
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, nextchapter: "" },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      bindClickHandlers ();
      navigationPollPassage ();
    },
  });
}


function navigationPollPassage ()
{
  if (navigatorTimeout) {
    clearTimeout (navigatorTimeout);
  }
  $.ajax ({
    url: "/navigation/poll",
    type: "GET",
    cache: false,
    success: function (response) {
      var ref = response.split ("\n");
      var book = ref [0];
      var chapter = ref [1];
      var verse = ref [2];
      if ((book != navigationBook) || (chapter != navigationChapter) || (verse != navigationVerse)) {
        navigationBook = book;
        navigationChapter = chapter;
        navigationVerse = verse;
        navigationCallNewPassage ();
        buildMouseNavigator ();
      }
    },
    complete: function (xhr, status) {
      navigatorTimeout = setTimeout (navigationPollPassage, 1000);
    }
  });
}


function navigationHandleKeyDown (event)
{
  // Ctrl-G
  if ((event.ctrlKey == true) && (event.keyCode == 71)) {
    event.preventDefault ();
    buildKeyboardNavigator ();
  }
  // Escape
  if (event.keyCode == 27) {
    event.preventDefault ();
    buildMouseNavigator ();
  }
  // Alt-Left Arrow
  if ((event.altKey == true) && (event.keyCode == 37)) {
    event.preventDefault ();
    navigateBack (event);
  }
  // Alt-Right Arrow
  if ((event.altKey == true) && (event.keyCode == 39)) {
    event.preventDefault ();
    navigateForward (event);
  }
  // Alt-Down Arrow
  if ((event.altKey == true) && (event.keyCode == 40)) {
    event.preventDefault ();
    navigateNextVerse (event);
  }
  // Alt-Up Arrow
  if ((event.altKey == true) && (event.keyCode == 38)) {
    event.preventDefault ();
    navigatePreviousVerse (event);
  }
}


function navigationCallNewPassage () {
  try  {
    navigationNewPassage ();
  }
  catch (err) {
  }
  $ ("iframe").each (function (index) {
    try {
      $ (this)[0].contentWindow.navigationNewPassage ();
    }
    catch (err) {
    }
  });
}


function historyForward (event) {
  // After the long press event, if releasing the mouse, it will fire a click event.
  // Set a flag to not handle the click event.
  navigateForwardSkip = true;
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, historyforward: "" },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      $("#applyhistory").on ("click", function (event) {
        applyHistory (event);
      });
    },
  });
}


function historyBack (event) {
  // After the long press event, if releasing the mouse, it will fire a click event.
  // Set a flag to not handle the click event.
  navigateBackSkip = true;
  $.ajax ({
    url: "/navigation/update",
    type: "GET",
    data: { bible: navigationBible, historyback: "" },
    cache: false,
    success: function (response) {
      navigatorContainer.empty ();
      navigatorContainer.append (response);
      $("#applyhistory").on ("click", function (event) {
        applyHistory (event);
      });
    },
  });
}


function applyHistory (event) {
  event.preventDefault ();
  if (event.target.localName == "a") {
    $.ajax ({
      url: "/navigation/update",
      type: "GET",
      data: { bible: navigationBible, applyhistory: event.target.id },
      cache: false,
      success: function (response) {
        navigatorContainer.empty ();
        navigatorContainer.append (response);
        bindClickHandlers ();
        navigationPollPassage ();
      },
    });
  }
}

