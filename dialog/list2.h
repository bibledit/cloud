#ifndef INCLUDED_OPTIONS_TO_SELECT_H
#define INCLUDED_OPTIONS_TO_SELECT_H


#include <config/libraries.h>


class Options_To_Select
{
public:
  static void add_selection (string text, string value);
  static void mark_selected (string value);
  static string return_tags ();
};


#endif
