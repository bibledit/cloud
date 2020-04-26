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


#ifndef INCLUDED_DATABASE_CONFIG_GENERAL_H
#define INCLUDED_DATABASE_CONFIG_GENERAL_H


#include <config/libraries.h>
#include <webserver/request.h>


class Database_Config_General
{
public:
  static string getSiteMailName();
  static void setSiteMailName (string value);

  static string getSiteMailAddress();
  static void setSiteMailAddress (string value);
  
  static string getMailStorageHost();
  static void setMailStorageHost (string value);
  
  static string getMailStorageUsername();
  static void setMailStorageUsername (string value);
  
  static string getMailStoragePassword();
  static void setMailStoragePassword (string value);
  
  static string getMailStorageProtocol();
  static void setMailStorageProtocol (string value);
  
  static string getMailStoragePort();
  static void setMailStoragePort (string value);
  
  static string getMailSendHost();
  static void setMailSendHost (string value);
  
  static string getMailSendUsername();
  static void setMailSendUsername (string value);
  
  static string getMailSendPassword();
  static void setMailSendPassword (string value);
  
  static string getMailSendPort();
  static void setMailSendPort (string value);
  
  static string getTimerMinute();
  static void setTimerMinute (string value);
  
  static int getTimezone();
  static void setTimezone (int value);
  
  static string getSiteURL();
  static void setSiteURL (string value);
  
  static string getSiteLanguage ();
  static void setSiteLanguage (string value);
  
  static bool getClientMode ();
  static void setClientMode (bool value);
  
  static string getServerAddress ();
  static void setServerAddress (string value);
  
  static int getServerPort ();
  static void setServerPort (int value);
  
  static int getRepeatSendReceive ();
  static void setRepeatSendReceive (int value);
  
  static int getLastSendReceive ();
  static void setLastSendReceive (int value);
  
  static string getInstalledInterfaceVersion ();
  static void setInstalledInterfaceVersion (string value);
  
  static string getInstalledDatabaseVersion ();
  static void setInstalledDatabaseVersion (string value);
  
  static bool getJustStarted ();
  static void setJustStarted (bool value);
  
  static string getParatextProjectsFolder ();
  static void setParatextProjectsFolder (string value);
  
  static string getSyncKey ();
  static void setSyncKey (string key);
  
  static string getLastMenuClick ();
  static void setLastMenuClick (string url);
  
  static vector <string> getResourcesToCache ();
  static void setResourcesToCache (vector <string> values);
  
  static bool getIndexNotes ();
  static void setIndexNotes (bool value);
  
  static bool getIndexBibles ();
  static void setIndexBibles (bool value);
  
  static int getUnsentBibleDataTime ();
  static void setUnsentBibleDataTime (int value);

  static int getUnreceivedBibleDataTime ();
  static void setUnreceivedBibleDataTime (int value);

  static bool getAuthorInRssFeed ();
  static void setAuthorInRssFeed (bool value);

  static bool getJustConnectedToCloud ();
  static void setJustConnectedToCloud (bool value);
  
  static bool getMenuInTabbedViewOn ();
  static void setMenuInTabbedViewOn (bool value);
  static string getMenuInTabbedViewJSON ();
  static void setMenuInTabbedViewJSON (string value);
  
  static bool getDisableSelectionPopupChromeOS ();
  static void setDisableSelectionPopupChromeOS (bool value);

private:
  static string file (const char * key);
  static string getValue (const char * key, const char * default_value);
  static void setValue (const char * key, string value);
  static bool getBValue (const char * key, bool default_value);
  static void setBValue (const char * key, bool value);
  static int getIValue (const char * key, int default_value);
  static void setIValue (const char * key, int value);
  static vector <string> getList (const char * key);
  static void setList (const char * key, vector <string> values);
};


#endif
