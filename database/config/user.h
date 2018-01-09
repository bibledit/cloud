/*
Copyright (©) 2003-2017 Teus Benschop.

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


#ifndef INCLUDED_DATABASE_CONFIG_USER_H
#define INCLUDED_DATABASE_CONFIG_USER_H


#include <config/libraries.h>
#include <filter/passage.h>


class Database_Config_User
{
public:
  Database_Config_User (void * webserver_request_in);
  void trim ();
  void remove (string username);
  void clear_cache ();
  string getBible ();
  void setBible (string bible);
  bool getSubscribeToConsultationNotesEditedByMe ();
  void setSubscribeToConsultationNotesEditedByMe (bool value);
  bool getNotifyMeOfAnyConsultationNotesEdits ();
  bool getNotifyUserOfAnyConsultationNotesEdits (string username);
  void setNotifyMeOfAnyConsultationNotesEdits (bool value);
  bool getSubscribedConsultationNoteNotification ();
  bool getUserSubscribedConsultationNoteNotification (string username);
  void setSubscribedConsultationNoteNotification (bool value);
  bool getAssignedToConsultationNotesChanges ();
  bool getUserAssignedToConsultationNotesChanges (string username);
  void setAssignedToConsultationNotesChanges (bool value);
  bool getGenerateChangeNotifications ();
  bool getUserGenerateChangeNotifications (string username);
  void setGenerateChangeNotifications (bool value);
  bool getAssignedConsultationNoteNotification ();
  bool getUserAssignedConsultationNoteNotification (string username);
  void setAssignedConsultationNoteNotification (bool value);
  int getConsultationNotesPassageSelector ();
  void setConsultationNotesPassageSelector (int value);
  int getConsultationNotesEditSelector ();
  void setConsultationNotesEditSelector (int value);
  int getConsultationNotesNonEditSelector ();
  void setConsultationNotesNonEditSelector (int value);
  string getConsultationNotesStatusSelector ();
  void setConsultationNotesStatusSelector (string value);
  string getConsultationNotesBibleSelector ();
  void setConsultationNotesBibleSelector (string value);
  string getConsultationNotesAssignmentSelector ();
  void setConsultationNotesAssignmentSelector (string value);
  bool getConsultationNotesSubscriptionSelector ();
  void setConsultationNotesSubscriptionSelector (bool value);
  int getConsultationNotesSeveritySelector ();
  void setConsultationNotesSeveritySelector (int value);
  int getConsultationNotesTextSelector ();
  void setConsultationNotesTextSelector (int value);
  string getConsultationNotesSearchText ();
  void setConsultationNotesSearchText (string value);
  int getConsultationNotesPassageInclusionSelector ();
  void setConsultationNotesPassageInclusionSelector (int value);
  int getConsultationNotesTextInclusionSelector ();
  void setConsultationNotesTextInclusionSelector (int value);
  bool getBibleChangesNotification ();
  bool getUserBibleChangesNotification (string username);
  void setBibleChangesNotification (bool value);
  bool getDeletedConsultationNoteNotification ();
  bool getUserDeletedConsultationNoteNotification (string username);
  void setDeletedConsultationNoteNotification (bool value);
  bool getBibleChecksNotification ();
  bool getUserBibleChecksNotification (string username);
  void setBibleChecksNotification (bool value);
  bool getPendingChangesNotification ();
  bool getUserPendingChangesNotification (string username);
  void setPendingChangesNotification (bool value);
  bool getUserChangesNotification ();
  bool getUserUserChangesNotification (string username);
  void setUserChangesNotification (bool value);
  bool getAssignedNotesStatisticsNotification ();
  bool getUserAssignedNotesStatisticsNotification (string username);
  void setAssignedNotesStatisticsNotification (bool value);
  bool getSubscribedNotesStatisticsNotification ();
  bool getUserSubscribedNotesStatisticsNotification (string username);
  void setSubscribedNotesStatisticsNotification (bool value);
  bool getNotifyMeOfMyPosts ();
  bool getUserNotifyMeOfMyPosts (string username);
  void setNotifyMeOfMyPosts (bool value);
  bool getSuppressMailFromYourUpdatesNotes ();
  bool getUserSuppressMailFromYourUpdatesNotes (string username);
  void setSuppressMailFromYourUpdatesNotes (bool value);
  vector <string> getActiveResources ();
  void setActiveResources (vector <string> values);
  vector <string> getConsistencyResources ();
  void setConsistencyResources (vector <string> values);
  int getSprintMonth ();
  void setSprintMonth (int value);
  int getSprintYear ();
  void setSprintYear (int value);
  bool getSprintProgressNotification ();
  bool getUserSprintProgressNotification (string username);
  void setSprintProgressNotification (bool value);
  bool getUserChangesNotificationsOnline ();
  bool getUserUserChangesNotificationsOnline (string username);
  void setUserChangesNotificationsOnline (bool value);
  bool getContributorChangesNotificationsOnline ();
  bool getContributorChangesNotificationsOnline (string username);
  void setContributorChangesNotificationsOnline (bool value);
  string getWorkspaceURLs ();
  void setWorkspaceURLs (string value);
  string getWorkspaceWidths ();
  void setWorkspaceWidths (string value);
  string getWorkspaceHeights ();
  void setWorkspaceHeights (string value);
  string getEntireWorkspaceWidths ();
  void setEntireWorkspaceWidths (string value);
  string getActiveWorkspace ();
  void setActiveWorkspace (string value);
  bool getPostponeNewNotesMails ();
  void setPostponeNewNotesMails (bool value);
  string getRecentlyAppliedStyles ();
  void setRecentlyAppliedStyles (string values);
  vector <string> getPrintResources ();
  vector <string> getPrintResourcesForUser (string user);
  void setPrintResources (vector <string> values);
  Passage getPrintPassageFrom ();
  Passage getPrintPassageFromForUser (string user);
  void setPrintPassageFrom (Passage value);
  Passage getPrintPassageTo ();
  Passage getPrintPassageToForUser (string user);
  void setPrintPassageTo (Passage value);
  int getFocusedBook ();
  void setFocusedBook (int book);
  int getFocusedChapter ();
  void setFocusedChapter (int chapter);
  int getFocusedVerse ();
  void setFocusedVerse (int verse);
  vector <int> getUpdatedSettings ();
  void setUpdatedSettings (vector <int> values);
  void addUpdatedSetting (int value);
  void removeUpdatedSetting (int value);
  vector <int> getRemovedChanges ();
  void setRemovedChanges (vector <int> values);
  void addRemovedChange (int value);
  void removeRemovedChange (int value);
  string getChangeNotificationsChecksum ();
  void setChangeNotificationsChecksum (string value);
  void setUserChangeNotificationsChecksum (string user, string value);
  void setLiveBibleEditor (int time);
  int getLiveBibleEditor ();
  void setResourceVersesBefore (int verses);
  int getResourceVersesBefore ();
  void setResourceVersesAfter (int verses);
  int getResourceVersesAfter ();
  string getSyncKey ();
  void setSyncKey (string key);
  void setGeneralFontSize (int size);
  int getGeneralFontSize ();
  void setMenuFontSize (int size);
  int getMenuFontSize ();
  void setResourcesFontSize (int size);
  int getBibleEditorsFontSize ();
  void setBibleEditorsFontSize (int size);
  int getResourcesFontSize ();
  void setHebrewFontSize (int size);
  int getHebrewFontSize ();
  void setGreekFontSize (int size);
  int getGreekFontSize ();
  void setVerticalCaretPosition (int position);
  int getVerticalCaretPosition ();
  bool getDisplayBreadcrumbs ();
  void setDisplayBreadcrumbs (bool value);
  void setWorkspaceMenuFadeoutDelay (int value);
  int getWorkspaceMenuFadeoutDelay ();
  int getEditingAllowedDifferenceChapter ();
  void setEditingAllowedDifferenceChapter (int value);
  int getEditingAllowedDifferenceVerse ();
  void setEditingAllowedDifferenceVerse (int value);
  bool getBasicInterfaceModeDefault ();
  bool getBasicInterfaceMode ();
  void setBasicInterfaceMode (bool value);
  bool getMainMenuAlwaysVisible ();
  void setMainMenuAlwaysVisible (bool value);
  bool getSwipeActionsAvailable ();
  void setSwipeActionsAvailable (bool value);
  bool getFastEditorSwitchingAvailable ();
  void setFastEditorSwitchingAvailable (bool value);
  bool getIncludeRelatedPassages ();
  void setIncludeRelatedPassages (bool value);
  int getFastSwitchVisualEditors ();
  void setFastSwitchVisualEditors (int value);
  int getFastSwitchUsfmEditors ();
  void setFastSwitchUsfmEditors (int value);
  bool getEnableStylesButtonVisualEditors ();
  void setEnableStylesButtonVisualEditors (bool value);
  bool getMenuChangesInBasicMode ();
  void setMenuChangesInBasicMode (bool value);
  bool getPrivilegeUseAdvancedMode ();
  bool getPrivilegeUseAdvancedModeForUser (string username);
  void setPrivilegeUseAdvancedModeForUser (string username, bool value);
  bool getPrivilegeDeleteConsultationNotes ();
  void setPrivilegeDeleteConsultationNotes (bool value);
  bool getPrivilegeDeleteConsultationNotesForUser (string username);
  void setPrivilegeDeleteConsultationNotesForUser (string username, bool value);
  bool getDismissChangesAtTop ();
  void setDismissChangesAtTop (bool value);
  bool getQuickNoteEditLink ();
  void setQuickNoteEditLink (bool value);
  bool getShowBibleInNotesList ();
  void setShowBibleInNotesList (bool value);
private:
  void * webserver_request;
  string file (string user);
  string file (string user, const char * key);
  string mapkey (string user, const char * key);
  string getValue (const char * key, const char * default_value);
  bool getBValue (const char * key, bool default_value);
  int getIValue (const char * key, int default_value);
  string getValueForUser (string user, const char * key, const char * default_value);
  bool getBValueForUser (string user, const char * key, bool default_value);
  int getIValueForUser (string user, const char * key, int default_value);
  void setValue (const char * key, string value);
  void setBValue (const char * key, bool value);
  void setIValue (const char * key, int value);
  void setValueForUser (string user, const char * key, string value);
  void setBValueForUser (string user, const char * key, bool value);
  vector <string> getList (const char * key);
  vector <string> getListForUser (string user, const char * key);
  void setList (const char * key, vector <string> values);
  void setListForUser (string user, const char * key, vector <string> values);
  vector <int> getIList (const char * key);
  void setIList (const char * key, vector <int> values);
  const char * keySprintMonth ();
  const char * keySprintYear ();
  bool defaultBibleChecksNotification ();
};


#endif
