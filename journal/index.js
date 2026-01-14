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


window.addEventListener ('load', function (event) {
  document.querySelector ("#logbook").addEventListener ("click", journalClicked);
  scrollToEnd();
});


function getMore ()
{
  const url = "index?filename=" + filename;
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
    if (response == "") {
      setTimeout (getMore, 2000);
    } else {
      var response = response.split ("\n");
      filename = response [0];
      for (var i = 1; i < response.length; i++) {
        document.querySelector ("#logbook").insertAdjacentHTML('beforeend', response [i]);
      }
      scrollToEnd();
      setTimeout (getMore, 100);
    }
  })
  .catch((error) => {
    console.log(error);
  })
}


setTimeout (getMore, 3000);


function journalClicked (event)
{
  event.preventDefault ();
  if (typeof event.target.href === "undefined") return;
  var target = event.target;
  const url = "index?expansion=" + target.href;
  fetch(url, {
    method: "GET",
  })
  .then((response) => {
    if (!response.ok) {
      throw new Error(response.status);
    }
    return response.text();
  })
  .then((response) => {
    target.replaceWith(response);
  })
  .catch((error) => {
    console.log(error);
  })
}


function scrollToEnd()
{
  document.querySelector ("#clear").scrollIntoView({
    behavior: 'smooth',
    block: 'center',
    inline: 'center'
  });
}
