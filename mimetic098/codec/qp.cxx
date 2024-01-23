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

    $Id: qp.cxx,v 1.3 2008-10-07 11:06:26 tat Exp $
 ***************************************************************************/
#include <mimetic098/codec/qp.h>

using namespace mimetic;


char QP::sTb[] = {
   4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 
   3, 4, 4, 3, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 

//                      %, &, ', 
   4, 4, 2, 5, 5, 5, 5, 0, 0, 0, 

// (, ), *, +, ,, -, ., /, 0, 1, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// 2, 3, 4, 5, 6, 7, 8, 9, :, ;, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// <,    >, ?,    A, B, C, D, E, 
   0, 5, 0, 0, 5, 0, 0, 0, 0, 0, 

// F, G, H, I, J, K, L, M, N, O, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// P, Q, R, S, T, U, V, W, X, Y, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// Z,             _,    a, b, c, 
   0, 5, 5, 5, 5, 0, 5, 0, 0, 0, 

// d, e, f, g, h, i, j, k, l, m, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// n, o, p, q, r, s, t, u, v, w, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// x, y, z,                      
   0, 0, 0, 5, 5, 5, 5, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 
};

