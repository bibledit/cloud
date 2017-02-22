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


#ifndef INCLUDED_DATABASE_NOTES_H
#define INCLUDED_DATABASE_NOTES_H


#include <config/libraries.h>
#include <filter/passage.h>


class Database_Notes_Text
{
public:
  string raw;
  string localized;
};


class Database_Notes
{
public:
  Database_Notes (void * webserver_request_in);
  void create ();
  string database_path ();
  string checksums_database_path ();
  bool healthy ();
  bool checksums_healthy ();
  bool checkup ();
  bool checkup_checksums ();
  void trim ();
  void trim_server ();
  void optimize ();
  void sync ();
  void updateDatabase (int identifier);
  string mainFolder ();
  string noteFolder (int identifier);
  string bibleFile (int identifier);
  string passageFile (int identifier);
  string statusFile (int identifier);
  string severityFile (int identifier);
  string modifiedFile (int identifier);
  string summaryFile (int identifier);
  string contentsFile (int identifier);
  string subscriptionsFile (int identifier);
  string assignedFile (int identifier);
  string publicFile (int identifier);
  bool identifierExists (int identifier);
  void setIdentifier (int identifier, int new_identifier);
  int getNewUniqueIdentifier ();
  vector <int> getIdentifiers ();
  int storeNewNote (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw);
  vector <int> selectNotes (vector <string> bibles, int book, int chapter, int verse, int passage_selector, int edit_selector, int non_edit_selector, const string& status_selector, string bible_selector, string assignment_selector, bool subscription_selector, int severity_selector, int text_selector, const string& search_text, int limit);
  string getSummary (int identifier);
  void setSummary (int identifier, const string& summary);
  string getContents (int identifier);
  void setContents (int identifier, const string& contents);
  void erase (int identifier);
  void addComment (int identifier, const string& comment);
  void subscribe (int identifier);
  void subscribeUser (int identifier, const string& user);
  vector <string> getSubscribers (int identifier);
  void setSubscribers (int identifier, vector <string> subscribers);
  bool isSubscribed (int identifier, const string& user);
  void unsubscribe (int identifier);
  void unsubscribeUser (int identifier, const string& user);
  vector <string> getAllAssignees (const vector <string>& bibles);
  vector <string> getAssignees (int identifier);
  void setAssignees (int identifier, vector <string> assignees);
  void assignUser (int identifier, const string& user);
  bool isAssigned (int identifier, const string& user);
  void unassignUser (int identifier, const string& user);
  string getBible (int identifier);
  void setBible (int identifier, const string& bible);
  vector <string> getAllBibles ();
  string encodePassage (int book, int chapter, int verse);
  Passage decodePassage (string passage);
  string getRawPassage (int identifier);
  vector <Passage> getPassages (int identifier);
  void setPassages (int identifier, const vector <Passage>& passages, bool import = false);
  void setRawPassage (int identifier, const string& passage);
  string getRawStatus (int identifier);
  string getStatus (int identifier);
  void setStatus (int identifier, const string& status, bool import = false);
  vector <Database_Notes_Text> getPossibleStatuses ();
  int getRawSeverity (int identifier);
  string getSeverity (int identifier);
  void setRawSeverity (int identifier, int severity);
  vector <Database_Notes_Text> getPossibleSeverities ();
  int getModified (int identifier);
  void setModified (int identifier, int time);
  bool getPublic (int identifier);
  void setPublic (int identifier, bool value);
  vector <int> selectDuplicateNotes (const string& rawpassage, const string& summary, const string& contents);
  void updateSearchFields (int identifier);
  string getSearchField (int identifier);
  vector <int> searchNotes (string search, const vector <string> & bibles);
  void markForDeletion (int identifier);
  void unmarkForDeletion (int identifier);
  bool isMarkedForDeletion (int identifier);
  void touchMarkedForDeletion ();
  vector <int> getDueForDeletion ();
  void setChecksum (int identifier, const string & checksum);
  string getChecksum (int identifier);
  void deleteChecksum (int identifier);
  void updateChecksum (int identifier);
  string getMultipleChecksum (const vector <int> & identifiers);
  vector <int> getNotesInRangeForBibles (int lowId, int highId, const vector <string> & bibles, bool anybible);
  void set_availability (bool available);
  bool available ();
  string getBulk (vector <int> identifiers);
  vector <string> setBulk (string json);
private:
  void * webserver_request;
  sqlite3 * connect ();
  sqlite3 * connect_checksums ();
  string expiryFile (int identifier);
  string assembleContents (int identifier, string contents);
  vector <string> getAssigneesInternal (string assignees);
  vector <string> standard_severities ();
  void noteEditedActions (int identifier);
  string getBibleSelector (vector <string> bibles);
  string availability_flag ();
  string notesSelectIdentifier ();
  string notesOptionalFulltextSearchRelevanceStatement (string search);
  string notesFromWhereStatement ();
  string notesOptionalFulltextSearchStatement (string search);
  string notesOrderByRelevanceStatement ();
};


#endif
