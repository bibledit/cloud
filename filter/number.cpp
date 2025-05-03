/*
Copyright (©) 2003-2025 Teus Benschop.

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


#include <filter/number.h>
#include <config/libraries.h>


namespace filter::number {


bool float_equal(const float x, const float y)
{
  // The `epsilon()` is the gap size (ULP, unit in the last place)
  // of floating-point numbers in interval [1, 2).
  // Therefore we can scale it to the gap size in interval [2^e, 2^{e+1}),
  // where `e` is the exponent of `x` and `y`.
  
  // If `x` and `y` have different gap sizes
  // (which means they have different exponents),
  // we take the smaller one.
  // Taking the bigger one is also reasonable, I guess.
  const float m = std::min(std::fabs(x), std::fabs(y));
  
  // Subnormal numbers have fixed exponent, which is `min_exponent - 1`.
  const int exp = m < std::numeric_limits<float>::min()
  ? std::numeric_limits<float>::min_exponent - 1
  : std::ilogb(m);
  
  // We consider `x` and `y` equal if the difference between them is within one ULP.
  return std::fabs(x - y) <= std::ldexp(std::numeric_limits<float>::epsilon(), exp);
}


}
