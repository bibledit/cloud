#include <dialog/list2.h>



// Generate the option tags based on the inserted key and its value.
string Options_To_Select::add_selection (string text, string value, string html)
{
    if (value == "") {
        html.append ("<option value=''>" + text + "</option>");
    } else {
        html.append ("<option value='" + value + "'>" + text + "</option>");
    }

    return html;
}


// Mark the current selected option's option tag.
string Options_To_Select::mark_selected (string value, string html)
{
    string new_value = "value='" + value + "'";
    int new_pos = html.find (new_value) + new_value.length ();
    string mark = " selected";

    if (html.find (mark) != string::npos) {
        html.erase (html.find (mark), mark.length ());
    }

    html.insert (new_pos, mark);

    return html;
}
