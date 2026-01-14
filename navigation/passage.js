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


var navigationBible;
var navigationBook;
var navigationChapter;
var navigationVerse;


var navigatorContainer;
var navigatorTimeout;


document.addEventListener("DOMContentLoaded", function(e) {
  navigatorContainer = document.querySelector ("#versepickerwrapper");
  buildMouseNavigator ();
  navigationPollPassage ();
});


function buildKeyboardNavigator () {
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["keyboard", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer = document.querySelector ("#versepickerwrapper");
    if (!navigatorContainer) {
      navigatorContainer = parent.document.document.querySelector ("#versepickerwrapper");
    }
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    var keyboard = document.querySelector ("#keyboard");
    if (!keyboard)
      keyboard = parent.document.querySelector ("#keyboard");
    keyboard.focus ();
    keyboard.addEventListener('keydown', keyboardNavigatorEnter);
  })
  .catch((error) => {
    console.log(error);
  })
}


function keyboardNavigatorEnter (event) {
  if (event.keyCode == 13) {
    passage = document.querySelector("#keyboard").value;
    const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["passage", passage], ["focusgroup", focusGroup] ]).toString();
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
      navigatorContainer.innerHTML = "";
      navigatorContainer.insertAdjacentHTML('beforeend', response);
      bindClickHandlers ();
      navigationPollPassage ();
    })
    .catch((error) => {
      console.log(error);
    })
    return false;
  }
}


function buildMouseNavigator () {
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    bindClickHandlers ();
  })
  .catch((error) => {
    console.log(error);
  })
}


var navigateBackTimer;
function startNavigateBackLongPress(event) {
  navigateBackTimer = setTimeout(function(event) {
    historyBack (event);
  },1000);
}
function stopNavigateBackLongPress(event) {
  clearTimeout(navigateBackTimer);
}


var navigateForwardTimer;
function startNavigateForwardLongPress(event) {
  navigateForwardTimer = setTimeout(function(event) {
    historyForward (event);
  },1000);
}
function stopNavigateForwardLongPress(event) {
  clearTimeout(navigateForwardTimer);
}


function bindClickHandlers () {
  var navigateback = document.querySelector("#navigateback");
  if (navigateback) {
    navigateback.addEventListener ("click", navigateBack);
    navigateback.addEventListener ("mousedown", startNavigateBackLongPress);
    navigateback.addEventListener ("mouseup", stopNavigateBackLongPress);
    navigateback.addEventListener ("touchstart", startNavigateBackLongPress);
    navigateback.addEventListener ("touchend", stopNavigateBackLongPress);
    navigateback.addEventListener ("contextmenu", function(event) {
      event.preventDefault();
    });
  }
  var navigateforward = document.querySelector("#navigateforward");
  if (navigateforward) {
    navigateforward.addEventListener ("click", navigateForward);
    navigateforward.addEventListener ("mousedown", startNavigateForwardLongPress);
    navigateforward.addEventListener ("mouseup", stopNavigateForwardLongPress);
    navigateforward.addEventListener ("touchstart", startNavigateForwardLongPress);
    navigateforward.addEventListener ("touchend", stopNavigateForwardLongPress);
    navigateforward.addEventListener ("contextmenu", function(event) {
      event.preventDefault();
    });
  }
  var previousbook = document.querySelector("#previousbook");
  if (previousbook) {
    previousbook.addEventListener ("click", navigatePreviousBook);
  }
  var selectbook = document.querySelector("#selectbook");
  if (selectbook) {
    selectbook.addEventListener ("click", function (event) {
      var fadeout = document.querySelector(".fadeout");
      if (fadeout)
        fadeout.hidden = true;
      var topbar = document.querySelector('#topbar');
      if (topbar)
        topbar.classList.add('wrap-active');
      displayBooks (event);
    });
  }
  var nextbook = document.querySelector("#nextbook");
  if (nextbook) {
    nextbook.addEventListener ("click", navigateNextBook);
  }
  var previouschapter = document.querySelector("#previouschapter");
  if (previouschapter) {
    previouschapter.addEventListener ("click", navigatePreviousChapter);
  }
  var selectchapter = document.querySelector("#selectchapter");
  if (selectchapter) {
    selectchapter.addEventListener ("click", function (event) {
      var fadeout = document.querySelector(".fadeout");
      if (fadeout)
        fadeout.hidden = true;
      displayChapters (event);
    });
  }
  var nextchapter = document.querySelector("#nextchapter");
  if (nextchapter) {
    nextchapter.addEventListener ("click", navigateNextChapter);
  }
  var previousverse = document.querySelector("#previousverse");
  if (previousverse) {
    previousverse.addEventListener ("click", navigatePreviousVerse);
  }
  var selectverse = document.querySelector("#selectverse");
  if (selectverse) {
    selectverse.addEventListener ("click", function (event) {
      var fadeout = document.querySelector(".fadeout");
      if (fadeout)
        fadeout.hidden = true;
      displayVerses (event);
    });
  }
  var nextverse = document.querySelector("#nextverse");
  if (nextverse) {
    nextverse.addEventListener ("click", navigateNextVerse);
  }
}



var navigateBackSkip = false;

function navigateBack (event) {
  if (navigateBackSkip) {
    navigateBackSkip = false;
    return;
  }
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["goback", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    bindClickHandlers ();
    navigationPollPassage ();
  })
  .catch((error) => {
    console.log(error);
  });
}


var navigateForwardSkip = false;

function navigateForward (event) {
  if (navigateForwardSkip) {
    navigateForwardSkip = false;
    return;
  }
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["goforward", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    bindClickHandlers ();
    navigationPollPassage ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function displayBooks (event) {
  event.preventDefault ();
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["getbooks", true], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    document.querySelector("#applybook").addEventListener ("click", function (event) {
      document.querySelector('#topbar').classList.remove('wrap-active');
      applyBook (event);
    });
  })
  .catch((error) => {
    console.log(error);
  })
}


function applyBook (event) {
  event.preventDefault ();
  if (event.target.localName == "a") {
    const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["applybook", event.target.id], ["focusgroup", focusGroup] ]).toString();
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
      navigatorContainer.innerHTML = "";
      navigatorContainer.insertAdjacentHTML('beforeend', response);
      bindClickHandlers ();
      navigationPollPassage ();
    })
    .catch((error) => {
      console.log(error);
    })
  }
}


function displayChapters (event) {
  event.preventDefault ();
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["book", navigationBook], ["getchapters", true], ["chapter", navigationChapter], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    document.querySelectorAll("#applychapter").forEach((element) => {
      element.addEventListener("click", applyChapter);
    });
  })
  .catch((error) => {
    console.log(error);
  })
}


function applyChapter (event) {
  event.preventDefault ();
  if (event.target.localName == "a") {
    const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["applychapter", event.target.id], ["focusgroup", focusGroup] ]).toString();
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
      navigatorContainer.innerHTML = "";
      navigatorContainer.insertAdjacentHTML('beforeend', response);
      bindClickHandlers ();
      navigationPollPassage ();
    })
    .catch((error) => {
      console.log(error);
    })
  }
}


function displayVerses (event) {
  event.preventDefault ();
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["book", navigationBook], ["chapter", navigationChapter], ["verse", navigationVerse], ["getverses", true], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    document.querySelectorAll("#applyverse").forEach((element) => {
      element.addEventListener("click", applyVerse);
    });
  })
  .catch((error) => {
    console.log(error);
  })
}


function applyVerse (event) {
  event.preventDefault ();
  if (event.target.localName == "a") {
    const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["applyverse", event.target.id], ["focusgroup", focusGroup] ]).toString();
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
      navigatorContainer.innerHTML = "";
      navigatorContainer.insertAdjacentHTML('beforeend', response);
      bindClickHandlers ();
      navigationPollPassage ();
    })
    .catch((error) => {
      console.log(error);
    })
  }
}


function navigatePreviousVerse (event) {
  event.preventDefault ();
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["previousverse", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    bindClickHandlers ();
    navigationPollPassage ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function navigateNextVerse (event) {
  event.preventDefault ();
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["nextverse", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    bindClickHandlers ();
    navigationPollPassage ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function navigatePreviousChapter (event) {
  event.preventDefault ();
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["previouschapter", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    bindClickHandlers ();
    navigationPollPassage ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function navigateNextChapter (event) {
  event.preventDefault ();
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["nextchapter", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    bindClickHandlers ();
    navigationPollPassage ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function navigatePreviousBook (event) {
  event.preventDefault ();
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["previousbook", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    bindClickHandlers ();
    navigationPollPassage ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function navigateNextBook (event) {
  event.preventDefault ();
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["nextbook", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    bindClickHandlers ();
    navigationPollPassage ();
  })
  .catch((error) => {
    console.log(error);
  })
}


function navigationPollPassage ()
{
  if (navigatorTimeout) {
    clearTimeout (navigatorTimeout);
  }
  const url = "/navigation/poll?focusgroup=" + focusGroup;
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
  })
  .catch((error) => {
    console.log(error);
  })
  .finally(() => {
    navigatorTimeout = setTimeout (navigationPollPassage, 1000);
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
  document.querySelectorAll("iframe").forEach((element) => {
    try {
      element.contentWindow.navigationNewPassage ();
    }
    catch (err) {
    }
  })
}


function historyForward (event) {
  // After the long press event, if releasing the mouse, it will fire a click event.
  // Set a flag to not handle the click event.
  navigateForwardSkip = true;
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["historyforward", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    document.querySelectorAll("#applyhistory").forEach((element) => {
      element.addEventListener ("click", applyHistory);
    })
  })
  .catch((error) => {
    console.log(error);
  })
}


function historyBack (event) {
  // After the long press event, if releasing the mouse, it will fire a click event.
  // Set a flag to not handle the click event.
  navigateBackSkip = true;
  const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["historyback", ""], ["focusgroup", focusGroup] ]).toString();
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
    navigatorContainer.innerHTML = "";
    navigatorContainer.insertAdjacentHTML('beforeend', response);
    document.querySelectorAll("#applyhistory").forEach((element) => {
      element.addEventListener ("click", applyHistory);
    })
  })
  .catch((error) => {
    console.log(error);
  })
}


function applyHistory (event) {
  event.preventDefault ();
  if (event.target.localName == "a") {
    const url = "/navigation/update?" + new URLSearchParams([ ["bible", navigationBible], ["applyhistory", event.target.id], ["focusgroup", focusGroup] ]).toString();
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
      navigatorContainer.innerHTML = "";
      navigatorContainer.insertAdjacentHTML('beforeend', response);
      bindClickHandlers ();
      navigationPollPassage ();
    })
    .catch((error) => {
      console.log(error);
    })
  }
}

