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


#pragma once

#include <config/libraries.h>
#include <webserver/request.h>

namespace database::config::general {

std::string getSiteMailName();
void setSiteMailName (std::string value);

std::string getSiteMailAddress();
void setSiteMailAddress (std::string value);

std::string getMailStorageHost();
void setMailStorageHost (std::string value);

std::string getMailStorageUsername();
void setMailStorageUsername (std::string value);

std::string getMailStoragePassword();
void setMailStoragePassword (std::string value);

std::string getMailStorageProtocol();
void setMailStorageProtocol (std::string value);

std::string getMailStoragePort();
void setMailStoragePort (std::string value);

std::string getMailSendHost();
void setMailSendHost (std::string value);

std::string getMailSendUsername();
void setMailSendUsername (std::string value);

std::string getMailSendPassword();
void setMailSendPassword (std::string value);

std::string getMailSendPort();
void setMailSendPort (std::string value);

std::string getTimerMinute();
void setTimerMinute (std::string value);

int getTimezone();
void setTimezone (int value);

std::string getSiteURL();
void setSiteURL (std::string value);

std::string getSiteLanguage ();
void setSiteLanguage (std::string value);

bool getClientMode ();
void setClientMode (bool value);

std::string getServerAddress ();
void setServerAddress (std::string value);

int getServerPort ();
void setServerPort (int value);

int getRepeatSendReceive ();
void setRepeatSendReceive (int value);

int getLastSendReceive ();
void setLastSendReceive (int value);

std::string getInstalledInterfaceVersion ();
void setInstalledInterfaceVersion (std::string value);

std::string getInstalledDatabaseVersion ();
void setInstalledDatabaseVersion (std::string value);

bool getJustStarted ();
void setJustStarted (bool value);

std::string getParatextProjectsFolder ();
void setParatextProjectsFolder (std::string value);

std::string getSyncKey ();
void setSyncKey (std::string key);

std::string getLastMenuClick ();
void setLastMenuClick (std::string url);

std::vector <std::string> getResourcesToCache ();
void setResourcesToCache (std::vector <std::string> values);

bool getIndexNotes ();
void setIndexNotes (bool value);

bool getIndexBibles ();
void setIndexBibles (bool value);

int getUnsentBibleDataTime ();
void setUnsentBibleDataTime (int value);

int getUnreceivedBibleDataTime ();
void setUnreceivedBibleDataTime (int value);

bool getAuthorInRssFeed ();
void setAuthorInRssFeed (bool value);

bool getJustConnectedToCloud ();
void setJustConnectedToCloud (bool value);

bool getMenuInTabbedViewOn ();
void setMenuInTabbedViewOn (bool value);
std::string getMenuInTabbedViewJSON ();
void setMenuInTabbedViewJSON (std::string value);

bool getDisableSelectionPopupChromeOS ();
void setDisableSelectionPopupChromeOS (bool value);

std::string getNotesVerseSeparator ();
void setNotesVerseSeparator (std::string url);

std::vector <std::string> getComparativeResources ();
void setComparativeResources (std::vector <std::string> values);

std::vector <std::string> getTranslatedResources ();
void setTranslatedResources (std::vector <std::string> values);

std::vector <std::string> getDefaultActiveResources ();
void setDefaultActiveResources (std::vector <std::string> values);

std::vector <std::string> getAccountCreationTimes ();
void setAccountCreationTimes (std::vector <std::string> values);

bool getKeepResourcesCacheForLong ();
void setKeepResourcesCacheForLong (bool value);

int getDefaultNewUserAccessLevel ();
void setDefaultNewUserAccessLevel (int value);

bool getKeepOsisContentInSwordResources ();
void setKeepOsisContentInSwordResources (bool value);

}
