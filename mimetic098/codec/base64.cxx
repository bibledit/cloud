#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"

#pragma GCC diagnostic ignored "-Wconversion"

/***************************************************************************
    copyright            : (C) 2002-2008 by Stefano Barbato
    email                : stefano@codesink.org

    $Id: base64.cxx,v 1.3 2008-10-07 11:06:26 tat Exp $
 ***************************************************************************/

#pragma GCC diagnostic ignored "-Wsign-conversion"

#include <mimetic098/codec/base64.h>

using namespace mimetic;

const char Base64::sEncTable[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/=";

const char Base64::sDecTable[] = {
        static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),
        static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),
        static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),
        static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),
        static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),62,static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),63,52,53,
        54,55,56,57,58,59,60,61,static_cast<char>(-1),static_cast<char>(-1),
        static_cast<char>(-1), eq_sign, static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1), 0, 1, 2, 3, 4,
         5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,
        25,static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),static_cast<char>(-1),26,27,28,
        29,30,31,32,33,34,35,36,37,38,
        39,40,41,42,43,44,45,46,47,48,
        49,50,51,static_cast<char>(-1)
};

const int Base64::sDecTableSz = sizeof(Base64::sDecTable) / sizeof(char);

