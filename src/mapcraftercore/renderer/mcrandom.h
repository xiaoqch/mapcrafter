/*
 * Copyright 2012-2016 Moritz Hilscher
 *
 * This file is part of Mapcrafter.
 *
 * Mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MCRANDOM_H_
#define MCRANDOM_H_

#include <sys/types.h>

namespace mapcrafter
{
  namespace renderer
  {

    class MCRandom
    {
    private:
      int64_t seed;

    public:
      MCRandom(int64_t seed);
      MCRandom(int32_t x, int32_t y, int32_t z);

      // Return the next value
      int64_t nextLong();

    protected:
      int32_t next(int16_t bits);

      static const int64_t mult = 0x5DEECE66DL;
      static const int64_t add = 0xBL;
      static const int64_t mask = (1L << 48) - 1;

    private:
      static int64_t initialScramble(int64_t seed);
    };

  }
}

#endif
