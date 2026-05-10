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

#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wcharacter-conversion"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <database/login.h>
#include <database/state.h>
#include <database/config/bible.h>
#include <database/config/general.h>
#include <demo/logic.h>
#include <filter/date.h>
#include <filter/string.h>
#include <filter/url.h>
#include <unittests/utilities.h>
#include "webserver/request.h"


TEST(database, config_general)
{
    refresh_sandbox(false);

    using namespace database::config::general;

    // Test set/get std::string and default value.
    EXPECT_EQ("Cloud", get_site_mail_name ());
    {
        constexpr const char* cloud {"cloud"};
        set_site_mail_name(cloud);
        EXPECT_EQ(cloud, get_site_mail_name ());
    }
    set_site_mail_name(std::string());
    EXPECT_EQ(std::string(), get_site_mail_name ());
    EXPECT_EQ(std::string(), get_mail_storage_protocol ());

    // Test set/get/default of boolean.
    {
        EXPECT_TRUE(get_menu_in_tabbed_view_on());
        set_menu_in_tabbed_view_on(false);
        EXPECT_FALSE(get_menu_in_tabbed_view_on());
        EXPECT_FALSE(get_just_connected_to_cloud());
        set_just_connected_to_cloud(true);
        EXPECT_TRUE(get_just_connected_to_cloud());
    }

    // Test set/get/default of integer.
    {
        EXPECT_EQ(get_server_port(), 8080);
        set_server_port(8090);
        EXPECT_EQ(get_server_port(), 8090);
        EXPECT_EQ(get_repeat_send_receive(), 0);
        set_repeat_send_receive(3);
        EXPECT_EQ(get_repeat_send_receive(), 3);
    }

    // Test set/get vector of strings.
    {
        EXPECT_TRUE(get_resources_to_cache().empty());
        const auto standard = std::vector<std::string> {"a", "b", "c"};
        set_resources_to_cache(standard);
        EXPECT_EQ(get_resources_to_cache(), standard);
        set_resources_to_cache({});
        EXPECT_TRUE(get_resources_to_cache().empty());
    }
}


TEST(database, config_bible)
{
    using namespace database::config::bible;

    // Test get/set string.
    EXPECT_EQ(get_versification_system("phpunit"), filter::string::english ());
    EXPECT_EQ(get_versification_system("x"), filter::string::english ());
    set_versification_system("phpunit", "Versification");
    EXPECT_EQ(get_versification_system("phpunit"), "Versification");
    EXPECT_EQ(get_editor_stylesheet("b"), "Standard");

    // Check default value for Bible.
    {
        std::string bible = "A Bible";
        std::string standard = ", ;";
        std::string suffix = " suffix";
        std::string value = get_sentence_structure_middle_punctuation(bible);
        EXPECT_EQ(standard, value);
        // Change value and check it.
        set_sentence_structure_middle_punctuation(bible, standard + suffix);
        value = get_sentence_structure_middle_punctuation(bible);
        EXPECT_EQ(standard + suffix, value);
        // Remove that Bible and check that the value is back to default.
        database::config::bible::remove(bible);
        value = get_sentence_structure_middle_punctuation(bible);
        EXPECT_EQ(standard, value);
    }

    // Test get/set/default of boolean value.
    EXPECT_FALSE(get_export_web_during_night("test"));
    set_export_web_during_night("test", true);
    EXPECT_TRUE(get_export_web_during_night("test"));
    EXPECT_TRUE(get_public_feedback_enabled("b"));
    EXPECT_FALSE(get_read_from_git("b"));

    // Test get/set/default of integer value.
    EXPECT_EQ(get_line_height("b"), 100);
    set_line_height("b", 200);
    EXPECT_EQ(get_line_height("b"), 200);
    EXPECT_EQ(get_repeat_send_receive("test"), 0);
}


// Test the user configuration database.
TEST(database, config_user)
{
    // Set it up.
    refresh_sandbox(true);
    Webserver_Request request;
    Database_State::create();
    Database_Login::create();
    Database_Users database_users;
    database_users.create();
    database_users.upgrade();
    std::string username = "username";
    std::string password = "password";
    database_users.add_user(username, password, 5, "");
    request.session_logic()->attempt_login(username, password, true);

    // Testing setList, getList, plus add/removeUpdatedSetting.
    {
        EXPECT_TRUE(request.database_config_user()->get_updated_settings().empty());

        std::vector standard = {123, 456};
        request.database_config_user()->set_updated_settings(standard);
        EXPECT_EQ(standard, request.database_config_user()->get_updated_settings());

        request.database_config_user()->add_updated_setting(789);
        standard.push_back(789);
        EXPECT_EQ(standard, request.database_config_user()->get_updated_settings());

        request.database_config_user()->remove_updated_setting(456);
        standard.erase(std::ranges::find(standard, 456));
        EXPECT_EQ(request.database_config_user()->get_updated_settings(), standard);
    }

    // Testing the Sprint month and trimming it.
    // It should get today's month.
    {
        int month = filter::date::numerical_month(filter::date::seconds_since_epoch());
        EXPECT_EQ(month, request.database_config_user ()->get_sprint_month ());
        // Set the sprint month to another month value: It should get this value back from the database.
        int newmonth = 123;
        request.database_config_user()->set_sprint_month(newmonth);
        EXPECT_EQ(newmonth, request.database_config_user ()->get_sprint_month ());
        // Trim: The sprint month should not be reset.
        request.database_config_user()->trim();
        EXPECT_EQ(newmonth, request.database_config_user ()->get_sprint_month ());
        // Set the modification time of the sprint month record to more than two days ago:
        // Trimming resets the sprint month to the current month.
        std::string filename = filter_url_create_path({
            testing_directory, "databases", "config", "user", "username", "sprint-month"
        });
        struct stat foo{};
        utimbuf new_times{};
        stat(filename.c_str(), &foo);
        new_times.actime = filter::date::seconds_since_epoch() - (2 * 24 * 3600) - 10;
        new_times.modtime = filter::date::seconds_since_epoch() - (2 * 24 * 3600) - 10;
        utime(filename.c_str(), &new_times);
        request.database_config_user()->trim();
        EXPECT_EQ(month, request.database_config_user ()->get_sprint_month ());
    }

    // Test boolean setting.
    EXPECT_EQ(false, request.database_config_user ()->get_subscribe_to_consultation_notes_edited_by_me ());
    request.database_config_user()->set_subscribe_to_consultation_notes_edited_by_me(true);
    EXPECT_EQ(true, request.database_config_user ()->get_subscribe_to_consultation_notes_edited_by_me ());

    // Test integer setting.
    EXPECT_EQ(1, request.database_config_user ()->get_consultation_notes_passage_selector ());
    request.database_config_user()->set_consultation_notes_passage_selector(11);
    EXPECT_EQ(11, request.database_config_user ()->get_consultation_notes_passage_selector ());

    // Test string setting.
    EXPECT_EQ(std::string(), request.database_config_user ()->get_consultation_notes_assignment_selector ());
    request.database_config_user()->set_consultation_notes_assignment_selector("test");
    EXPECT_EQ("test", request.database_config_user ()->get_consultation_notes_assignment_selector ());

    // Sprint year.
    EXPECT_EQ(filter::date::numerical_year (filter::date::seconds_since_epoch ()),
              request.database_config_user ()->get_sprint_year ());

    // Test getting a Bible that does not exist: It creates one.
    EXPECT_EQ(demo_sample_bible_name (), request.database_config_user ()->get_bible ());

    // Test that after removing a user, the setting reverts to its default value.
    EXPECT_EQ(0, request.database_config_user ()->get_consultation_notes_text_inclusion_selector ());
    request.database_config_user()->set_consultation_notes_text_inclusion_selector(1);
    EXPECT_EQ(1, request.database_config_user ()->get_consultation_notes_text_inclusion_selector ());
    request.database_config_user()->remove(username);
    EXPECT_EQ(0, request.database_config_user ()->get_consultation_notes_text_inclusion_selector ());

    // Test setting privileges for a user, and the user retrieving them.
    {
        // Privilege is on by default.
        EXPECT_EQ(true, request.database_config_user ()->get_privilege_use_advanced_mode ());
        // Privilege is on for another user also.
        std::string anotheruser = "anotheruser";
        EXPECT_EQ(true, request.database_config_user ()->get_privilege_use_advanced_mode_for_user (anotheruser));
        // Set it off for the other user.
        request.database_config_user()->set_privilege_use_advanced_mode_for_user(anotheruser, false);
        EXPECT_EQ(false, request.database_config_user ()->get_privilege_use_advanced_mode_for_user (anotheruser));
        // The privilege is still on for the current user.
        EXPECT_EQ(true, request.database_config_user ()->get_privilege_use_advanced_mode ());
    }

    // Filter allowed journal entries.
    refresh_sandbox(true, {"Creating sample Bible", "Sample Bible was created"});
}

#endif

