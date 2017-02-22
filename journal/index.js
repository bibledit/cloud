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

$(window).on ('load', function () {
  $ ("#logbook").on ("click", journal_clicked);
  $ ("#workspacewrapper").animate ({ scrollTop: $("#workspacewrapper").prop("scrollHeight") }, 3000);
});

var getMore = function () 
{
  $.ajax ({
    url: "index",
    type: "GET",
    data: { filename: filename },
    cache: false,
    success: function (response) {
      if (response == "") {
        setTimeout (getMore, 2000);
      } else {
        var response = response.split ("\n");
        filename = response [0];
        for (var i = 1; i < response.length; i++) {
          $ ("#logbook").append ($ (response [i]));
        }
        $("#workspacewrapper").animate ({ scrollTop: $("#workspacewrapper").prop("scrollHeight") }, 100);
        setTimeout (getMore, 100);
      }
    },
  });
}

setTimeout (getMore, 3000);

function journal_clicked (event)
{
  event.preventDefault ();
  if (typeof event.target.href === "undefined") return;
  var target = $ (event.target);
  $.ajax ({
    url: "index",
    type: "GET",
    data: { expansion: event.target.href },
    cache: false,
    success: function (response) {
      target.replaceWith (response);
    },
  });
}
