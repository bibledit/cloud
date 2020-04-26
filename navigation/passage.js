/*
Copyright (©) 2003-2020 Teus Benschop.

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
  buildNavigator ();
  navigationPollPassage ();
});


function buildNavigator () {
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
  $("#navigateforward").on ("click", function (event) {
    navigateForward (event);
  });
  $("#selectbook").on ("click", function (event) {
    $ (".fadeout").hide ();
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


function navigateBack (event) {
  event.preventDefault ();
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


function navigateForward (event) {
  event.preventDefault ();
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
        buildNavigator ();
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
    $("#selectbook").focus();
  }
  // Escape
  if (event.keyCode == 27) {
    event.preventDefault ();
    buildNavigator ();
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
