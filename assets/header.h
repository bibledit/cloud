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


#ifndef INCLUDED_ASSETS_HEADER_H
#define INCLUDED_ASSETS_HEADER_H


#include <config/libraries.h>
#include <assets/view.h>


class Assets_Header
{
public:
  Assets_Header (string title, void * webserver_request_in);
  ~Assets_Header ();
  void jQueryTouchOn ();
  void touchCSSOn ();
  void notifItOn ();
  void setNavigator ();
  void setStylesheet ();
  void setEditorStylesheet ();
  bool displayTopbar ();
  void refresh (int seconds, string url = "");
  void setFadingMenu (string html);
  void addBreadCrumb (string item, string text);
  string run ();
private:
  Assets_View * view;
  bool includeJQueryTouch;
  bool includeTouchCSS;
  bool includeNotifIt;
  vector <string> headLines;
  bool displayNavigator;
  string includedStylesheet;
  string includedEditorStylesheet;
  void * webserver_request;
  string loginrequest;
  string fadingmenu;
  vector <pair <string, string> > breadcrumbs;
};


#endif
