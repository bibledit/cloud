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


$ (document).ready (function () 
{
  let arrayOfQueryNames = [
    "themepicker",
    "fontsizegeneral",
    "fontsizemenu",
    "languageselection",
    "timezone",
    "fontsizeeditors",
    "fontsizeresources",
    "fontsizehebrew",
    "fontsizegreek",
    "caretposition",
    "workspacefadeoutdelay",
    "fastswitchusfmeditors",
    "verseseparator",
    "fastswitchvisualeditors",
    "chapterpercentage",
    "versepercentage",
    "dateformat"
    ]
  let arrayOfQuerySubmissionTechniques = [];
  for (var i = 0; i < 5; i++) { arrayOfQuerySubmissionTechniques.push("post+reload") };
  for (var i = 0; i < 9; i++) { arrayOfQuerySubmissionTechniques.push("post") };
  for (var i = 0; i < 2; i++) { arrayOfQuerySubmissionTechniques.push("post+") };
  for (var i = 0; i < 1; i++) { arrayOfQuerySubmissionTechniques.push("post") };

  for (let i = 0; i < arrayOfQueryNames.length; i++) {
    (function () {
      var elementName = "#" + arrayOfQueryNames[i];
      var queryName = arrayOfQueryNames[i];
      var submissionTechnique = arrayOfQuerySubmissionTechniques[i];

      $ (elementName).change (function () {
        var value = $(elementName).val ();
        var postData = '{"'+queryName+'":"'+value+'"}';
        postData = JSON.parse(postData);

        if (submissionTechnique == "post+reload") {
          $.post ("index", postData)
            .done (function() { window.location.reload () });
        }

        if (submissionTechnique == "post" || submissionTechnique == "post+") {
          $.post ("index", postData);
        }

        if (submissionTechnique == "post+") {
          $ (elementName + "val").html (value);
        }
      })
    }());
  }
});
