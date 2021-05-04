#include <dialog/list2.h>


string option_tags;
string to_be_returned;


// Generate the option tags based on the inserted key and its value.
void Options_To_Select::add_selection (string text, string value)
{
    if (value == "") {
        option_tags.append ("<option value=''>" + text + "</option>");
    } else {
        option_tags.append ("<option value='" + value + "'>" + text + "</option>");
    }
}


// Mark the current selected option's option tag.
void Options_To_Select::mark_selected (string value)
{
    string new_value = "value='" + value + "'";
    int new_pos = option_tags.find (new_value) + new_value.length ();
    string mark = " selected";

    if (option_tags.find (mark) != string::npos) {
        option_tags.erase (option_tags.find (mark), mark.length ());
    }

    option_tags.insert (new_pos, mark);
    to_be_returned = option_tags;
    option_tags = "";
}


// Return the resultant string and clean up the string for future use.
string Options_To_Select::return_tags ()
{
    return to_be_returned;
}
