/*
Copyright (©) 2003-2025 Teus Benschop.

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


function topbarRemovalQueryAddition (elementsAttribute) {
  if (/topbar/.test(elementsAttribute) === false) {
    if (/\?/.test(elementsAttribute)) {
      return elementsAttribute + "&topbar=0";
    } else if (/\?/.test(elementsAttribute) === false &&
    /topbar/.test(elementsAttribute) === false) {
      return elementsAttribute + "?topbar=0";
    }
  } else {
    return elementsAttribute;
  }
}


$ (document).ready (function () {
  if (window.self === window.top) {
    // On main page: Enable menu on touch screen.
    $ ('.toggle').click (function () {
      var hovered = $ (this).hasClass ('hover');
      $ ('.hover').removeClass('hover');
      if (!hovered) {
        $ (this).addClass ('hover');
      }
    });
    // Deal with the optional html back button.
    $ ('#backbutton').click (function () {
      window.history.back();
    });
  } else {
    // In workspace iframe: Remove possible top bar.
    // The topbar is removed by the server via the Workspace.
    // But in other cases the topbar makes it to the browser,
    // and is to be removed here within the iframe.
    // (1) By adding the query to each anchor element's href
    // attribute and form element's action attribute.
    // The function is declared outside $ (document).ready
    // so that it can be called from other JavaScript files.
    // (2) By emptying the actual topbar.
    document.querySelectorAll('a').forEach((element) => {
      if (element.id !== "public-feedback") {
        element.href = topbarRemovalQueryAddition (element.href);
      }
    })
    document.querySelectorAll('form').forEach((element) => {
      element.action = topbarRemovalQueryAddition (element.action);
    })
    $ ('#topbar').empty ();
  };
  if (typeof (fadingMenuDelay) != 'undefined' && fadingMenuDelay != 0 && fadingMenuDelay !== 'false') {
    $ (".fadeout").delay (parseInt (fadingMenuDelay)).hide (2000);
  };

  // These for loops is for coloring the span element on the top bar
  // for the Red Blue Light/Black theme styling.
  // The total of all span is divided by three and used for indexing.
  // The middle spans will be extra light red, half of the rest
  // on the inner part will be light blue/red, and the others blue/red.
  if (/redblue-menu-tabs/.test(themeColorForTabs)) {
    let nodeListOfTopbarSpan = document.querySelectorAll('#topbar > span');
    let nodeListCountIterator = Math.round(nodeListOfTopbarSpan.length/3);

    for (let i = 0; i < nodeListOfTopbarSpan.length; i++) {
      nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(242, 17%, 57%)";
    };
    for (let i = nodeListCountIterator*2; i < nodeListOfTopbarSpan.length; i++) {
      nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(354, 20%, 57%)";
    };
    for (let i = nodeListCountIterator; i < nodeListCountIterator*2; i++) {
      nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(354, 20%, 77%)";
    };
    for (let i = Math.round(nodeListCountIterator/2); i < nodeListCountIterator; i++) {
      nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(242, 17%, 67%)";
    };
    for (let i = nodeListCountIterator*2; i < (nodeListCountIterator*2)+(Math.round(nodeListCountIterator/2)); i++) {
      nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(354, 20%, 67%)";
    };
  }
    
  // Add class selector for user requested menu tabs color.
  if (themeColorForTabs !== "") {
    document.querySelectorAll('#topbar > span').forEach((element) => {
      element.classList.add(themeColorForTabs);
    })
    if (document.querySelector('#cancelapply') !== null) {
      document.querySelector('#cancelapply').parentNode.classList.add('colored-cancel');
    }
  }

  // Change the state of the current active sub-page's respective main menu.
  function activeTopbarButton (pageLocation, nodeListVariable) {
    for (i = 0; i < nodeListVariable.length; i++) {
      var pageLocationRegExp = new RegExp(pageLocation);
      if (pageLocationRegExp.test(nodeListVariable[i].href)) {
        nodeListVariable[i].parentNode.style.opacity = "1";
        if (themeColorForTabs !== "redblue-menu-tabs" && themeColorForTabs !== "") {
          nodeListVariable[i].parentNode.style.boxShadow = "inset 1px 1px 3px rgba(22, 27, 29, 75%), inset -1px -1px 3px rgba(250, 251, 255, 15%)";
        }
      }
    }
  }
  if (mainMenuAlwaysOn === "1") {
    var nodeListOfTopbarLinks = document.querySelectorAll('div#topbar > span >  a');
		// For advanced mode.
    if (/workspace\/index/.test(window.location.pathname) === true) {
      activeTopbarButton ("workspace", nodeListOfTopbarLinks);
    } else if (/edit\/index/.test(window.location.pathname) || /editone\/index/.test(window.location.pathname) ||
               /editusfm\/index/.test(window.location.pathname) || /notes\/index/.test(window.location.pathname) ||
               /resource\/index/.test(window.location.pathname) || /changes\/changes/.test(window.location.pathname) ||
               /public\/index/.test(window.location.pathname)) {
      activeTopbarButton ("translate", nodeListOfTopbarLinks);
    } else if (/search/.test(window.location.pathname) || /search\/replace/.test(window.location.pathname) ||
               /search\/search2/.test(window.location.pathname) || /search\/replace2/.test(window.location.pathname) ||
               /search\/all/.test(window.location.pathname) || /search\/similar/.test(window.location.pathname) ||
               /search\/strongs/.test(window.location.pathname) || /search\/strong/.test(window.location.pathname) ||
               /search\/originals/.test(window.location.pathname)) {
      activeTopbarButton ("search", nodeListOfTopbarLinks);
    } else if (/tools/.test(window.location.pathname) || /developer\/index/.test(window.location.pathname) ||
               /consistency\/index/.test(window.location.pathname) || /checks\/index/.test(window.location.pathname) ||
               /manage\/exports/.test(window.location.pathname) ||
               /journal\/index/.test(window.location.pathname) || /sendreceive\/index/.test(window.location.pathname) ||
               /revisions/.test(window.location.pathname) || /sprint\/index/.test(window.location.pathname) ||
               /resources\/print/.test(window.location.pathname)) {
      activeTopbarButton ("tools", nodeListOfTopbarLinks);
    } else if (/user\/account/.test(window.location.pathname) || /bible\/manage/.test(window.location.pathname) ||
               /personalize\/index/.test(window.location.pathname) || /checks\/settings/.test(window.location.pathname) ||
               /email\/index/.test(window.location.pathname) || /user\/notifications/.test(window.location.pathname) ||
               /mapping\/index/.test(window.location.pathname) || /manage\/users/.test(window.location.pathname) ||
               /changes\/manage/.test(window.location.pathname) || /collaboration\/index/.test(window.location.pathname) ||
               /styles\/indexm/.test(window.location.pathname) || /system\/index/.test(window.location.pathname) ||
               /settings-resources/.test(window.location.pathname) || /versification\/index/.test(window.location.pathname) ||
               /workspace\/organize/.test(window.location.pathname)) {
      activeTopbarButton ("settings", nodeListOfTopbarLinks);
    } else if (/help/.test(window.location.pathname)) {
      activeTopbarButton ("help", nodeListOfTopbarLinks);
		}
		// For basic mode.
    if (/read\/index/.test(window.location.pathname)) {
			activeTopbarButton ("read", nodeListOfTopbarLinks);
    } else if (/resource\/index/.test(window.location.pathname)) {
			activeTopbarButton ("resource", nodeListOfTopbarLinks);
    } else if (/editone\/index/.test(window.location.pathname)) {
			activeTopbarButton ("editone", nodeListOfTopbarLinks);
    } else if (/notes\/index/.test(window.location.pathname)) {
			activeTopbarButton ("notes", nodeListOfTopbarLinks);
    } else if (/personalize\/index/.test(window.location.pathname)) {
			activeTopbarButton ("personalize", nodeListOfTopbarLinks);
		}
  }

  // If there are more than 9 tabs, a wrapping property in CSS will be activated.
  if ($ ('#topbar > span').length > 9) {
    $ ('#topbar').addClass('wrap-active');
  } else {
    $ ('#topbar').removeClass('wrap-active');
  };

  // If it's notes/actions.html, add padding for all the p tag
  if (/notes\/actions/.test(window.location.href)) {
    var allPTag = document.querySelectorAll('p');
    allPTag.forEach((e) => { e.style.padding = "4px 0" });
  };
});
