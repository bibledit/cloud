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
  }
  if (typeof (fadingMenuDelay) != 'undefined' && fadingMenuDelay != 0) {
    $ (".fadeout").delay (parseInt (fadingMenuDelay)).hide (2000);
  }
});
