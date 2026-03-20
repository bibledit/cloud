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


#include <filter/mail.h>
#include <filter/string.h>


#ifdef HAVE_CLOUD


// https://www.codesink.org/mimetic_mime_library.html


// Suppress warnings in the included header.
#pragma GCC diagnostic push
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wdeprecated-register"
#pragma clang diagnostic ignored "-Wunused-private-field"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wconditional-uninitialized"
#pragma clang diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <mimetic098/mimetic.h>
#pragma clang diagnostic pop
#pragma GCC diagnostic pop


std::string filter_mail_remove_headers_internal (std::string contents)
{
  bool empty_line_encountered = false;
  std::vector <std::string> cleaned;
  std::vector <std::string> inputlines = filter::string::explode (contents, '\n');
  for (auto line : inputlines) {
    if (line.find ("Content-Type") != std::string::npos) continue;
    if (line.find ("Content-Transfer-Encoding") != std::string::npos) continue;
    if (empty_line_encountered) cleaned.push_back (line);
    if (filter::string::trim (line).empty ()) empty_line_encountered = true;
  }
  contents = filter::string::implode (cleaned, "\n");
  contents = filter::string::trim (contents);
  return contents;
}


void filter_mail_dissect_internal (const mimetic::MimeEntity& me, std::string& plaintext)
{
  // If the plain text of this email has been found already,
  // there's no need to search any further.
  if (!plaintext.empty ()) return;
  
  // Get the header of this part.
  const mimetic::Header& h = me.header();
  
  // Look for content type and subtype.
  // Fold their case as some messages use upper case.
  const std::string type = filter::string::unicode_string_casefold (h.contentType().type());
  const std::string subtype = filter::string::unicode_string_casefold (h.contentType().subtype());

  if (type == "text") {
  
    if (subtype== "plain") {
      // Get the plain text of the message.
      std::stringstream ss;
      ss << me;
      plaintext = ss.str ();
      // Remove headers.
      plaintext = filter_mail_remove_headers_internal (plaintext);
    }
    
    if (subtype== "html") {
      // Get the html text of the message.
      std::stringstream ss;
      ss << me;
      std::string html = ss.str ();
      // Remove headers.
      html = filter_mail_remove_headers_internal (html);
      // Convert the html to plain text.
      plaintext = filter::string::html2text (html);
    }
    
    // Get transfer encoding.
    // Fold the case as some email messages use uppercase.
    std::string transfer_encoding = filter::string::unicode_string_casefold (h.contentTransferEncoding().str ());
    
    // Decode quoted-printable text.
    if (transfer_encoding == mimetic::ContentTransferEncoding::quoted_printable) {
      std::istringstream is (plaintext);
      std::ostringstream os;
      std::istreambuf_iterator<char> ibeg (is), iend;
      std::ostreambuf_iterator<char> out (os);
      mimetic::QP::Decoder qp;
      decode (ibeg, iend, qp, out);
      plaintext = os.str ();
    }
    
    // Decode base64 text.
    if (transfer_encoding == mimetic::ContentTransferEncoding::base64) {
      std::istringstream is (plaintext);
      std::ostringstream os;
      std::istreambuf_iterator<char> ibeg (is), iend;
      std::ostreambuf_iterator<char> out (os);
      mimetic::Base64::Decoder b64;
      code (ibeg, iend, b64, out);
      plaintext = os.str ();
    }
  }

  // Iterate over the other parts it may contain and process them.
  mimetic::MimeEntityList::const_iterator mime_body_iterator = me.body().parts().begin();
  mimetic::MimeEntityList::const_iterator meit = me.body().parts().end();
  for (; mime_body_iterator != meit; ++mime_body_iterator) {
    filter_mail_dissect_internal (**mime_body_iterator, plaintext);
  }
}


// Dissects an email $message.
// It extracts the $from address, the $subject, and the plain text body.
void filter_mail_dissect (std::string message, std::string & from, std::string & subject, std::string & plaintext)
{
  // Load the email message into the mimetic library.
  mimetic::MimeEntity me;
  me.load (message.begin(), message.end(), 0);

  // Get the sender's address.
  std::stringstream fromstream;
  fromstream << me.header().from();
  from = fromstream.str ();

  // Get the subject.
  std::stringstream subjectstream;
  subjectstream << me.header().subject();
  subject = subjectstream.str ();

  // Get the plain text body.
  filter_mail_dissect_internal (me, plaintext);
  
  // In case it's not a MIME message, the plain/text part will not be there.
  // Take the email's entire body instead.
  if (plaintext.empty ()) plaintext = me.body ();

  // Clean the text body up.
  std::vector <std::string> cleaned;
  std::vector <std::string> inputlines = filter::string::explode (plaintext, '\n');
  for (auto line : inputlines) {
    // Remove whitespace and empty lines.
    line = filter::string::trim (line);
    if (line.empty ()) continue;
    // If the line starts with ">", it indicates quoted text. Skip it.
    if (line.substr (0, 1) == ">") continue;
    // Store this line.
    cleaned.push_back (line);
  }
  plaintext = filter::string::implode (cleaned, "\n");
}


std::string filter_mail_address_name (std::string name)
{
  // Allowed characters in the To: and From: headers.
  // Letters: a-z, A-Z.
  // Digits: 0-9.
  // Symbols: ! # $ % & ' * + - / = ? ^ _ { | } ~`.
  // Dot (.): Allowed, but cannot be the first or last character,
  //          and cannot appear consecutively (e.g., user..name@example.com is invalid).
  // Quoted Strings: If the local part is enclosed in quotes, spaces and other characters are permitted, though rarely used.
  // To make the filter easy, allow only letters, digits, underscores and spaces.
  // See https://www.rfc-editor.org/rfc/rfc5322.txt
  // more specifically https://datatracker.ietf.org/doc/html/rfc5322#section-3.4
  const auto allowed = [](const char c) {
    // Put the checks in an order likely optimized for speed.
    if (c >= 'a' and c <= 'z')
      return true;
    if (c >= 'A' and c <= 'Z')
      return true;
    if (c == ' ')
      return true;
    if (c >= '0' and c <= '9')
      return true;
    return false;
  };
  std::string output;
  std::ranges::copy_if(name, std::back_inserter(output), allowed);
  return output;
}


#endif


// Limit the length of one line according to RFC5322 section 2.1.1.
// https://www.rfc-editor.org/rfc/rfc5322#section-2.1.1
// The function does this while focusing on performance.
std::string filter_mail_limit_line_length_rfc5322(std::string body, const int length)
{
  // If the HTML body length is within the maximum line length: Ready.
  if (body.length() <= length)
    return body;

  // Maximum number of iterations allowed: This prevents an infinite loop.
  int iterations {1000};

  // The caret: The location in the HTML body to work from.
  size_t caret {0};

  while (iterations--) {

    // Increase the caret with one character: Reasons:
    // 1. Never search from the same location.
    // 2. Always move forward.
    caret++;

    // If the fragment length after the caret is within the maximum length: Ready.
    if (static_cast<int>(body.length() - caret) < length) {
      break;
    }

    // Check whether the next new line after the caret is within the maximum line length.
    // If so update the caret position and go to next iteration.
    if (const auto nl_pos = body.find('\n', caret);
        (nl_pos - caret) <= length) {
      caret = nl_pos;
      continue;
    }

    // Search for the last ">"
    // in the range of the caret to the caret plus max line length.
    // Add a new line after that.
    // Update the caret positon and go to the next iteration.
    if (auto gt_pos = body.rfind('>', caret + length);
        gt_pos != std::string::npos and gt_pos > caret)
    {
      body.insert(gt_pos + 1, "\n");
      caret = gt_pos + 1;
      continue;
    }

    // The last ">" was not found.
    // Now look at the last "<"
    // in the range of the caret to the caret plus max line length.
    // Add a new line before that.
    // Update the caret position and go to the next iteration.
    if (auto st_pos = body.rfind('<', caret + length);
        st_pos != std::string::npos and st_pos > caret)
    {
      body.insert(st_pos, "\n");
      caret = st_pos + 1;
      continue;
    }

    // Okay, the ">" or the "<" were not found: If needed:
    // 1. Add a new line in the body at the range end.
    // 2. Update the caret position.
    if ((body.length() - caret) > length) {
      const size_t range_end = std::min(caret + length, body.length());
      body.insert(range_end, "\n");
      caret = range_end;
    }
  }

  // Return the possibly updated body.
  return body;
}
