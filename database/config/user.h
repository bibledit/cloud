/*
Copyright (©) 2003-2025 Teus Benschop.

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

class Webserver_Request;

class Database_Config_User
{
public:
  Database_Config_User (Webserver_Request& webserver_request);
  void trim () const;
  void remove (const std::string& username) const;
  void clear_cache () const;
  std::string getBible () const;
  void setBible (const std::string& bible) const;
  bool getSubscribeToConsultationNotesEditedByMe () const;
  void setSubscribeToConsultationNotesEditedByMe (bool value) const;
  bool getNotifyMeOfAnyConsultationNotesEdits () const;
  bool getNotifyUserOfAnyConsultationNotesEdits (const std::string& username) const;
  void setNotifyMeOfAnyConsultationNotesEdits (bool value) const;
  bool getSubscribedConsultationNoteNotification () const;
  bool getUserSubscribedConsultationNoteNotification (const std::string& username) const;
  void setSubscribedConsultationNoteNotification (bool value) const;
  bool getAssignedToConsultationNotesChanges () const;
  bool getUserAssignedToConsultationNotesChanges (const std::string& username) const;
  void setAssignedToConsultationNotesChanges (bool value) const;
  bool getGenerateChangeNotifications () const;
  bool getUserGenerateChangeNotifications (const std::string& username) const;
  void setGenerateChangeNotifications (bool value) const;
  bool getAssignedConsultationNoteNotification () const ;
  bool getUserAssignedConsultationNoteNotification (const std::string& username) const;
  void setAssignedConsultationNoteNotification (bool value) const;
  int getConsultationNotesPassageSelector () const;
  void setConsultationNotesPassageSelector (int value) const;
  int getConsultationNotesEditSelector () const;
  void setConsultationNotesEditSelector (int value) const;
  int getConsultationNotesNonEditSelector () const;
  void setConsultationNotesNonEditSelector (int value) const;
  std::string getConsultationNotesStatusSelector () const;
  void setConsultationNotesStatusSelector (const std::string& value) const;
  std::string getConsultationNotesBibleSelector () const;
  void setConsultationNotesBibleSelector (const std::string& value) const;
  std::string getConsultationNotesAssignmentSelector () const;
  void setConsultationNotesAssignmentSelector (const std::string& value) const;
  bool getConsultationNotesSubscriptionSelector () const;
  void setConsultationNotesSubscriptionSelector (bool value) const;
  int getConsultationNotesSeveritySelector () const;
  void setConsultationNotesSeveritySelector (int value) const;
  int getConsultationNotesTextSelector () const;
  void setConsultationNotesTextSelector (int value) const;
  std::string getConsultationNotesSearchText () const;
  void setConsultationNotesSearchText (const std::string& value) const;
  int getConsultationNotesPassageInclusionSelector () const;
  void setConsultationNotesPassageInclusionSelector (int value) const;
  int getConsultationNotesTextInclusionSelector () const;
  void setConsultationNotesTextInclusionSelector (int value) const;
  bool getBibleChangesNotification () const;
  bool getUserBibleChangesNotification (const std::string& username) const;
  void setBibleChangesNotification (bool value) const;
  bool getDeletedConsultationNoteNotification () const;
  bool getUserDeletedConsultationNoteNotification (const std::string& username) const;
  void setDeletedConsultationNoteNotification (bool value) const;
  bool getBibleChecksNotification () const;
  bool getUserBibleChecksNotification (const std::string& username) const;
  void setBibleChecksNotification (bool value) const;
  bool getPendingChangesNotification () const;
  bool getUserPendingChangesNotification (const std::string& username) const;
  void setPendingChangesNotification (bool value) const;
  bool getUserChangesNotification () const;
  bool getUserUserChangesNotification (const std::string& username) const;
  void setUserChangesNotification (bool value) const;
  bool getAssignedNotesStatisticsNotification () const;
  bool getUserAssignedNotesStatisticsNotification (const std::string& username) const;
  void setAssignedNotesStatisticsNotification (bool value) const;
  bool getSubscribedNotesStatisticsNotification () const;
  bool getUserSubscribedNotesStatisticsNotification (const std::string& username) const;
  void setSubscribedNotesStatisticsNotification (bool value) const;
  bool getNotifyMeOfMyPosts () const;
  bool getUserNotifyMeOfMyPosts (const std::string& username) const;
  void setNotifyMeOfMyPosts (bool value) const;
  bool getSuppressMailFromYourUpdatesNotes () const;
  bool getUserSuppressMailFromYourUpdatesNotes (const std::string& username) const;
  void setSuppressMailFromYourUpdatesNotes (bool value) const;
  std::vector <std::string> getActiveResources () const;
  void setActiveResources (std::vector <std::string> values) const;
  std::vector <std::string> getConsistencyResources () const;
  void setConsistencyResources (std::vector <std::string> values) const;
  int getSprintMonth () const;
  void setSprintMonth (int value) const;
  int getSprintYear () const;
  void setSprintYear (int value) const;
  bool getSprintProgressNotification () const;
  bool getUserSprintProgressNotification (const std::string& username) const;
  void setSprintProgressNotification (bool value) const;
  bool getUserChangesNotificationsOnline () const;
  bool getUserUserChangesNotificationsOnline (const std::string& username) const;
  void setUserChangesNotificationsOnline (bool value) const;
  bool getContributorChangesNotificationsOnline () const;
  bool getContributorChangesNotificationsOnline (const std::string& username) const;
  void setContributorChangesNotificationsOnline (bool value) const;
  std::string getWorkspaceURLs () const;
  void setWorkspaceURLs (const std::string& value) const;
  std::string getWorkspaceWidths () const;
  void setWorkspaceWidths (const std::string& value) const;
  std::string getWorkspaceHeights () const;
  void setWorkspaceHeights (const std::string& value) const;
  std::string getEntireWorkspaceWidths () const;
  void setEntireWorkspaceWidths (const std::string& value) const;
  std::string getActiveWorkspace () const;
  void setActiveWorkspace (const std::string& value) const;
  bool getPostponeNewNotesMails () const;
  void setPostponeNewNotesMails (bool value) const;
  std::string getRecentlyAppliedStyles () const;
  void setRecentlyAppliedStyles (const std::string& values) const;
  std::vector <std::string> getPrintResources () const;
  std::vector <std::string> getPrintResourcesForUser (const std::string& user) const;
  void setPrintResources (std::vector <std::string> values) const;
  Passage getPrintPassageFrom () const;
  Passage getPrintPassageFromForUser (const std::string& user) const;
  void setPrintPassageFrom (Passage value) const;
  Passage getPrintPassageTo () const;
  Passage getPrintPassageToForUser (const std::string& user) const;
  void setPrintPassageTo (const Passage& value) const;
  int getFocusedBook () const;
  void setFocusedBook (int book) const;
  int getFocusedChapter () const;
  void setFocusedChapter (int chapter) const;
  int getFocusedVerse () const;
  void setFocusedVerse (int verse) const;
  std::vector <int> getUpdatedSettings () const;
  void setUpdatedSettings (const std::vector <int>& values) const;
  void addUpdatedSetting (int value) const;
  void removeUpdatedSetting (int value) const;
  std::vector <int> getRemovedChanges () const;
  void setRemovedChanges (const std::vector <int>& values) const;
  void addRemovedChange (int value) const;
  void removeRemovedChange (int value) const;
  std::string getChangeNotificationsChecksum () const;
  void setChangeNotificationsChecksum (const std::string& value) const;
  void setUserChangeNotificationsChecksum (const std::string& user, const std::string& value) const;
  void setLiveBibleEditor (int time) const;
  int getLiveBibleEditor () const;
  void setResourceVersesBefore (int verses) const;
  int getResourceVersesBefore () const;
  void setResourceVersesAfter (int verses) const;
  int getResourceVersesAfter () const;
  std::string get_sync_key () const;
  void set_sync_key (const std::string& key) const;
//  std::string get_site_language () const;
//  void set_site_language (const std::string& value) const;
  void setGeneralFontSize (int size) const;
  int getGeneralFontSize () const;
  void setMenuFontSize (int size) const;
  int getMenuFontSize () const;
  void setResourcesFontSize (int size) const;
  int getBibleEditorsFontSize () const;
  void setBibleEditorsFontSize (int size) const;
  int getResourcesFontSize () const;
  void setHebrewFontSize (int size) const;
  int getHebrewFontSize () const;
  void setGreekFontSize (int size) const;
  int getGreekFontSize () const;
  void setVerticalCaretPosition (int position) const;
  int getVerticalCaretPosition () const;
  void setCurrentTheme (int index) const;
  int getCurrentTheme () const;
  bool getDisplayBreadcrumbs () const;
  void setDisplayBreadcrumbs (bool value) const;
  void setWorkspaceMenuFadeoutDelay (int value) const;
  int getWorkspaceMenuFadeoutDelay () const;
  int getEditingAllowedDifferenceChapter () const;
  void setEditingAllowedDifferenceChapter (int value) const;
  int getEditingAllowedDifferenceVerse () const;
  void setEditingAllowedDifferenceVerse (int value) const;
  bool getBasicInterfaceModeDefault () const;
  bool getBasicInterfaceMode () const;
  void setBasicInterfaceMode (bool value) const;
  bool getMainMenuAlwaysVisible () const;
  void setMainMenuAlwaysVisible (bool value) const;
  bool getSwipeActionsAvailable () const;
  void setSwipeActionsAvailable (bool value) const;
  bool getFastEditorSwitchingAvailable () const;
  void setFastEditorSwitchingAvailable (bool value) const;
  bool getIncludeRelatedPassages () const;
  void setIncludeRelatedPassages (bool value) const;
  int getFastSwitchVisualEditors () const;
  void setFastSwitchVisualEditors (int value) const;
  int getFastSwitchUsfmEditors () const;
  void setFastSwitchUsfmEditors (int value) const;
  bool getEnableStylesButtonVisualEditors () const;
  void setEnableStylesButtonVisualEditors (bool value) const;
  bool getMenuChangesInBasicMode () const;
  void setMenuChangesInBasicMode (bool value) const;
  bool getPrivilegeUseAdvancedMode () const;
  bool getPrivilegeUseAdvancedModeForUser (const std::string& username) const;
  void setPrivilegeUseAdvancedModeForUser (const std::string& username, bool value) const;
  bool getPrivilegeDeleteConsultationNotes () const;
  void setPrivilegeDeleteConsultationNotes (bool value) const;
  bool getPrivilegeDeleteConsultationNotesForUser (const std::string& username) const;
  void setPrivilegeDeleteConsultationNotesForUser (const std::string& username, bool value) const;
  bool getPrivilegeSetStylesheets () const;
  bool getPrivilegeSetStylesheetsForUser (const std::string& username) const;
  void setPrivilegeSetStylesheetsForUser (const std::string& username, bool value) const;
  bool getDismissChangesAtTop () const;
  void setDismissChangesAtTop (bool value) const;
  bool getQuickNoteEditLink () const;
  void setQuickNoteEditLink (bool value) const;
  bool getShowBibleInNotesList () const;
  void setShowBibleInNotesList (bool value) const;
  bool getShowNoteStatus () const;
  void setShowNoteStatus (bool value) const;
  bool getShowVerseTextAtCreateNote () const;
  void setShowVerseTextAtCreateNote (bool value) const;
  bool getOrderChangesByAuthor () const;
  void setOrderChangesByAuthor (bool value) const;
  std::vector <std::string> getAutomaticNoteAssignment () const;
  void setAutomaticNoteAssignment (const std::vector <std::string>& values) const;
  bool getReceiveFocusedReferenceFromParatext () const;
  void setReceiveFocusedReferenceFromParatext (bool value) const;
  bool getReceiveFocusedReferenceFromAccordance () const;
  void setReceiveFocusedReferenceFromAccordance (bool value) const;
  bool getUseColoredNoteStatusLabels () const;
  void setUseColoredNoteStatusLabels (bool value) const;
  int getNotesDateFormat () const;
  void setNotesDateFormat (int value) const;
  std::vector <std::string> getChangeNotificationsBibles () const;
  std::vector <std::string> getChangeNotificationsBiblesForUser (const std::string & user) const;
  void setChangeNotificationsBibles (const std::vector <std::string>& values) const;
  bool get_enable_spell_check () const;
  void set_enable_spell_check (bool value) const;
private:
  Webserver_Request& m_webserver_request;
  std::string file (const std::string& user) const;
  std::string file (const std::string& user, const char * key) const;
  std::string mapkey (const std::string& user, const char * key) const;
  std::string getValue (const char * key, const char * default_value) const;
  bool getBValue (const char * key, bool default_value) const;
  int getIValue (const char * key, int default_value) const;
  std::string getValueForUser (const std::string& user, const char * key, const char * default_value) const;
  bool getBValueForUser (const std::string& user, const char * key, bool default_value) const;
  int getIValueForUser (const std::string& user, const char * key, int default_value) const;
  void setValue (const char * key, const std::string& value) const;
  void setBValue (const char * key, bool value) const;
  void setIValue (const char * key, int value) const;
  void setValueForUser (const std::string& user, const char * key, const std::string& value) const;
  void setBValueForUser (const std::string& user, const char * key, bool value) const;
  std::vector <std::string> getList (const char * key) const;
  std::vector <std::string> getListForUser (const std::string& user, const char * key) const;
  void setList (const char * key, const std::vector <std::string>& values) const;
  void setListForUser (const std::string& user, const char * key, const std::vector <std::string>& values) const;
  std::vector <int> getIList (const char * key) const;
  void setIList (const char * key, const std::vector <int>& values) const;
  static const char * sprint_month_key ();
  static const char * sprint_year_key ();
  bool default_bible_checks_notification () const;
};
