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


#include <setup/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <webserver/request.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <index/index.h>
#include <demo/logic.h>
#include <config/globals.h>
#include <database/config/general.h>
#include <setup/logic.h>


// Returns a html page with a Bibledit splash screen.
// In case of $refresh, it refreshes to the home page.
string notice;
const char * setup_initialization_notice ()
{
  // Use http://base64online.org/encode/ to convert an image to inline graphic data.
  
  notice =
  "<!DOCTYPE html>\n"
  "<html>\n"
  "<head>\n"
  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
  "<meta http-equiv=\"refresh\" content=\"2;URL=/index/index\" />\n"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
  "<title>Setup</title>\n"
  "<style>\n"
  ".splash {\n"
  " height: 100%;\n"
  " width: 100%;\n"
  " background: radial-gradient(red, yellow, white);\n"
  " position: absolute;\n"
  " z-index: -1;\n"
  " opacity: 0.15;\n"
  "}\n"
  "img {\n"
  " min-height: 100%;\n"
  " min-width: 100%;\n"
  " height: 100%;\n"
  " width: 100%;\n"
  " position: absolute;\n"
  " top:-50%; bottom: -50%;\n"
  " left:-50%; right: -50%;\n"
  " margin: auto;\n"
  " z-index: -1;\n"
  " background-repeat: no-repeat;\n"
  " background-position: center center;\n"
  " background-size: contain;\n"
  "}\n"
  "</style>\n"
  "</head>\n"
  "<body>\n"
  "<div class=\"splash\"></div>\n"
  "<div><img src=\"data:image/svg+xml;base64,PHN2ZyBoZWlnaHQ9IjEwNS4yODQ3NSIgaWQ9InN2ZzM5NjAiIGlua3NjYXBlOnZlcnNpb249IjAuNDMrMC40NHByZTMiIHNvZGlwb2RpOmRvY2Jhc2U9IkM6XERvY3VtZW50cyBhbmQgU2V0dGluZ3NcSXZhbiBCYWNoZXZcRGVza3RvcCIgc29kaXBvZGk6ZG9jbmFtZT0iYm9vazIuc3ZnIiBzb2RpcG9kaTp2ZXJzaW9uPSIwLjMyIiB2ZXJzaW9uPSIxLjAiIHdpZHRoPSIyMjQuNjQ5MTEiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgeG1sbnM6Y2M9Imh0dHA6Ly93ZWIucmVzb3VyY2Uub3JnL2NjLyIgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIiB4bWxuczppbmtzY2FwZT0iaHR0cDovL3d3dy5pbmtzY2FwZS5vcmcvbmFtZXNwYWNlcy9pbmtzY2FwZSIgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIiB4bWxuczpzb2RpcG9kaT0iaHR0cDovL3NvZGlwb2RpLnNvdXJjZWZvcmdlLm5ldC9EVEQvc29kaXBvZGktMC5kdGQiIHhtbG5zOnN2Zz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHhtbG5zOnhsaW5rPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5L3hsaW5rIj4KICA8bWV0YWRhdGE+CiAgICA8cmRmOlJERiB4bWxuczpjYz0iaHR0cDovL3dlYi5yZXNvdXJjZS5vcmcvY2MvIiB4bWxuczpkYz0iaHR0cDovL3B1cmwub3JnL2RjL2VsZW1lbnRzLzEuMS8iIHhtbG5zOnJkZj0iaHR0cDovL3d3dy53My5vcmcvMTk5OS8wMi8yMi1yZGYtc3ludGF4LW5zIyI+CiAgICAgIDxjYzpXb3JrIHJkZjphYm91dD0iIj4KICAgICAgICA8ZGM6dGl0bGU+b3BlbiBib29rPC9kYzp0aXRsZT4KICAgICAgICA8ZGM6ZGVzY3JpcHRpb24+PC9kYzpkZXNjcmlwdGlvbj4KICAgICAgICA8ZGM6c3ViamVjdD4KICAgICAgICAgIDxyZGY6QmFnPgogICAgICAgICAgICA8cmRmOmxpPm5vdGUtYm9vazwvcmRmOmxpPgogICAgICAgICAgICA8cmRmOmxpPmJvb2s8L3JkZjpsaT4KICAgICAgICAgICAgPHJkZjpsaT5wYXBlcjwvcmRmOmxpPgogICAgICAgICAgPC9yZGY6QmFnPgogICAgICAgIDwvZGM6c3ViamVjdD4KICAgICAgICA8ZGM6cHVibGlzaGVyPgogICAgICAgICAgPGNjOkFnZW50IHJkZjphYm91dD0iaHR0cDovL3d3dy5vcGVuY2xpcGFydC5vcmcvIj4KICAgICAgICAgICAgPGRjOnRpdGxlPk9wZW4gQ2xpcCBBcnQgTGlicmFyeTwvZGM6dGl0bGU+CiAgICAgICAgICA8L2NjOkFnZW50PgogICAgICAgIDwvZGM6cHVibGlzaGVyPgogICAgICAgIDxkYzpjcmVhdG9yPgogICAgICAgICAgPGNjOkFnZW50PgogICAgICAgICAgICA8ZGM6dGl0bGU+SXZhbiBCYWNoZXY8L2RjOnRpdGxlPgogICAgICAgICAgPC9jYzpBZ2VudD4KICAgICAgICA8L2RjOmNyZWF0b3I+CiAgICAgICAgPGRjOnJpZ2h0cz4KICAgICAgICAgIDxjYzpBZ2VudD4KICAgICAgICAgICAgPGRjOnRpdGxlPkl2YW4gQmFjaGV2PC9kYzp0aXRsZT4KICAgICAgICAgIDwvY2M6QWdlbnQ+CiAgICAgICAgPC9kYzpyaWdodHM+CiAgICAgICAgPGRjOmRhdGU+MTMtMDYtMjAwNjwvZGM6ZGF0ZT4KICAgICAgICA8ZGM6Zm9ybWF0PmltYWdlL3N2Zyt4bWw8L2RjOmZvcm1hdD4KICAgICAgICA8ZGM6dHlwZSByZGY6cmVzb3VyY2U9Imh0dHA6Ly9wdXJsLm9yZy9kYy9kY21pdHlwZS9TdGlsbEltYWdlIi8+CiAgICAgICAgPGNjOmxpY2Vuc2UgcmRmOnJlc291cmNlPSJodHRwOi8vd2ViLnJlc291cmNlLm9yZy9jYy9QdWJsaWNEb21haW4iLz4KICAgICAgICA8ZGM6bGFuZ3VhZ2U+ZW48L2RjOmxhbmd1YWdlPgogICAgICA8L2NjOldvcms+CiAgICAgIDxjYzpMaWNlbnNlIHJkZjphYm91dD0iaHR0cDovL3dlYi5yZXNvdXJjZS5vcmcvY2MvUHVibGljRG9tYWluIj4KICAgICAgICA8Y2M6cGVybWl0cyByZGY6cmVzb3VyY2U9Imh0dHA6Ly93ZWIucmVzb3VyY2Uub3JnL2NjL1JlcHJvZHVjdGlvbiIvPgogICAgICAgIDxjYzpwZXJtaXRzIHJkZjpyZXNvdXJjZT0iaHR0cDovL3dlYi5yZXNvdXJjZS5vcmcvY2MvRGlzdHJpYnV0aW9uIi8+CiAgICAgICAgPGNjOnBlcm1pdHMgcmRmOnJlc291cmNlPSJodHRwOi8vd2ViLnJlc291cmNlLm9yZy9jYy9EZXJpdmF0aXZlV29ya3MiLz4KICAgICAgPC9jYzpMaWNlbnNlPgogICAgPC9yZGY6UkRGPgogIDwvbWV0YWRhdGE+CiAgPGRlZnMgaWQ9ImRlZnMzOTYyIj4KICAgIDxsaW5lYXJHcmFkaWVudCBpZD0ibGluZWFyR3JhZGllbnQxMDk3MSI+CiAgICAgIDxzdG9wIGlkPSJzdG9wMTA5NzMiIG9mZnNldD0iMCIgc3R5bGU9InN0b3AtY29sb3I6d2hpdGU7c3RvcC1vcGFjaXR5OjE7Ii8+CiAgICAgIDxzdG9wIGlkPSJzdG9wMTA5NzUiIG9mZnNldD0iMSIgc3R5bGU9InN0b3AtY29sb3I6I2JlYmViZTtzdG9wLW9wYWNpdHk6MDsiLz4KICAgIDwvbGluZWFyR3JhZGllbnQ+CiAgICA8cmFkaWFsR3JhZGllbnQgY3g9IjM4OS45MTg4OCIgY3k9IjQ4Ny42ODY5MiIgZng9IjM4OS45MTg4OCIgZnk9IjQ4Ny42ODY5MiIgZ3JhZGllbnRUcmFuc2Zvcm09Im1hdHJpeCgxLDAsMCwwLjY2LDAsMTY1LjgxMzUpIiBncmFkaWVudFVuaXRzPSJ1c2VyU3BhY2VPblVzZSIgaWQ9InJhZGlhbEdyYWRpZW50MjkyMSIgaW5rc2NhcGU6Y29sbGVjdD0iYWx3YXlzIiByPSI1MC41MDc2MjYiIHhsaW5rOmhyZWY9IiNsaW5lYXJHcmFkaWVudDEwOTcxIi8+CiAgICA8bGluZWFyR3JhZGllbnQgaWQ9ImxpbmVhckdyYWRpZW50NjQwOCI+CiAgICAgIDxzdG9wIGlkPSJzdG9wNjQxMCIgb2Zmc2V0PSIwIiBzdHlsZT0ic3RvcC1jb2xvcjojZTRkOWFhO3N0b3Atb3BhY2l0eToxOyIvPgogICAgICA8c3RvcCBpZD0ic3RvcDczMDQiIG9mZnNldD0iMC41MTc4NTcxMyIgc3R5bGU9InN0b3AtY29sb3I6I2Y4ZjdmNDtzdG9wLW9wYWNpdHk6MTsiLz4KICAgICAgPHN0b3AgaWQ9InN0b3A2NDEyIiBvZmZzZXQ9IjEiIHN0eWxlPSJzdG9wLWNvbG9yOiNlNGQ5YWE7c3RvcC1vcGFjaXR5OjE7Ii8+CiAgICA8L2xpbmVhckdyYWRpZW50PgogICAgPGxpbmVhckdyYWRpZW50IGdyYWRpZW50VW5pdHM9InVzZXJTcGFjZU9uVXNlIiBpZD0ibGluZWFyR3JhZGllbnQyOTIzIiBpbmtzY2FwZTpjb2xsZWN0PSJhbHdheXMiIHgxPSIyMzQuNjM4NzUiIHgyPSI1NTEuMjU5ODkiIHhsaW5rOmhyZWY9IiNsaW5lYXJHcmFkaWVudDY0MDgiIHkxPSI2ODUuNzIyNTMiIHkyPSI2ODUuNzIyNTMiLz4KICAgIDxsaW5lYXJHcmFkaWVudCBpZD0ibGluZWFyR3JhZGllbnQzODQxIiBpbmtzY2FwZTpjb2xsZWN0PSJhbHdheXMiPgogICAgICA8c3RvcCBpZD0ic3RvcDM4NDMiIG9mZnNldD0iMCIgc3R5bGU9InN0b3AtY29sb3I6d2hpdGU7c3RvcC1vcGFjaXR5OjE7Ii8+CiAgICAgIDxzdG9wIGlkPSJzdG9wMzg0NSIgb2Zmc2V0PSIxIiBzdHlsZT0ic3RvcC1jb2xvcjp3aGl0ZTtzdG9wLW9wYWNpdHk6MDsiLz4KICAgIDwvbGluZWFyR3JhZGllbnQ+CiAgICA8bGluZWFyR3JhZGllbnQgZ3JhZGllbnRUcmFuc2Zvcm09Im1hdHJpeCgxLjE4MzEwNywtMC4xOTA2ODksMC4xOTA2ODksMS4xODMxMDcsLTE0MjIuODEyLC0xODkuOTAwNikiIGdyYWRpZW50VW5pdHM9InVzZXJTcGFjZU9uVXNlIiBpZD0ibGluZWFyR3JhZGllbnQzOTEwIiBpbmtzY2FwZTpjb2xsZWN0PSJhbHdheXMiIHgxPSIxNTI1LjYzMTUiIHgyPSIxMzk2LjE0NzYiIHhsaW5rOmhyZWY9IiNsaW5lYXJHcmFkaWVudDM4NDEiIHkxPSI4MzYuMTA4MjgiIHkyPSI4MjMuMjE0NjYiLz4KICAgIDxyYWRpYWxHcmFkaWVudCBjeD0iMzg5LjkxODg4IiBjeT0iNDg3LjY4NjkyIiBmeD0iMzg5LjkxODg4IiBmeT0iNDg3LjY4NjkyIiBncmFkaWVudFRyYW5zZm9ybT0ibWF0cml4KDEsMCwwLDAuNjYsMCwxNjUuODEzNSkiIGdyYWRpZW50VW5pdHM9InVzZXJTcGFjZU9uVXNlIiBpZD0icmFkaWFsR3JhZGllbnQ0MDAzIiBpbmtzY2FwZTpjb2xsZWN0PSJhbHdheXMiIHI9IjUwLjUwNzYyNiIgeGxpbms6aHJlZj0iI2xpbmVhckdyYWRpZW50MTA5NzEiLz4KICAgIDxsaW5lYXJHcmFkaWVudCBncmFkaWVudFVuaXRzPSJ1c2VyU3BhY2VPblVzZSIgaWQ9ImxpbmVhckdyYWRpZW50NDAwNSIgaW5rc2NhcGU6Y29sbGVjdD0iYWx3YXlzIiB4MT0iMjM0LjYzODc1IiB4Mj0iNTUxLjI1OTg5IiB4bGluazpocmVmPSIjbGluZWFyR3JhZGllbnQ2NDA4IiB5MT0iNjg1LjcyMjUzIiB5Mj0iNjg1LjcyMjUzIi8+CiAgICA8bGluZWFyR3JhZGllbnQgZ3JhZGllbnRUcmFuc2Zvcm09Im1hdHJpeCgxLjE4MzEwNywtMC4xOTA2ODksMC4xOTA2ODksMS4xODMxMDcsLTE0MjIuODEyLC0xODkuOTAwNikiIGdyYWRpZW50VW5pdHM9InVzZXJTcGFjZU9uVXNlIiBpZD0ibGluZWFyR3JhZGllbnQ0MDA3IiBpbmtzY2FwZTpjb2xsZWN0PSJhbHdheXMiIHgxPSIxNTI1LjYzMTUiIHgyPSIxMzk2LjE0NzYiIHhsaW5rOmhyZWY9IiNsaW5lYXJHcmFkaWVudDM4NDEiIHkxPSI4MzYuMTA4MjgiIHkyPSI4MjMuMjE0NjYiLz4KICA8L2RlZnM+CiAgPHNvZGlwb2RpOm5hbWVkdmlldyBib3JkZXJjb2xvcj0iIzY2NjY2NiIgYm9yZGVyb3BhY2l0eT0iMS4wIiBpZD0iYmFzZSIgaW5rc2NhcGU6Y3VycmVudC1sYXllcj0ibGF5ZXIxIiBpbmtzY2FwZTpjeD0iMjM0LjQ0MjMxIiBpbmtzY2FwZTpjeT0iMjQ5LjQ0NjQ4IiBpbmtzY2FwZTpkb2N1bWVudC11bml0cz0icHgiIGlua3NjYXBlOnBhZ2VvcGFjaXR5PSIwLjAiIGlua3NjYXBlOnBhZ2VzaGFkb3c9IjIiIGlua3NjYXBlOnpvb209IjAuNyIgcGFnZWNvbG9yPSIjZmZmZmZmIi8+CiAgPGcgaWQ9ImxheWVyMSIgaW5rc2NhcGU6Z3JvdXBtb2RlPSJsYXllciIgaW5rc2NhcGU6bGFiZWw9IkxheWVyIDEiIHRyYW5zZm9ybT0idHJhbnNsYXRlKC0yNjIuNjc1NCwtNDc5LjcxOTgpIj4KICAgIDxnIGlkPSJnMzk5MSI+CiAgICAgIDxwYXRoIGQ9Ik0gNDQwLjQyNjUxIDQ4Ny42ODY5MiBBIDUwLjUwNzYyNiAzMy4zMzUwMzMgMCAxIDEgIDMzOS40MTEyNiw0ODcuNjg2OTIgQSA1MC41MDc2MjYgMzMuMzM1MDMzIDAgMSAxICA0NDAuNDI2NTEgNDg3LjY4NjkyIHoiIGlkPSJwYXRoMjg4OSIgc29kaXBvZGk6Y3g9IjM4OS45MTg4OCIgc29kaXBvZGk6Y3k9IjQ4Ny42ODY5MiIgc29kaXBvZGk6cng9IjUwLjUwNzYyNiIgc29kaXBvZGk6cnk9IjMzLjMzNTAzMyIgc29kaXBvZGk6dHlwZT0iYXJjIiBzdHlsZT0ib3BhY2l0eToxO2ZpbGw6dXJsKCNyYWRpYWxHcmFkaWVudDQwMDMpO2ZpbGwtb3BhY2l0eToxO2ZpbGwtcnVsZTpub256ZXJvO3N0cm9rZTpub25lO3N0cm9rZS13aWR0aDozO3N0cm9rZS1saW5lY2FwOmJ1dHQ7c3Ryb2tlLWxpbmVqb2luOnJvdW5kO3N0cm9rZS1taXRlcmxpbWl0OjQ7c3Ryb2tlLWRhc2hhcnJheTpub25lO3N0cm9rZS1kYXNob2Zmc2V0OjA7c3Ryb2tlLW9wYWNpdHk6MSIgdHJhbnNmb3JtPSJtYXRyaXgoMS44ODEyMjQsMCwwLDEuMDQxMTYyLC0zNjAuMDA2NCw0Mi41MzYzKSIvPgogICAgICA8ZyBpZD0iZzI4OTEiIHRyYW5zZm9ybT0ibWF0cml4KDAuNjU2NjQ1LDAsMCwwLjY1NjY0NSwxMTguMzc3NywyMTkuMzkwNikiPgogICAgICAgIDxwYXRoIGQ9Ik0gMzE1Ljg3NSw0MDguMTQyMjMgQyAyOTcuMDI5NDYsNDA4LjI2NDUgMjc3LjQyNzA4LDQxMC45MDQ4OCAyNTQuNTMxMjUsNDEzLjc2NzIzIEwgMjE5Ljc1LDUyNi4yMzU5OCBDIDI3Ny42MjMxNCw1MTIuMjgzOTcgMzE3LjUyNzIxLDUyNC4zOTc5NiAzOTAuMTg3NSw1NDYuMjM1OTggTCAzOTAuMDkzNzUsNDI1LjIwNDczIEMgMzYzLjA2Njk3LDQxMi4wMTE4MyAzNDAuMTA0OTgsNDA3Ljk4NTAyIDMxNS44NzUsNDA4LjE0MjIzIHogTSA0NjEuNDM3NSw0MDguMTQyMjMgQyA0MzguNjcxNDcsNDA4LjQ0ODM2IDQxNi43NzUzLDQxMi43NDQ3NyAzOTEuMjUsNDI1LjIwNDczIEwgMzkxLjE4NzUsNTQ2LjIzNTk4IEMgNDYzLjg0Nzc5LDUyNC4zOTc5NiA1MDMuNzUxODcsNTEyLjI4Mzk3IDU2MS42MjUsNTI2LjIzNTk4IEwgNTI2Ljg0Mzc1LDQxMy43NjcyMyBDIDUwMi4zMTI1LDQxMC43MDA0MyA0ODEuNTI1MTcsNDA3Ljg3MjEyIDQ2MS40Mzc1LDQwOC4xNDIyMyB6ICIgaWQ9InBhdGgyODkzIiBpbmtzY2FwZTp0aWxlLWN4PSIzMTAuNjg3NSIgaW5rc2NhcGU6dGlsZS1jeT0iNDAxLjg4MTY3IiBpbmtzY2FwZTp0aWxlLWg9IjEzOC4xMTE2NSIgaW5rc2NhcGU6dGlsZS13PSIzNDEuODc1IiBzdHlsZT0iY29sb3I6YmxhY2s7ZmlsbDojYTZhMThlO2ZpbGwtb3BhY2l0eToxO2ZpbGwtcnVsZTpldmVub2RkO3N0cm9rZTpub25lO3N0cm9rZS13aWR0aDowLjY0MjY2MzI0cHg7c3Ryb2tlLWxpbmVjYXA6YnV0dDtzdHJva2UtbGluZWpvaW46bWl0ZXI7bWFya2VyOm5vbmU7bWFya2VyLXN0YXJ0Om5vbmU7bWFya2VyLW1pZDpub25lO21hcmtlci1lbmQ6bm9uZTtzdHJva2UtbWl0ZXJsaW1pdDo0O3N0cm9rZS1kYXNob2Zmc2V0OjA7c3Ryb2tlLW9wYWNpdHk6MTt2aXNpYmlsaXR5OnZpc2libGU7ZGlzcGxheTppbmxpbmU7b3ZlcmZsb3c6dmlzaWJsZSIvPgogICAgICAgIDxnIGlkPSJnMjg5NSIgc3R5bGU9ImZpbGw6IzkzNmI1MDtmaWxsLW9wYWNpdHk6MSIgdHJhbnNmb3JtPSJ0cmFuc2xhdGUoMC4zNTcxNDMsLTIxOC4xOTI5KSI+CiAgICAgICAgICA8cGF0aCBkPSJNIDMzNi42ODg1Myw3NDAuMzQyNjMgTCAzMzUuNzk3NjQsNzQ4LjgzNjY5IEMgMzU3LjQxMzU2LDc1NC44MzE5NSAzNjcuMzU0NzcsNzU3LjQ4MDEgMzkwLjA1NDg3LDc2NC4zMDI2MSBMIDM5MC4wMjU1OCw3NTUuNjU4OSBDIDM3My44OTQzMiw3NTAuNzc1NTUgMzcyLjI3MTQzLDc1MS41OTQ4MSAzMzYuNjg4NTMsNzQwLjM0MjYzIHogIiBpZD0icGF0aDI4OTciIHNvZGlwb2RpOm5vZGV0eXBlcz0iY2NjY2MiIHN0eWxlPSJjb2xvcjpibGFjaztmaWxsOiM5MzZiNTA7ZmlsbC1vcGFjaXR5OjE7ZmlsbC1ydWxlOmV2ZW5vZGQ7c3Ryb2tlOm5vbmU7c3Ryb2tlLXdpZHRoOjAuNjQyNjYzMjRweDtzdHJva2UtbGluZWNhcDpidXR0O3N0cm9rZS1saW5lam9pbjptaXRlcjttYXJrZXI6bm9uZTttYXJrZXItc3RhcnQ6bm9uZTttYXJrZXItbWlkOm5vbmU7bWFya2VyLWVuZDpub25lO3N0cm9rZS1taXRlcmxpbWl0OjQ7c3Ryb2tlLWRhc2hvZmZzZXQ6MDtzdHJva2Utb3BhY2l0eToxO3Zpc2liaWxpdHk6dmlzaWJsZTtkaXNwbGF5OmlubGluZTtvdmVyZmxvdzp2aXNpYmxlIi8+CiAgICAgICAgICA8cGF0aCBkPSJNIDQ0NC40Njk4Myw3NDAuMzQyNjMgTCA0NDUuMzYwNzIsNzQ4LjgzNjY5IEMgNDIzLjc0NDgsNzU0LjgzMTk1IDQxMy44MDM1OSw3NTcuNDgwMSAzOTEuMTAzNDksNzY0LjMwMjYxIEwgMzkxLjEzMjc4LDc1NS42NTg5IEMgNDA3LjI2NDA0LDc1MC43NzU1NSA0MDguODg2OTMsNzUxLjU5NDgxIDQ0NC40Njk4Myw3NDAuMzQyNjMgeiAiIGlkPSJwYXRoMjg5OSIgc29kaXBvZGk6bm9kZXR5cGVzPSJjY2NjYyIgc3R5bGU9ImNvbG9yOmJsYWNrO2ZpbGw6IzkzNmI1MDtmaWxsLW9wYWNpdHk6MTtmaWxsLXJ1bGU6ZXZlbm9kZDtzdHJva2U6bm9uZTtzdHJva2Utd2lkdGg6MC42NDI2NjMyNHB4O3N0cm9rZS1saW5lY2FwOmJ1dHQ7c3Ryb2tlLWxpbmVqb2luOm1pdGVyO21hcmtlcjpub25lO21hcmtlci1zdGFydDpub25lO21hcmtlci1taWQ6bm9uZTttYXJrZXItZW5kOm5vbmU7c3Ryb2tlLW1pdGVybGltaXQ6NDtzdHJva2UtZGFzaG9mZnNldDowO3N0cm9rZS1vcGFjaXR5OjE7dmlzaWJpbGl0eTp2aXNpYmxlO2Rpc3BsYXk6aW5saW5lO292ZXJmbG93OnZpc2libGUiLz4KICAgICAgICA8L2c+CiAgICAgICAgPGcgaWQ9ImcyOTAxIiB0cmFuc2Zvcm09InRyYW5zbGF0ZSgwLC0yMjAuMjEzMikiPgogICAgICAgICAgPHBhdGggZD0iTSAzMTYuMTE2NTMsNjIyLjYwMTkzIEMgMjk3LjI3MDk5LDYyMi43MjQyIDI3Ny42Njg2MSw2MjUuMzY0NTggMjU0Ljc3Mjc4LDYyOC4yMjY5MyBMIDIxOS45OTE1Myw3NDAuNjk1NjggQyAyNzcuODY0NjcsNzI2Ljc0MzY3IDMxNy43Njg3NCw3MzguODU3NjYgMzkwLjQyOTAzLDc2MC42OTU2OCBMIDM5MC4zMzUyOCw2MzkuNjY0NDMgQyAzNjMuMzA4NSw2MjYuNDcxNTMgMzQwLjM0NjUxLDYyMi40NDQ3MiAzMTYuMTE2NTMsNjIyLjYwMTkzIHogTSA0NjEuNjc5MDMsNjIyLjYwMTkzIEMgNDM4LjkxMyw2MjIuOTA4MDYgNDE3LjAxNjgzLDYyNy4yMDQ0NyAzOTEuNDkxNTMsNjM5LjY2NDQzIEwgMzkxLjQyOTAzLDc2MC42OTU2OCBDIDQ2NC4wODkzMiw3MzguODU3NjYgNTAzLjk5MzQsNzI2Ljc0MzY3IDU2MS44NjY1Myw3NDAuNjk1NjggTCA1MjcuMDg1MjgsNjI4LjIyNjkzIEMgNTAyLjU1NDAzLDYyNS4xNjAxMyA0ODEuNzY2Nyw2MjIuMzMxODIgNDYxLjY3OTAzLDYyMi42MDE5MyB6ICIgaWQ9InBhdGgyOTAzIiBpbmtzY2FwZTp0aWxlLWN4PSIzMTAuNjg3NSIgaW5rc2NhcGU6dGlsZS1jeT0iNDAxLjg4MTY3IiBpbmtzY2FwZTp0aWxlLWg9IjEzOC4xMTE2NSIgaW5rc2NhcGU6dGlsZS13PSIzNDEuODc1IiBzdHlsZT0iY29sb3I6YmxhY2s7ZmlsbDojZTRkOGFhO2ZpbGwtb3BhY2l0eToxO2ZpbGwtcnVsZTpldmVub2RkO3N0cm9rZTpub25lO3N0cm9rZS13aWR0aDowLjY0MjY2MzI0cHg7c3Ryb2tlLWxpbmVjYXA6YnV0dDtzdHJva2UtbGluZWpvaW46bWl0ZXI7bWFya2VyOm5vbmU7bWFya2VyLXN0YXJ0Om5vbmU7bWFya2VyLW1pZDpub25lO21hcmtlci1lbmQ6bm9uZTtzdHJva2UtbWl0ZXJsaW1pdDo0O3N0cm9rZS1kYXNob2Zmc2V0OjA7c3Ryb2tlLW9wYWNpdHk6MTt2aXNpYmlsaXR5OnZpc2libGU7ZGlzcGxheTppbmxpbmU7b3ZlcmZsb3c6dmlzaWJsZSIvPgogICAgICAgICAgPHBhdGggZD0iTSAzMTYuMTE2NTMsNjE2LjY4NDYgQyAyOTcuMjcwOTksNjE2LjgwNjg3IDI3Ny42Njg2MSw2MTkuNDQ3MjUgMjU0Ljc3Mjc4LDYyMi4zMDk2IEwgMjE5Ljk5MTUzLDczNC43NzgzNSBDIDI3Ny44NjQ2Nyw3MjAuODI2MzQgMzE3Ljc2ODc0LDczMi45NDAzMyAzOTAuNDI5MDMsNzU0Ljc3ODM1IEwgMzkwLjMzNTI4LDYzMy43NDcxIEMgMzYzLjMwODUsNjIwLjU1NDIgMzQwLjM0NjUxLDYxNi41MjczOSAzMTYuMTE2NTMsNjE2LjY4NDYgeiBNIDQ2MS42NzkwMyw2MTYuNjg0NiBDIDQzOC45MTMsNjE2Ljk5MDczIDQxNy4wMTY4Myw2MjEuMjg3MTQgMzkxLjQ5MTUzLDYzMy43NDcxIEwgMzkxLjQyOTAzLDc1NC43NzgzNSBDIDQ2NC4wODkzMiw3MzIuOTQwMzMgNTAzLjk5MzQsNzIwLjgyNjM0IDU2MS44NjY1Myw3MzQuNzc4MzUgTCA1MjcuMDg1MjgsNjIyLjMwOTYgQyA1MDIuNTU0MDMsNjE5LjI0MjggNDgxLjc2NjcsNjE2LjQxNDQ5IDQ2MS42NzkwMyw2MTYuNjg0NiB6ICIgaWQ9InBhdGgyOTA1IiBpbmtzY2FwZTp0aWxlLWN4PSIzMTAuNjg3NSIgaW5rc2NhcGU6dGlsZS1jeT0iNDAxLjg4MTY3IiBpbmtzY2FwZTp0aWxlLWg9IjEzOC4xMTE2NSIgaW5rc2NhcGU6dGlsZS13PSIzNDEuODc1IiBzdHlsZT0iY29sb3I6YmxhY2s7ZmlsbDp1cmwoI2xpbmVhckdyYWRpZW50NDAwNSk7ZmlsbC1vcGFjaXR5OjE7ZmlsbC1ydWxlOmV2ZW5vZGQ7c3Ryb2tlOm5vbmU7c3Ryb2tlLXdpZHRoOjAuNjQyNjYzMjRweDtzdHJva2UtbGluZWNhcDpidXR0O3N0cm9rZS1saW5lam9pbjptaXRlcjttYXJrZXI6bm9uZTttYXJrZXItc3RhcnQ6bm9uZTttYXJrZXItbWlkOm5vbmU7bWFya2VyLWVuZDpub25lO3N0cm9rZS1taXRlcmxpbWl0OjQ7c3Ryb2tlLWRhc2hvZmZzZXQ6MDtzdHJva2Utb3BhY2l0eToxO3Zpc2liaWxpdHk6dmlzaWJsZTtkaXNwbGF5OmlubGluZTtvdmVyZmxvdzp2aXNpYmxlIi8+CiAgICAgICAgPC9nPgogICAgICA8L2c+CiAgICAgIDxwYXRoIGQ9Ik0gNDY3LjI5ODUxLDUwNi40MTMwMyBDIDQ3NC4xODQwMSw1MjIuNDM2MjkgNDc0LjQ4MDcxLDUzMy4zMjYzMiA0ODIuMzIyMDEsNTQ5LjQ0MTkyIEMgNDU5LjIzNzIxLDU1My4yMDQ0NiA0NDguMTY3OTEsNTQ2LjgzMTc4IDQxNi40NTU4MSw1NTMuOTkyMjYgQyA0NDQuMTgwNzEsNTE2LjE4NDkyIDQ0OC45MDg3MSw1MTcuNDAzNSA0NjcuMjk4NTEsNTA2LjQxMzAzIHogIiBpZD0icGF0aDI5NTEiIHNvZGlwb2RpOm5vZGV0eXBlcz0iY2NjYyIgc3R5bGU9ImZpbGw6dXJsKCNsaW5lYXJHcmFkaWVudDQwMDcpO2ZpbGwtb3BhY2l0eToxO2ZpbGwtcnVsZTpldmVub2RkO3N0cm9rZTpub25lO3N0cm9rZS13aWR0aDoxcHg7c3Ryb2tlLWxpbmVjYXA6YnV0dDtzdHJva2UtbGluZWpvaW46bWl0ZXI7c3Ryb2tlLW9wYWNpdHk6MSIvPgogICAgPC9nPgogIDwvZz4KPC9zdmc+\"></div>\n"
  "<div style=\"text-align: center;\">\n"
  "<h1>Setup</h1>\n"
  "<h2>... upgrading ...</h2>\n";
  // The above signature (... upgrading ...) is used elsewhere and should match with this for proper operation.

  // Visual progress indicator.
  // The progress bar restarts when it is near the end and the device is not yet ready initializing.
  // The user will understand that the device is still busy, and not think that it is not stuck.
  config_globals_setup_progress += 5;
  if (config_globals_setup_progress > 90) config_globals_setup_progress = 10;
  notice.append ("<p><progress value=\"" + convert_to_string (config_globals_setup_progress) + "\" max=\"100\"></progress></p>");
  
  // Informative text for progress indication.
  notice.append ("<p>... " + config_globals_setup_message + " ...</p>");
  
  // Remainder of the setup page.
  notice.append (
  "<p>Please sit back and keep the device on till the upgrade has completed.</p>\n"
  "</div>\n"
  "</body>\n"
  "</html>\n"
  );

  return notice.c_str();
}


string setup_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  Assets_View view;

  // Get the existing Administrators.
  vector <string> admins = request->database_users ()->getAdministrators ();

  // Admins do not yet exist: Allow to enter an admin.
  if (admins.empty ()) {
    if (!request->post ["Submit"].empty ()) {
      string admin_username = request->post ["admin_username"];
      string admin_password = request->post ["admin_password"];
      string admin_email = request->post ["admin_email"];
      vector <string> errors;
      if (admin_username.length() < 5) errors.push_back ("Choose a longer username.");
      if (admin_password.length() < 7) errors.push_back ("Choose a longer password.");
      if (admin_email.length() < 5) errors.push_back ("Enter a valid email address.");
      if (errors.empty()) {
        // Store admin details.
        setup_set_admin_details (admin_username, admin_password, admin_email);
        setup_complete_gui ();
        // Store web site's base URL.
        string siteUrl = get_base_url (request);
        Database_Config_General::setSiteURL (siteUrl);
        // Redirect.
        redirect_browser (request, index_index_url ());
      } else {
        view.enable_zone ("errors");
        view.set_variable ("error", filter_string_implode (errors, " "));
      }
    }
  }

  // Enable appropriate zones: Either enter admin's details, or else display the details.
  if (admins.empty ()) {
    view.enable_zone ("enteradmin");
    view.enable_zone ("enteruser");
    view.enable_zone ("enterpass");
    view.enable_zone ("entermail");
    view.enable_zone ("displaysubmit");
  } else {
    string usernames;
    string emails;
    for (unsigned int i = 0; i < admins.size(); i++) {
      if (i) {
        usernames.append (" / ");
        emails.append (" / ");
      }
      usernames.append (admins[i]);
      emails.append (request->database_users ()->get_email (admins[i]));
    }
    view.set_variable ("usernames", usernames);
    view.set_variable ("emails", emails);
    view.enable_zone ("displayok");
    view.set_variable ("readonly", "readonly");
    // If the admin's are already there, then the setup has completed.
    // The automatic page refresh will kick in, and navigate to the main screen.
    Database_Config_General::setInstalledInterfaceVersion (config_logic_version ());
  }

  return view.render ("setup", "index");
}


