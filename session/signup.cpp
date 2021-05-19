/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <session/login.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/indonesian.h>
#include <confirm/worker.h>
#include <email/send.h>
#include <pugixml/pugixml.hpp>
#include <tasks/logic.h>


using namespace pugi;


class Verification
{
public:
  string question;
  string answer;
  string passage;
};


const char * session_signup_url ()
{
  return "session/signup";
}


bool session_signup_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string session_signup (void * webserver_request)
{
  (void) webserver_request;
  
  string page;

#ifdef HAVE_CLOUD

  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  Assets_Header header = Assets_Header (translate ("Signup"), webserver_request);
  header.touchCSSOn ();
  page += header.run ();
  
  Assets_View view;

  // Some security questions.
  vector <Verification> verifications;
  Verification verification;

  verification.question = translate("To which city was Paul travelling when a light from heaven shone round about him?");
  verification.answer   = translate("Damascus");
  verification.passage  = translate("And while he travelled, he came near Damascus; and suddenly a light from heaven shone round about him.");
  verifications.push_back (verification);
 
  verification.question = translate("What is the name of the brother of Aaron the high priest?");
  verification.answer   = translate("Moses");
  verification.passage  = translate("And the anger of Jehova was kindled against Moses, and he said: Is not Aaron the Levite your brother?");
  verifications.push_back (verification);
  
  verification.question = translate("What is the name of the city where Jesus was born?");
  verification.answer   = translate("Bethlehem");
  verification.passage  = translate("When Jesus was born in Bethlehem of Judaea in the days of Herod the king, behold, wise men from the east came to Jerusalem.");
  verifications.push_back (verification);
  
  verification.question = translate("What is the name of the island where John was sent to?");
  verification.answer   = translate("Patmos");
  verification.passage  = translate("I, John, your brother and companion in the persecution, and in the kingdom and endurance of Jesus Christ, was in the island called Patmos, because of the word of God, and because of the testimony of Jesus Christ.");
  verifications.push_back (verification);
  
  verification.question = translate("Who is the first person to acknowledge Jesus as the Christ?");
  verification.answer   = translate("Simeon");
  verification.passage  = translate("There was Simeon. A just and devout man. The Holy Spirit told him that he will see Christ before his death.");
  verifications.push_back (verification);

  verification.question = translate("To whom was the Acts Of Apostles written?");
  verification.answer   = translate("Theophilus");
  verification.passage  = translate("The former treatise have I made, O Theophilus, of all that Jesus began both to do and teach,");
  verifications.push_back (verification);

  verification.question = translate("Where did Jesus met the woman from Samaria?");
  verification.answer   = translate("Jacob's well");
  verification.passage  = translate("Jesus being wearied by his journey, sat on Jacob's well. Then came a woman from Samaria to get water.");
  verifications.push_back (verification);

  verification.question = translate("Who was the father of Abraham?");
  verification.answer   = translate("Terah");
  verification.passage  = translate("And Terah took Abram his son,");
  verifications.push_back (verification);

  verification.question = translate("Who was the son of Cush, the mighty hunter before the Lord?");
  verification.answer   = translate("Nimrod");
  verification.passage  = translate("And Cush bore Nimrod. He was a mighty hunter before the Lord.");
  verifications.push_back (verification);

  verification.question = translate("What is the name of Job's friend that answered him first?");
  verification.answer   = translate("Eliphaz");
  verification.passage  = translate("Then Eliphaz the Temanite answered and said to Job.");
  verifications.push_back (verification);

  verification.question = translate("What is the name of Ruth's husband?");
  verification.answer   = translate("Boaz");
  verification.passage  = translate("So Boaz took Ruth as his wife.");
  verifications.push_back (verification);

  verification.question = translate("Who taught Lemuel Proverbs chapter 31?");
  verification.answer   = translate("Lemuel's mother");
  verification.passage  = translate("The words of king Lemuel, the prophecy that his mother taught him.");
  verifications.push_back (verification);

  verification.question = translate("Who is the chosen people of God in the Old Testament?");
  verification.answer   = translate("Israel");
  verification.passage  = translate("Israel, you are the chosen people of the Lord your God.");
  verifications.push_back (verification);

  verification.question = translate("Where did God rescue Israel from?");
  verification.answer   = translate("Egypt");
  verification.passage  = translate("Then with his mighty arm, he rescued you from Egypt.");
  verifications.push_back (verification);

  verification.question = translate("Who's the father of Abimelech?");
  verification.answer   = translate("Gideon");
  verification.passage  = translate("Abimelech the son of Gideon went to Shechem.");
  verifications.push_back (verification);

  verification.question = translate("Who's the father of Joshua the priest in the time of Ezra?");
  verification.answer   = translate("Jozadak");
  verification.passage  = translate("The priest Joshua son of Jozadak.");
  verifications.push_back (verification);

  verification.question = translate("Who's Nebuchadnezzar?");
  verification.answer   = translate("The king of Babylonia");
  verification.passage  = translate("We were told that their people had made God angry, and he let them be captured by Nebuchadnezzar, the king of Babylonia.");
  verifications.push_back (verification);

  verification.question = translate("Who's Cyrus?");
  verification.answer   = translate("King of Babylonia");
  verification.passage  = translate("They also said that during the first year Cyrus was king of Babylonia.");
  verifications.push_back (verification);

  verification.question = translate("For how many days does Jesus appeared to the disciples after he rose from the dead?");
  verification.answer   = translate("40");
  verification.passage  = translate("For 40 days after Jesus had suffered and died, he proved in many ways that he had been raised from death.");
  verifications.push_back (verification);

  verification.question = translate("Who wrote Song of Songs?");
  verification.answer   = translate("Solomon");
  verification.passage  = translate("This is Solomon's most beautiful song.");
  verifications.push_back (verification);

  verification.question = translate("Who's the father of Hosea?");
  verification.answer   = translate("Beeri");
  verification.passage  = translate("I am Hosea son of Beeri.");
  verifications.push_back (verification);

  int question_number = filter_string_rand (0, 20);
  view.set_variable ("question", verifications[question_number].question);
  view.set_variable ("passage", verifications[question_number].passage);
  // The form has a hidden text entry. This text entry stores the right answer to the questions.
  // When the form is submitted, this right answer is submitted too, and we can check whether
  // the user gave the right answer.
  view.set_variable ("standard", verifications[question_number].answer);
  
  // Form submission handler.
  bool signed_up = false;
  if (request->post["submit"] != "") {
    bool form_is_valid = true;
    string user     = request->post["user"];
    string pass     = request->post["pass"];
    string mail     = request->post["mail"];
    string answer   = request->post["answer"];
    string standard = request->post["standard"];
    if (user.length () < 4) {
      form_is_valid = false;
      view.set_variable ("username_invalid_message", translate("Username should be at least four characters long"));
    }
    if (pass.length () < 4) {
      form_is_valid = false;
      view.set_variable ("password_invalid_message", translate("Password should be at least four characters long"));
    }
    if (!filter_url_email_is_valid (mail)) {
      form_is_valid = false;
      view.set_variable ("email_invalid_message", translate("The email address is not valid"));
    }
    if (answer != standard) {
      form_is_valid = false;
      view.set_variable ("answer_invalid_message", translate("The answer to the question is not correct"));
    }
    Database_Users database_users;
    if (form_is_valid) {
      if (database_users.usernameExists (user)) {
        string message = translate("The username that you have chosen has already been taken.") + " " + translate("Please choose another one.");
        view.set_variable ("error_message", message);
        form_is_valid = false;
      }
    }
    if (form_is_valid) {
      if (database_users.emailExists (mail)) {
        string message = translate("The email address that you have chosen has already been taken.") + " " + translate("Please choose another one.");
        view.set_variable ("error_message", message);
        form_is_valid = false;
      }
    }
    if (form_is_valid) {
      Confirm_Worker confirm_worker = Confirm_Worker (webserver_request);
      string initial_subject = translate("Signup verification");
      // Create the initial body of the email to send to the new user.
      xml_node node;
      xml_document initial_document;
      node = initial_document.append_child ("h3");
      node.text ().set (initial_subject.c_str());
      string information;
      if (config_logic_default_bibledit_configuration ()) {
        node = initial_document.append_child ("p");
        information = translate("There is a request to open an account with this email address.");
        node.text ().set (information.c_str());
      }
      if (config_logic_indonesian_cloud_free ()) {
        node = initial_document.append_child ("p");
        information = "Shalom " + user + "!";
        node.text ().set (information.c_str());
        node = initial_document.append_child ("p");
        node.text ().set ("Senang sekali Saudara ingin mendaftar sebagai Tamu Bibledit. Sebelum meng-klik tautan untuk mulai menggunakannya, mohon Saudara membaca berita penting ini:");
        node = initial_document.append_child ("p");
        node.text ().set ("• Cara yang terbaik masuk dalam Bibledit Tamu adalah melalui tautan yang terdapat di halaman dasar alkitabkita.info. Dengan demikian Saudara mendapat kesempatan untuk membaca pengumuman di halaman dasar situs kami. Dalam formulir yang terdapat di bagian atas halaman dasar alkitabkita.info isilah:");
        node = initial_document.append_child ("p");
        information = "Nama pengguna: " + user;
        node.text ().set (information.c_str());
        node = initial_document.append_child ("p");
        node.text ().set (R"(• Kalau Saudara lupa kata sandi, di halaman login kliklah tautan tentang “Aku lupa kata sandiku!””)");
        node = initial_document.append_child ("p");
        node.text ().set ("• Saudara diberi izin untuk menggunakan Bibledit sebagai Tamu selama satu bulan. Ketika bulan tersebut habis, tautan untuk mengunduh hasil terjemahanmu akan dikirim kepada Saudara di alamat email ini.");
        node = initial_document.append_child ("p");
        node.text ().set ("• Layanan Tamu Bibledit ini diberikan secara gratis dan Saudara dipersilahkan mendaftar ulang setiap bulan.");
        node = initial_document.append_child ("p");
        node.text ().set ("• Simpanlah email ini.");
      }
      string initial_body;
      {
        stringstream output;
        initial_document.print (output, "", format_raw);
        initial_body = output.str ();
      }
      string query;
      if (config_logic_default_bibledit_configuration ()) {
        query = database_users.add_userQuery (user, pass, Filter_Roles::member (), mail);
      }
      if (config_logic_indonesian_cloud_free ()) {
        // The Indonesian free Cloud new account should have the consultant role for things to work well.
        query = database_users.add_userQuery (user, pass, Filter_Roles::consultant (), mail);
      }
      // Create the contents for the confirmation email
      // that will be sent after the account has been verified.
      string subsequent_subject = translate("Account opened");
      xml_document subsequent_document;
      node = subsequent_document.append_child ("h3");
      node.text ().set (subsequent_subject.c_str());
      if (config_logic_default_bibledit_configuration ()) {
        node = subsequent_document.append_child ("p");
        information = translate("Welcome!");
        node.text ().set (information.c_str());
        node = subsequent_document.append_child ("p");
        information = translate("Your account is now active and you have logged in.");
        node.text ().set (information.c_str());
      }
      if (config_logic_indonesian_cloud_free ()) {
        node = subsequent_document.append_child ("p");
        information = "Shalom " + user + ",";
        node.text ().set (information.c_str());
        node = subsequent_document.append_child ("p");
        information = "Puji TUHAN, Saudara sudah menjadi Tamu Bibledit!";
        node.text ().set (information.c_str());
        node = subsequent_document.append_child ("p");
        information = "Kami mengajak Saudara supaya sesering mungkin mengunjungi situs alkitabkita.info untuk melihat pengumuman tentang kesempatan mengikuti pelatihan dan seminar zoom. Segeralah menonton semua video petunjuk yang terdapat pada halaman dasar.";
        node.text ().set (information.c_str());
        node = subsequent_document.append_child ("p");
        information = "Di tingkat Bibledit Tamu, Saudara dapat menggunakan Antarmuka Sederhana. Kami sarankan menggunakan Antarmuka Sederhana selama kurang lebih sebulan. Saat Saudara ingin menggunakan Antarmuka Lengkap yang lebih canggih dan powerful, silakan mendaftar untuk tingkat Bibledit Anggota.";
        node.text ().set (information.c_str());
        node = subsequent_document.append_child ("p");
        information = "Harga pendaftaran sebagai anggota adalah Rp 100.000,- setahun. Para anggota diberi izin menginstal program Bibledit dan sumber penelitiannya di komputer dan tablet. Dengan demikian Saudara dapat bekerja dengan Bibledit tanpa menggunakan pulsa data Internet. Lihat informasi lebih lanjut mengenai tingkat anggota di situs alkitabkita.info.";
        node.text ().set (information.c_str());
        node = subsequent_document.append_child ("p");
        information = "Kami tim situs alkitabkita.info sangat berharap dengan menggunakan Bibledit ini Saudara akan dimampukan meneliti Firman Tuhan secara lebih mendalam. Mohon jangan menggunakan kemampuan itu untuk membanggakan dirimu sendiri, tetapi gunakanlah untuk memuliakan TUHAN, untuk mengajar, dan menerjemahkan Firman TUHAN dengan lebih wajar, jelas, dan tepat.";
        node.text ().set (information.c_str());
        node = subsequent_document.append_child ("p");
        information = "Tuhan memberkati!";
        node.text ().set (information.c_str());
        node = subsequent_document.append_child ("p");
        information = "Balazi Gulo";
        node.text ().set (information.c_str());
        node = subsequent_document.append_child ("p");
        information = "Ketua Yayasan Albata";
        node.text ().set (information.c_str());
      }
      string subsequent_body;
      {
        stringstream output;
        subsequent_document.print (output, "", format_raw);
        subsequent_body = output.str ();
      }
      // Store the confirmation information in the database.
      confirm_worker.setup (mail, user, initial_subject, initial_body, query, subsequent_subject, subsequent_body);
      if (config_logic_indonesian_cloud_free ()) {
        // In the Indonesian free Cloud, create the Bible for the user.
        string bible = filter_indonesian_terjemahanku_mytranslation_name (user);
        tasks_logic_queue (CREATEEMPTYBIBLE, {bible});
      }
      // Done signup.
      signed_up = true;
    }
  }
  
  view.set_variable ("mailer", email_setup_information (true, false));

  if (signed_up) page += view.render ("session", "signedup");
  else page += view.render ("session", "signup");

  page += Assets_Page::footer ();

#endif

  return page;
}
