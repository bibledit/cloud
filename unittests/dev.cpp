/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


#include <unittests/french.h>
#include <unittests/utilities.h>
#include <checks/french.h>
#include <database/check.h>
#include <filter/string.h>
#include <filter/text.h>
#include <checks/usfm.h>


void test_dev () // Todo
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  
  string bible = "phpunit";
  
  
  // Test check on matching markers.
  {
    string usfm = R"(
\id GEN
\v 1 Njalo kwakwenziwe \w \+add ng\+add*amakherubhi\w* lezihlahla zelala, kuze kuthi isihlahla selala sasiphakathi kwekherubhi lekherubhi; njalo \add yilelo lalelo\add* ikherubhi lalilobuso obubili.
\v 2 Izembatho zakho zonke \add ziqholiwe\add* \w ngemure\w* lenhlaba \w \+add l\+add*ekhasiya\w*; ezigodlweni zempondo zendlovu abakwenze wathokoza ngazo.
\v 3 Ngasengiyithenga insimu kuHanameli indodana kamalumami, eseAnathothi, ngamlinganisela imali \w \+add eng\+add*amashekeli\w** ayisikhombisa lenhlamvu ezilitshumi zesiliva.
    )";
    Checks_Usfm check = Checks_Usfm ("phpunit");
    check.initialize (0, 0);
    check.check (usfm);
    check.finalize ();
    vector <pair<int, string>> results = check.getResults ();
    vector <pair<int, string>> standard = {};
    evaluate (__LINE__, __func__, standard, results);
    for (size_t i = 0; i < results.size (); i++) {
      cout << results[i].first << " " << results[i].second << endl;
    }
  }
  
}

