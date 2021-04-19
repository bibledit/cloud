/*
Copyright (©) 2003-2021 Teus Benschop.

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
let counterForBreadcrumbContainerDisplay = 0;
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
      element.href = topbarRemovalQueryAddition (element.href);
    })
    document.querySelectorAll('form').forEach((element) => {
      element.action = topbarRemovalQueryAddition (element.action);
    })
    $ ('#topbar').empty ();
  };
  if (typeof (fadingMenuDelay) != 'undefined' && fadingMenuDelay != 0) {
    $ (".fadeout").delay (parseInt (fadingMenuDelay)).hide (2000);
  };
    
  // Add class selector for user requested menu tabs color.
  if (themeColorForTabs !== "") {
    document.querySelectorAll('#topbar > span').forEach((element) => {
      element.classList.add(themeColorForTabs);
    })
    if (document.querySelector('#cancelapply') !== null) {
      document.querySelector('#cancelapply').parentNode.classList.add('colored-cancel');
    }
  }

  // If there are more than 9 tabs, a wrapping property in CSS will be activated.
  if ($ ('#topbar > span').length > 9) {
    $ ('#topbar').addClass('wrap-active');
  } else {
    $ ('#topbar').removeClass('wrap-active');
  };
});
