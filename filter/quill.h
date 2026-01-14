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


#pragma once

#include <config/libraries.h>

namespace quill {

// The prefix configured in a Quill-based editor for block text.
constexpr const char* class_prefix_block {"b-"};

// The prefix configured in a Quill-based editor for inline text.
constexpr const char* class_prefix_inline {"i-"};

// The prefix used in a Quill-based editor for referencing word-level attributes stored elsewhere.
constexpr const std::string_view word_level_attribute_class_prefix {"wla"};

// The prefix used in a Quill-based editor for referencing milestone attributes stored elsewhere.
constexpr const std::string_view milestone_attribute_class_prefix {"mls"};
 
// The class of the notes container in a Quill-based editor.
constexpr const char* notes_class {"notes"};

// The class that Quill uses to mark the caret.
constexpr const char* caret_class {"ql-cursor"};

// The class in Quill-based editor for the note caller.
constexpr const std::string_view note_caller_class {"notecall"};

// The class of the word-level attributes container in a Quill-based editor.
constexpr const char* word_level_attributes_class {"wordlevelattributes"};

// The emoji for a milestone to show in the editor.
constexpr const char* milestone_emoji {"🏁"};

// The class of the milestone attributes container in a Quill-based editor.
constexpr const char* milestone_attributes_class {"milestoneattributes"};

// Converters for milestone styles to/from Quill styles.
std::string hyphen_to_underscore (std::string style);
std::string underscore_to_hyphen (std::string style);

} // Namespace.
