/*
Copyright (©) 2003-2024 Teus Benschop.

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
function noteClickSetup ()
{
  $("a.opennote").off ("click");
  $("a.newnote").off ("click");
  $("a.opennote").on ("click", function (event) {
    event.preventDefault ();
    $.ajax ({
      url: "/notes/click",
      type: "GET",
      data: { open: event.target.href },
      cache: false,
    });
  });
  $("a.newnote").on ("click", function (event) {
    event.preventDefault ();
    $.ajax ({
      url: "/notes/click",
      type: "GET",
      data: { new: event.target.href },
      cache: false,
    });
  });
}
