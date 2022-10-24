/*
Copyright (©) 2003-2022 Teus Benschop.

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


void test_gbs ()
{
  trace_unit_tests (__func__);

  string resource {};
  int book {};
  string text {};

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
  evaluate (__LINE__, __func__, "WANT a 1 de wet, hebbende 2 een schaduw 3 der toekomende goederen, niet 4 het beeld zelf der zaken, kan met 5 dezelfde offeranden die zij alle jaar 6 geduriglijk opofferen, nimmermeer 7 heiligen degenen die 8 daar toegaan. <br> a Kol. 2:17 Welke zijn een schaduw der toekomende dingen, maar het lichaam is van Christus. <br> a Hebr. 8:5 Welke het voorbeeld en de schaduw der hemelse dingen dienen, gelijk Mozes door Goddelijke aanspraak vermaand was, als hij den tabernakel volmaken zou. Want zie, zegt Hij, dat gij het alles maakt naar de afbeelding die u op den berg getoond is. <br> 1 Namelijk der ceremoniën onder het Oude Testament. <br> 2 Dat is, een ruw ontwerp, gelijk de schilders plegen een beeld, dat zij daarna willen volmaken, eerst met enige linies en schaduwen in het ruw af te tekenen of te ontwerpen. <br> 3 Dat is, der geestelijke en hemelse zaken, die ons in het Nieuwe Testament zouden verworven en medegedeeld worden, dat is, van Christus Zelven en Zijn weldaden. <br> 4 Sommigen verstaan hierdoor de volmaakte wijze van den uitwendigen godsdienst, dien God door Christus in het Nieuwe Testament zou instellen, die van de instellingen des Ouden Testaments verschilden als een schaduw of eerste ontwerp van het volmaakte beeld eniger zaak; gelijk die daarom ook de eerste beginselen of elementen en abc der wereld worden genaamd, Gal. 4:3 , 9 . Doch dewijl de apostel nergens in dit en in het voorgaande hoofdstuk enige tegenstelling maakt tussen den uitwendigen godsdienst des Ouden en des Nieuwen Testaments, maar alleen tussen de schaduwen des Ouden Testaments en Christus Zelven met Zijn offerande en weldaden, die Hij ons verworven heeft, zo wordt van anderen door deze woorden het beeld zelf bekwamelijker de betekende zaak zelve of het evenbeeld verstaan, naar hetwelk deze schaduwen zijn voorgesteld, gelijk wij worden gezegd geschapen te zijn naar Gods beeld of evenbeeld. Welken zin het bijgevoegde woord het beeld zelf der zaken ook vereist, dat is, de zaken zelve in haar volle beeltenis of gedaante, gelijk zij moesten en zouden zijn. <br> 5 Dat is, van enerlei soort, of van eenzelfden aard en natuur. <br> 6 Gr. in gedurigheid of eeuwigheid , dat is, zonder nalaten, zolang dit priesterdom en deze wet moest duren. <br> 7 Of: volmaken , namelijk naar de consciëntie, door het wegnemen der zonde en der schuld der zonde, gelijk verklaard wordt Hebr. 9:9 . <br> 8 Of: tot God gaan , namelijk met hun offeranden.", text);
 
  resource = "Statenbijbel GBS";
  book = 3; // Leviticus.
  text = resource_external_cloud_fetch_cache_extract (resource, book, 1, 0);
  evaluate (__LINE__, __func__, "Wetten aangaande de manier van het vrijwillig brandoffer in den tabernakel te slachten, vs. 1 , enz. Hetwelk moest zijn óf van grootvee, als van runderen, 2 . Of van kleinvee, als van schapen en geiten, 10 . Of van vogels, als van tortelduiven en jonge duiven, 14 .", text);

  resource = "Statenbijbel Plus GBS";
  book = 3; // Leviticus.
  text = resource_external_cloud_fetch_cache_extract (resource, book, 1, 0);
  evaluate (__LINE__, __func__, "Wetten aangaande de manier van het vrijwillig brandoffer in den tabernakel te slachten, vs. 1 , enz. Hetwelk moest zijn óf van grootvee, als van runderen, 2 . Of van kleinvee, als van schapen en geiten, 10 . Of van vogels, als van tortelduiven en jonge duiven, 14 .", text);

}
