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


#include <unittests/gbs.h>
#include <unittests/utilities.h>
#include <resource/logic.h>
#include <resource/external.h>
#include <filter/string.h>


void test_gbs () // Todo
{
  trace_unit_tests (__func__);

  string resource;
  int book, chapter, verse;
  string text;

  resource = "Statenbijbel GBS";
  book = 58; // Hebrews.
  text = resource_external_cloud_fetch_cache_extract (resource, book, 10, 14);
  evaluate (__LINE__, __func__, "Want met één offerande heeft Hij in eeuwigheid volmaakt degenen die geheiligd worden.", text);

  resource = "King James Version GBS";
  book = 58; // Hebrews.
  text = resource_external_cloud_fetch_cache_extract (resource, book, 10, 14);
  evaluate (__LINE__, __func__, "For by one offering he hath perfected for ever them that are sanctified.", text);

  resource = "Statenbijbel Plus GBS";
  book = 58; // Hebrews.
  text = resource_external_cloud_fetch_cache_extract (resource, book, 10, 1);
  evaluate (__LINE__, __func__, "WANT a 1 de wet, hebbende 2 een schaduw 3 der toekomende goederen, niet 4 het beeld zelf der zaken, kan met 5 dezelfde offeranden die zij alle jaar 6 geduriglijk opofferen, nimmermeer 7 heiligen degenen die 8 daar toegaan. <br> a Kol. 2:17 Welke zijn een schaduw der toekomende dingen, maar het lichaam is van Christus. <br> a Hebr. 8:5 Welke het voorbeeld en de schaduw der hemelse dingen dienen, gelijk Mozes door Goddelijke aanspraak vermaand was, als hij den tabernakel volmaken zou. Want zie, zegt Hij, dat gij het alles maakt naar de afbeelding die u op den berg getoond is.", text);
 
}
