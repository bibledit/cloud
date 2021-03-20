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

  // If the mouse is over the Workspace tab, the tab will expand
  // and the fadeout class will be removed.
  $ ("#topbar > span:first-child").mouseover (function () {
    this.classList.remove('fadeout');
    this.style.overflow = "unset";
    this.style.whiteSpace = "unset";
    this.style.display = "flex";
    this.style.flexDirection = "row";
    this.style.flexWrap = "wrap";
    this.style.justifyContent = "center";
  });
  // If the mouse left the Workspace tab, the tab will go back
  // to its initial state, the fadeout class is added again.
  // Also there will be half the delay time of the user set delay
  // before the actual user set delay will occur.
  $ ("#topbar > span:first-child").mouseleave (function () {
    this.classList.add('fadeout');
    this.style.overflow = "hidden";
    this.style.whiteSpace = "nowrap";
    this.style.display = "initial";
    this.style.flexDirection = "initial";
    this.style.flexWrap = "initial";
    this.style.justifyContent = "initial";
    setTimeout(function () {
      if (typeof (fadingMenuDelay) != 'undefined' && fadingMenuDelay != 0) {
        $ (".fadeout").delay (parseInt (fadingMenuDelay)).hide (2000);
      };
    }, fadingMenuDelay/2);
  });
    
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

  // These change the colors of OT, NT, and other books respectively by their
  // index number. The name of the books have to be inserted manually because
  // they are identified in the DOM from the element's text content. Also
  // the if statement with regex is added so that the verse and chapters
  // wouldn't be given background colors.
  var startOfOT = "Genesis" || "Kejadian";
  var endOfOT = "Malachi" || "Maleaki";
  var startOfOTIndex = 0;
  var endOfOTIndex = 0;
  var startOfNT = "Matthew" || "Matius";
  var endOfNT = "Revelation" || "Wahyu";
  var startOfNTIndex = 0;
  var endOfNTIndex = 0;
  $ ("#versepickerwrapper").on ("click", function() {
    setTimeout(() => {
      let bookSelector = $(".selector");
      for (var i = 0; i < bookSelector.length; i++) {
        if (bookSelector[i].textContent == startOfOT) {
          startOfOTIndex = i;
        } else if (bookSelector[i].textContent == endOfOT) {
          endOfOTIndex = i;
        } else if (bookSelector[i].textContent == startOfNT) {
          startOfNTIndex = i;
        } else if (bookSelector[i].textContent == endOfNT) {
          endOfNTIndex = i;
          break;
        }
      };
      if (/[0-9]/.test(bookSelector[0].textContent) == false) {
        for (var i = startOfOTIndex; i <= endOfOTIndex; i++) {
          bookSelector[i].querySelector('a').style.backgroundColor = "#b81a1a";
          bookSelector[i].querySelector('a').style.color = "#fff";
          };
        for (var i = startOfNTIndex; i <= endOfNTIndex; i++) {
          bookSelector[i].querySelector('a').style.backgroundColor = "#1a24ad";
          bookSelector[i].querySelector('a').style.color = "#fff";
          };
      };
    },750);
  });

  // These add the active class to the workspace picker of the current workspace.
  var startOfIndex = window.location.href.length - 2;
  var endOfIndex = window.location.href.length;
  let workspacePicker = $('.fadeout > span');
  for (var i = 0; i <= workspacePicker.length; i++) {
    if (i == window.location.href.slice(startOfIndex,endOfIndex).match(/[0-9]*/)) {
      workspacePicker[i].classList.add('active');
    }
  }
});
