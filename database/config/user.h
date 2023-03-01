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
#include <filter/passage.h>

class Database_Config_User
{
public:
  Database_Config_User (void * webserver_request_in);
  void trim ();
  void remove (std::string username);
  void clear_cache ();
  std::string getBible ();
  void setBible (std::string bible);
  bool getSubscribeToConsultationNotesEditedByMe ();
  void setSubscribeToConsultationNotesEditedByMe (bool value);
  bool getNotifyMeOfAnyConsultationNotesEdits ();
  bool getNotifyUserOfAnyConsultationNotesEdits (std::string username);
  void setNotifyMeOfAnyConsultationNotesEdits (bool value);
  bool getSubscribedConsultationNoteNotification ();
  bool getUserSubscribedConsultationNoteNotification (std::string username);
  void setSubscribedConsultationNoteNotification (bool value);
  bool getAssignedToConsultationNotesChanges ();
  bool getUserAssignedToConsultationNotesChanges (std::string username);
  void setAssignedToConsultationNotesChanges (bool value);
  bool getGenerateChangeNotifications ();
  bool getUserGenerateChangeNotifications (std::string username);
  void setGenerateChangeNotifications (bool value);
  bool getAssignedConsultationNoteNotification ();
  bool getUserAssignedConsultationNoteNotification (std::string username);
  void setAssignedConsultationNoteNotification (bool value);
  int getConsultationNotesPassageSelector ();
  void setConsultationNotesPassageSelector (int value);
  int getConsultationNotesEditSelector ();
  void setConsultationNotesEditSelector (int value);
  int getConsultationNotesNonEditSelector ();
  void setConsultationNotesNonEditSelector (int value);
  std::string getConsultationNotesStatusSelector ();
  void setConsultationNotesStatusSelector (std::string value);
  std::string getConsultationNotesBibleSelector ();
  void setConsultationNotesBibleSelector (std::string value);
  std::string getConsultationNotesAssignmentSelector ();
  void setConsultationNotesAssignmentSelector (std::string value);
  bool getConsultationNotesSubscriptionSelector ();
  void setConsultationNotesSubscriptionSelector (bool value);
  int getConsultationNotesSeveritySelector ();
  void setConsultationNotesSeveritySelector (int value);
  int getConsultationNotesTextSelector ();
  void setConsultationNotesTextSelector (int value);
  std::string getConsultationNotesSearchText ();
  void setConsultationNotesSearchText (std::string value);
  int getConsultationNotesPassageInclusionSelector ();
  void setConsultationNotesPassageInclusionSelector (int value);
  int getConsultationNotesTextInclusionSelector ();
  void setConsultationNotesTextInclusionSelector (int value);
  bool getBibleChangesNotification ();
  bool getUserBibleChangesNotification (std::string username);
  void setBibleChangesNotification (bool value);
  bool getDeletedConsultationNoteNotification ();
  bool getUserDeletedConsultationNoteNotification (std::string username);
  void setDeletedConsultationNoteNotification (bool value);
  bool getBibleChecksNotification ();
  bool getUserBibleChecksNotification (std::string username);
  void setBibleChecksNotification (bool value);
  bool getPendingChangesNotification ();
  bool getUserPendingChangesNotification (std::string username);
  void setPendingChangesNotification (bool value);
  bool getUserChangesNotification ();
  bool getUserUserChangesNotification (std::string username);
  void setUserChangesNotification (bool value);
  bool getAssignedNotesStatisticsNotification ();
  bool getUserAssignedNotesStatisticsNotification (std::string username);
  void setAssignedNotesStatisticsNotification (bool value);
  bool getSubscribedNotesStatisticsNotification ();
  bool getUserSubscribedNotesStatisticsNotification (std::string username);
  void setSubscribedNotesStatisticsNotification (bool value);
  bool getNotifyMeOfMyPosts ();
  bool getUserNotifyMeOfMyPosts (std::string username);
  void setNotifyMeOfMyPosts (bool value);
  bool getSuppressMailFromYourUpdatesNotes ();
  bool getUserSuppressMailFromYourUpdatesNotes (std::string username);
  void setSuppressMailFromYourUpdatesNotes (bool value);
  std::vector <std::string> getActiveResources ();
  void setActiveResources (std::vector <std::string> values);
  std::vector <std::string> getConsistencyResources ();
  void setConsistencyResources (std::vector <std::string> values);
  int getSprintMonth ();
  void setSprintMonth (int value);
  int getSprintYear ();
  void setSprintYear (int value);
  bool getSprintProgressNotification ();
  bool getUserSprintProgressNotification (std::string username);
  void setSprintProgressNotification (bool value);
  bool getUserChangesNotificationsOnline ();
  bool getUserUserChangesNotificationsOnline (std::string username);
  void setUserChangesNotificationsOnline (bool value);
  bool getContributorChangesNotificationsOnline ();
  bool getContributorChangesNotificationsOnline (std::string username);
  void setContributorChangesNotificationsOnline (bool value);
  std::string getWorkspaceURLs ();
  void setWorkspaceURLs (std::string value);
  std::string getWorkspaceWidths ();
  void setWorkspaceWidths (std::string value);
  std::string getWorkspaceHeights ();
  void setWorkspaceHeights (std::string value);
  std::string getEntireWorkspaceWidths ();
  void setEntireWorkspaceWidths (std::string value);
  std::string getActiveWorkspace ();
  void setActiveWorkspace (std::string value);
  bool getPostponeNewNotesMails ();
  void setPostponeNewNotesMails (bool value);
  std::string getRecentlyAppliedStyles ();
  void setRecentlyAppliedStyles (std::string values);
  std::vector <std::string> getPrintResources ();
  std::vector <std::string> getPrintResourcesForUser (std::string user);
  void setPrintResources (std::vector <std::string> values);
  Passage getPrintPassageFrom ();
  Passage getPrintPassageFromForUser (std::string user);
  void setPrintPassageFrom (Passage value);
  Passage getPrintPassageTo ();
  Passage getPrintPassageToForUser (std::string user);
  void setPrintPassageTo (Passage value);
  int getFocusedBook ();
  void setFocusedBook (int book);
  int getFocusedChapter ();
  void setFocusedChapter (int chapter);
  int getFocusedVerse ();
  void setFocusedVerse (int verse);
  std::vector <int> getUpdatedSettings ();
  void setUpdatedSettings (std::vector <int> values);
  void addUpdatedSetting (int value);
  void removeUpdatedSetting (int value);
  std::vector <int> getRemovedChanges ();
  void setRemovedChanges (std::vector <int> values);
  void addRemovedChange (int value);
  void removeRemovedChange (int value);
  std::string getChangeNotificationsChecksum ();
  void setChangeNotificationsChecksum (std::string value);
  void setUserChangeNotificationsChecksum (std::string user, std::string value);
  void setLiveBibleEditor (int time);
  int getLiveBibleEditor ();
  void setResourceVersesBefore (int verses);
  int getResourceVersesBefore ();
  void setResourceVersesAfter (int verses);
  int getResourceVersesAfter ();
  std::string getSyncKey ();
  void setSyncKey (std::string key);
//  std::string getSiteLanguage ();
//  void setSiteLanguage (std::string value);
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
  void setCurrentTheme (int index);
  int getCurrentTheme ();
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
  bool getPrivilegeUseAdvancedModeForUser (std::string username);
  void setPrivilegeUseAdvancedModeForUser (std::string username, bool value);
  bool getPrivilegeDeleteConsultationNotes ();
  void setPrivilegeDeleteConsultationNotes (bool value);
  bool getPrivilegeDeleteConsultationNotesForUser (std::string username);
  void setPrivilegeDeleteConsultationNotesForUser (std::string username, bool value);
  bool getPrivilegeSetStylesheets ();
  bool getPrivilegeSetStylesheetsForUser (std::string username);
  void setPrivilegeSetStylesheetsForUser (std::string username, bool value);
  bool getDismissChangesAtTop ();
  void setDismissChangesAtTop (bool value);
  bool getQuickNoteEditLink ();
  void setQuickNoteEditLink (bool value);
  bool getShowBibleInNotesList ();
  void setShowBibleInNotesList (bool value);
  bool getShowNoteStatus ();
  void setShowNoteStatus (bool value);
  bool getShowVerseTextAtCreateNote ();
  void setShowVerseTextAtCreateNote (bool value);
  bool getOrderChangesByAuthor ();
  void setOrderChangesByAuthor (bool value);
  std::vector <std::string> getAutomaticNoteAssignment ();
  void setAutomaticNoteAssignment (std::vector <std::string> values);
  bool getReceiveFocusedReferenceFromParatext ();
  void setReceiveFocusedReferenceFromParatext (bool value);
  bool getReceiveFocusedReferenceFromAccordance ();
  void setReceiveFocusedReferenceFromAccordance (bool value);
  bool getUseColoredNoteStatusLabels ();
  void setUseColoredNoteStatusLabels (bool value);
  int getNotesDateFormat ();
  void setNotesDateFormat (int value);
  std::vector <std::string> getChangeNotificationsBibles ();
  std::vector <std::string> getChangeNotificationsBiblesForUser (const std::string & user);
  void setChangeNotificationsBibles (const std::vector <std::string>& values);
private:
  void * webserver_request {nullptr};
  std::string file (std::string user);
  std::string file (std::string user, const char * key);
  std::string mapkey (std::string user, const char * key);
  std::string getValue (const char * key, const char * default_value);
  bool getBValue (const char * key, bool default_value);
  int getIValue (const char * key, int default_value);
  std::string getValueForUser (std::string user, const char * key, const char * default_value);
  bool getBValueForUser (std::string user, const char * key, bool default_value);
  int getIValueForUser (std::string user, const char * key, int default_value);
  void setValue (const char * key, std::string value);
  void setBValue (const char * key, bool value);
  void setIValue (const char * key, int value);
  void setValueForUser (std::string user, const char * key, std::string value);
  void setBValueForUser (std::string user, const char * key, bool value);
  std::vector <std::string> getList (const char * key);
  std::vector <std::string> getListForUser (std::string user, const char * key);
  void setList (const char * key, std::vector <std::string> values);
  void setListForUser (std::string user, const char * key, std::vector <std::string> values);
  std::vector <int> getIList (const char * key);
  void setIList (const char * key, std::vector <int> values);
  const char * keySprintMonth ();
  const char * keySprintYear ();
  bool defaultBibleChecksNotification ();
};
