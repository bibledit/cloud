/*
Copyright (©) 2003-2023 Teus Benschop.

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


#pragma once

#include <config/libraries.h>
#include <webserver/request.h>

class Database_Config_General
{
public:
  static std::string getSiteMailName();
  static void setSiteMailName (std::string value);

  static std::string getSiteMailAddress();
  static void setSiteMailAddress (std::string value);
  
  static std::string getMailStorageHost();
  static void setMailStorageHost (std::string value);
  
  static std::string getMailStorageUsername();
  static void setMailStorageUsername (std::string value);
  
  static std::string getMailStoragePassword();
  static void setMailStoragePassword (std::string value);
  
  static std::string getMailStorageProtocol();
  static void setMailStorageProtocol (std::string value);
  
  static std::string getMailStoragePort();
  static void setMailStoragePort (std::string value);
  
  static std::string getMailSendHost();
  static void setMailSendHost (std::string value);
  
  static std::string getMailSendUsername();
  static void setMailSendUsername (std::string value);
  
  static std::string getMailSendPassword();
  static void setMailSendPassword (std::string value);
  
  static std::string getMailSendPort();
  static void setMailSendPort (std::string value);
  
  static std::string getTimerMinute();
  static void setTimerMinute (std::string value);
  
  static int getTimezone();
  static void setTimezone (int value);
  
  static std::string getSiteURL();
  static void setSiteURL (std::string value);
  
  static std::string getSiteLanguage ();
  static void setSiteLanguage (std::string value);
  
  static bool getClientMode ();
  static void setClientMode (bool value);
  
  static std::string getServerAddress ();
  static void setServerAddress (std::string value);
  
  static int getServerPort ();
  static void setServerPort (int value);
  
  static int getRepeatSendReceive ();
  static void setRepeatSendReceive (int value);
  
  static int getLastSendReceive ();
  static void setLastSendReceive (int value);
  
  static std::string getInstalledInterfaceVersion ();
  static void setInstalledInterfaceVersion (std::string value);
  
  static std::string getInstalledDatabaseVersion ();
  static void setInstalledDatabaseVersion (std::string value);
  
  static bool getJustStarted ();
  static void setJustStarted (bool value);
  
  static std::string getParatextProjectsFolder ();
  static void setParatextProjectsFolder (std::string value);
  
  static std::string getSyncKey ();
  static void setSyncKey (std::string key);
  
  static std::string getLastMenuClick ();
  static void setLastMenuClick (std::string url);
  
  static std::vector <std::string> getResourcesToCache ();
  static void setResourcesToCache (std::vector <std::string> values);
  
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
  static std::string getMenuInTabbedViewJSON ();
  static void setMenuInTabbedViewJSON (std::string value);
  
  static bool getDisableSelectionPopupChromeOS ();
  static void setDisableSelectionPopupChromeOS (bool value);

  static std::string getNotesVerseSeparator ();
  static void setNotesVerseSeparator (std::string url);

  static std::vector <std::string> getComparativeResources ();
  static void setComparativeResources (std::vector <std::string> values);

  static std::vector <std::string> getTranslatedResources ();
  static void setTranslatedResources (std::vector <std::string> values);

  static std::vector <std::string> getDefaultActiveResources ();
  static void setDefaultActiveResources (std::vector <std::string> values);

  static std::vector <std::string> getAccountCreationTimes ();
  static void setAccountCreationTimes (std::vector <std::string> values);

  static bool getKeepResourcesCacheForLong ();
  static void setKeepResourcesCacheForLong (bool value);

  static int getDefaultNewUserAccessLevel ();
  static void setDefaultNewUserAccessLevel (int value);

private:
  static std::string file (const char * key);
  static std::string getValue (const char * key, const char * default_value);
  static void setValue (const char * key, std::string value);
  static bool getBValue (const char * key, bool default_value);
  static void setBValue (const char * key, bool value);
  static int getIValue (const char * key, int default_value);
  static void setIValue (const char * key, int value);
  static std::vector <std::string> getList (const char * key);
  static void setList (const char * key, std::vector <std::string> values);
};
