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


#include <filter/mail.h>
#include <filter/string.h>


#ifdef HAVE_CLOUD


// http://www.codesink.org/mimetic_mime_library.html


// Suppress warnings in the included header.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wdeprecated-register"
#pragma clang diagnostic ignored "-Wunused-private-field"
#include <mimetic/mimetic.h>
#pragma clang diagnostic pop


using namespace mimetic;


void filter_mail_dissect_internal (const MimeEntity& me, string& plaintext)
{
  // Get the header of this part.
  const Header& h = me.header();
  // Look for content type text/plain.
  if (h.contentType().type() == "text") {
    if (h.contentType().subtype() == "plain") {
      if (plaintext.empty ()) {
        // Get the plain text of the message.
        stringstream ss;
        ss << me;
        plaintext = ss.str ();
        // Remove headers.
        vector <string> cleaned;
        vector <string> inputlines = filter_string_explode (plaintext, '\n');
        for (auto line : inputlines) {
          if (line.find ("Content-Type") != string::npos) continue;
          if (line.find ("Content-Transfer-Encoding") != string::npos) continue;
          cleaned.push_back (line);
        }
        plaintext = filter_string_implode (cleaned, "\n");
        // Decode quoted-printable text.
        if (h.contentTransferEncoding().str () == ContentTransferEncoding::quoted_printable) {
          istringstream is (plaintext);
          ostringstream os;
          istreambuf_iterator<char> ibeg (is), iend;
          ostreambuf_iterator<char> out (os);
          QP::Decoder qp;
          decode (ibeg, iend, qp, out);
          plaintext = os.str ();
        }
        // Decode base64 text.
        if (h.contentTransferEncoding().str () == ContentTransferEncoding::base64) {
          istringstream is (plaintext);
          ostringstream os;
          istreambuf_iterator<char> ibeg (is), iend;
          ostreambuf_iterator<char> out (os);
          Base64::Decoder b64;
          code (ibeg, iend, b64, out);
          plaintext = os.str ();
        }
      }
    }
  }
  // Iterate over the other parts and process them.
  MimeEntityList::const_iterator mime_body_iterator = me.body().parts().begin();
  MimeEntityList::const_iterator meit = me.body().parts().end();
  for (; mime_body_iterator != meit; ++mime_body_iterator) {
    filter_mail_dissect_internal (**mime_body_iterator, plaintext);
  }
}


// Dissects an email $message.
// It extracts the $from address, the $subject, and the plain text body.
void filter_mail_dissect (string message, string & from, string & subject, string & plaintext)
{
  // Load the email message into the mimetic library.
  MimeEntity me;
  me.load (message.begin(), message.end(), 0);

  // Get the sender's address.
  stringstream fromstream;
  fromstream << me.header().from();
  from = fromstream.str ();

  // Get the subject.
  stringstream subjectstream;
  subjectstream << me.header().subject();
  subject = subjectstream.str ();

  // Get the plain text body.
  filter_mail_dissect_internal (me, plaintext);
  
  // In case it's not a MIME message, the plain/text part will not be there.
  // Take the email's entire body instead.
  if (plaintext.empty ()) plaintext = me.body ();

  // Clean the text body up.
  vector <string> cleaned;
  vector <string> inputlines = filter_string_explode (plaintext, '\n');
  for (auto line : inputlines) {
    // Remove whitespace and empty lines.
    line = filter_string_trim (line);
    if (line.empty ()) continue;
    // If the line starts with ">", it indicates quoted text. Skip it.
    if (line.substr (0, 1) == ">") continue;
    // Store this line.
    cleaned.push_back (line);
  }
  plaintext = filter_string_implode (cleaned, "\n");
}


#endif
