#ifndef INCLUDED_OPTIONS_TO_SELECT_H
#define INCLUDED_OPTIONS_TO_SELECT_H


#include <config/libraries.h>


class Options_To_Select
{
public:
  static string add_selection (string text, string value, string html);
  static string mark_selected (string value, string html);
};


#endif
