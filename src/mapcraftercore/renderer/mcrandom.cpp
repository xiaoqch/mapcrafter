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

    MCRandom::MCRandom(long seed)
    {
      this->seed = initialScramble(seed);
    }

    MCRandom::MCRandom(long x, long y, long z)
    {
      long seed = (long)(x * 3129871L) ^ (long)z * 116129781L ^ (long)y;
      seed = seed * seed * 42317861L + seed * 11L;
      seed >>= 16;
      this->seed = initialScramble(seed);
    }

    long MCRandom::initialScramble(long seed)
    {
      return (seed ^ MCRandom::mult) & MCRandom::mask;
    }

    long MCRandom::nextLong()
    {
      return ((long)(next(32)) << 32) + next(32);
    }

    long MCRandom::next(long bits)
    {
      long oldseed, nextseed;

      oldseed = this->seed;
      nextseed = (oldseed * MCRandom::mult + MCRandom::add) & MCRandom::mask;
      this->seed = nextseed;

      return (long)(nextseed >> (48 - bits));
    }

  }
}
