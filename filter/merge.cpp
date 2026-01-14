/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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


#include <filter/merge.h>
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include <dtl/dtl.hpp>
#pragma GCC diagnostic pop
#include <filter/string.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
#include <email/send.h>
#include <filter/usfm.h>


static std::mutex filter_merge_mutex;


// This uses the dtl:: library for merge in C++.
// At times the library failed to merge.
// It was tried whether the Linux "merge" command was able to successfully merge such cases.
// But also this command failed to merge in such cases.
// The conclusion therefore is that the C++ merge library is equivalent in quality.


// merge - three-way merge.
// Merge is useful for combining separate changes to an original.
// The function normally returns the merged text.
// If case of conflicts, it returns an empty container.
std::vector <std::string> filter_merge_merge (const std::vector <std::string>& base, const std::vector <std::string>& user, const std::vector <std::string>& server)
{
  // See issue https://github.com/bibledit/cloud/issues/418
  // It is unclear at this time whether the code below
  // to find the differences between texts, is thread-safe.
  // So just to be sure, a mutex is placed around it.
  filter_merge_mutex.lock();

  std::vector <std::string> user_sequence (user);
  std::vector <std::string> base_sequence (base);
  std::vector <std::string> server_sequence (server);

  dtl::Diff3 <std::string, std::vector <std::string>> diff3 (user_sequence, base_sequence, server_sequence);
  diff3.compose ();
  bool merged = diff3.merge ();
  filter_merge_mutex.unlock();
  if (!merged) {
    return {};
  }
  return diff3.getMergedSequence ();
}


std::string filter_merge_lines2words (std::string data)
{
  data = filter::string::replace ("\n", " new__line ", data);
  data = filter::string::replace (" ", "\n", data);
  return data;
}


std::string filter_merge_words2lines (std::string data)
{
  data = filter::string::replace ("\n", " ", data);
  data = filter::string::replace (" new__line ", "\n", data);
  return data;
}


std::string filter_merge_lines2graphemes (std::string data)
{
  data = filter::string::replace ("\n", " new__line ", data);
  std::string data2;
  size_t count = filter::string::unicode_string_length (data);
  for (size_t i = 0; i < count; i++) {
    std::string grapheme = filter::string::unicode_string_substr (data, i, 1);
    data2.append (grapheme);
    data2.append ("\n");
  }
  return data2;
}


std::string filter_merge_graphemes2lines (std::string data)
{
  data = filter::string::replace ("\n", "", data);
  data = filter::string::replace (" new__line ", "\n", data);
  return data;
}


void filter_merge_detect_conflict (std::string base,
                                   std::string change,
                                   std::string prioritized_change,
                                   std::string result,
                                   std::vector <Merge_Conflict> & conflicts)
{
  // Clean input.
  base = filter::string::trim (base);
  change = filter::string::trim (change);
  prioritized_change = filter::string::trim (prioritized_change);
  result = filter::string::trim (result);

  bool irregularity = false;
  std::string subject;
  
  if (!irregularity) {
    if (base.empty ()) {
      subject = "There was no text to base the merge upon";
      irregularity = true;
    }
  }
  
  if (!irregularity) {
    if (change.empty ()) {
      subject = "There was no changed text to merge with";
      irregularity = true;
    }
  }
  
  if (!irregularity) {
    if (prioritized_change.empty ()) {
      subject = "There was no existing text to merge with";
      irregularity = true;
    }
  }
  
  if (!irregularity) {
    if (result.empty ()) {
      subject = "The merge resulted in empty text";
      irregularity = true;
    }
  }
  
  if (!irregularity) {
    if ((change != base) && (prioritized_change != change) && (prioritized_change == result)) {
      subject = "Failed to merge your changes";
      irregularity = true;
    }
  }

  if (irregularity) {
    Merge_Conflict conflict;
    conflict.base = base;
    conflict.change = change;
    conflict.prioritized_change = prioritized_change;
    conflict.result = result;
    conflict.subject = subject;
    conflicts.push_back (conflict);
  }
}


// This filter merges files.
// $base: Data for the merge base.
// $change: Data as modified by one user.
// $prioritized_change: Data as modified by a user but prioritized.
// The filter uses a three-way merge algorithm.
// There should be one unchanged segment (either a line or word) between the modifications.
// If necessary it converts the data into a new format with one character per line for more fine-grained merging.
// In case of a conflict, it prioritizes changes from $prioritized_change.
// The filter returns the merged data.
// If $clever, it calls a more clever routine when it fails to merge.
std::string filter_merge_run (std::string base, std::string change, std::string prioritized_change,
                              bool clever,
                              std::vector <Merge_Conflict> & conflicts)
{
  // Trim the input.
  base = filter::string::trim (base);
  change = filter::string::trim (change);
  prioritized_change = filter::string::trim (prioritized_change);

  // Try a standard line-based merge. Should be sufficient for most cases.
  std::vector <std::string> baselines = filter::string::explode (base, '\n');
  std::vector <std::string> userlines = filter::string::explode (change, '\n');
  std::vector <std::string> serverlines = filter::string::explode (prioritized_change, '\n');
  std::vector <std::string> results = filter_merge_merge (baselines, userlines, serverlines);
  if (!results.empty ()) {
    std::string result = filter::string::implode (results, "\n");
    filter_merge_detect_conflict (base, change, prioritized_change, result, conflicts);
    return result;
  }

  // Convert the data to one word per line, and try to merge again.
  std::string baseWords = filter_merge_lines2words (base);
  std::string userWords = filter_merge_lines2words (change);
  std::string serverWords = filter_merge_lines2words (prioritized_change);
  baselines = filter::string::explode (baseWords, '\n');
  userlines = filter::string::explode (userWords, '\n');
  serverlines = filter::string::explode (serverWords, '\n');
  results = filter_merge_merge (baselines, userlines, serverlines);
  if (!results.empty ()) {
    std::string mergedWords = filter::string::implode (results, "\n");
    std::string result = filter_merge_words2lines (mergedWords);
    filter_merge_detect_conflict (base, change, prioritized_change, result, conflicts);
    return result;
  }

  // Convert the data so it has one grapheme per line, and try again.
  std::string baseGraphemes = filter_merge_lines2graphemes (base);
  std::string userGraphemes = filter_merge_lines2graphemes (change);
  std::string serverGraphemes = filter_merge_lines2graphemes (prioritized_change);
  baselines = filter::string::explode (baseGraphemes, '\n');
  userlines = filter::string::explode (userGraphemes, '\n');
  serverlines = filter::string::explode (serverGraphemes, '\n');
  results = filter_merge_merge (baselines, userlines, serverlines);
  if (!results.empty ()) {
    std::string mergedGraphemes = filter::string::implode (results, "\n");
    std::string result = filter_merge_graphemes2lines (mergedGraphemes);
    filter_merge_detect_conflict (base, change, prioritized_change, result, conflicts);
    return result;
  }

  if (clever) {
    // It failed to merge: Call a more clever routine to do the merge.
    std::string result = filter_merge_run_clever (base, change, prioritized_change, conflicts);
    // Check on merge failure.
    filter_merge_detect_conflict (base, change, prioritized_change, result, conflicts);
    // Done.
    return result;
  }
  
  // The data could not be merged no matter how hard it tried.
  // Detect it as a conflict.
  filter_merge_detect_conflict (base, change, prioritized_change, prioritized_change, conflicts);
  // Done;
  return prioritized_change;
}


// This filter merges USFM data in a clever way.
// $base: Data for the merge base.
// $change: Data as modified by one user.
// $prioritized_change: Data as modified by a user but prioritized.
// The filter uses a three-way merge algorithm.
std::string filter_merge_run_clever (std::string base, std::string change, std::string prioritized_change,
                                     std::vector <Merge_Conflict> & conflicts)
{
  // Get the verse numbers in the changed text.
  std::vector <int> verses = filter::usfm::get_verse_numbers (change);
  
  std::vector <std::string> results;
  
  std::string previous_change;

  // Go through the verses.
  for (auto verse : verses) {
    
    // Gets the texts to merge for this verse.
    std::string base_text = filter::usfm::get_verse_text (base, verse);
    std::string change_text = filter::usfm::get_verse_text (change, verse);
    std::string prioritized_change_text = filter::usfm::get_verse_text (prioritized_change, verse);
    
    // Check for combined verses.
    if (change_text == previous_change) continue;
    previous_change = change_text;
    
    // Check whether any of the three text fragments can be considered to be a verse without content.
    size_t empty_length = 3 + std::to_string(verse).length() + 1; // "\v n "
    bool base_empty = base_text.length () <= empty_length;
    bool change_empty = change_text.length () <= empty_length;
    bool prioritized_change_empty = prioritized_change_text.length () <= empty_length;

    // If the prioritized change is empty, and the other two are not,
    // update the priotitized change to match the change.
    // Without doing this the merge behaves in an unexpected way:
    // It would take the prioritized change instead of the the base/change.
    if (prioritized_change_empty) {
      if (!base_empty && !change_empty) {
        prioritized_change_text = change_text;
      }
    }
    
    // Run the merge, but clear the "clever" flags else it may enter an infinite loop.
    std::string result = filter_merge_run (base_text, change_text, prioritized_change_text, false, conflicts);

    // Store it.
    results.push_back (result);
  }
  
  // Done.
  return filter::string::implode (results, "\n");
}


void filter_merge_add_book_chapter (std::vector <Merge_Conflict> & conflicts, int book, int chapter)
{
  for (auto & conflict : conflicts) {
    conflict.book = book;
    conflict.chapter = chapter;
  }
}
