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
    // The topbar is removed by the server via the Workspace,
    // but in other cases the topbar makes it to the browser,
    // and is to be removed here within the iframe.
    $ ("#topbar").empty ();
  };
  if (typeof (fadingMenuDelay) != 'undefined' && fadingMenuDelay != 0) {
    $ (".fadeout").delay (parseInt (fadingMenuDelay)).hide (2000);
  };
    
  // These for loops is for coloring the span element on the top bar.
  // The total of all span is divided by three and used for indexing.
  // The middle spans will be extra light red, half of the rest
  // on the inner part will be light blue/red, and the others blue/red.
  let nodeListOfTopbarSpan = document.querySelectorAll('#topbar > span');
  let nodeListCountIterator = Math.round(nodeListOfTopbarSpan.length/3);
  
  for (let i = 0; i < nodeListOfTopbarSpan.length; i++) {
    nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(242, 17%, 60%)";
  };
  for (let i = nodeListCountIterator*2; i < nodeListOfTopbarSpan.length; i++) {
    nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(354, 20%, 60%)";
  };
  for (let i = nodeListCountIterator; i < nodeListCountIterator*2; i++) {
    nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(354, 20%, 90%)";
  };
  for (let i = Math.round(nodeListCountIterator/2); i < nodeListCountIterator; i++) {
    nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(242, 17%, 75%)";
  };
  for (let i = nodeListCountIterator*2; i < (nodeListCountIterator*2)+(Math.round(nodeListCountIterator/2)); i++) {
    nodeListOfTopbarSpan[i].style.backgroundColor = "hsl(354, 20%, 75%)";
  };

  // If there are more than 6 tabs, a wrapping property in CSS will be activated.
  if ($ ('#topbar > span').length > 6 || $('#versepickerwrapper').on) {
    $ ('#topbar').addClass('wrap-active');
  } else {
    $ ('#topbar').removeClass('wrap-active');
  };
});
