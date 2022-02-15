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

#include "mcrandom.h"

namespace mapcrafter
{
  namespace renderer
  {

    MCRandom::MCRandom(int64_t seed)
    {
      this->seed = initialScramble(seed);
    }

    MCRandom::MCRandom(int32_t x, int32_t y, int32_t z)
    {
      int64_t seed = (int64_t)(x * 3129871) ^ (int64_t)z * 116129781L ^ (int64_t)y;
      seed = seed * seed * 42317861L + seed * 11L;
      seed >>= 16;
      this->seed = initialScramble(seed);
    }

    int64_t MCRandom::initialScramble(int64_t seed)
    {
      return (seed ^ MCRandom::mult) & MCRandom::mask;
    }

    int64_t MCRandom::nextLong()
    {
      int64_t mult = next(32);
      int64_t added = next(32);
      return (mult << 32) + added;
    }

    int32_t MCRandom::next(int16_t bits)
    {
      int64_t oldseed, nextseed;

      oldseed = this->seed;
      nextseed = (oldseed * MCRandom::mult + MCRandom::add) & MCRandom::mask;
      this->seed = nextseed;

      return (int32_t)(nextseed >> (48 - bits));
    }

  }
}
