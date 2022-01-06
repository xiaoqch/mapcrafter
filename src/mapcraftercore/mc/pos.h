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

#ifndef POS_H_
#define POS_H_

#include <iostream>

/**
 * Here are some Minecraft position things. In Minecraft, x/z are the horizontal axes
 * and y is the vertical axis.
 *
 * Here are the Minecraft directions (also available as constants):
 * north = -z
 * south = +z
 * east = +x
 * west = -x
 */

namespace mapcrafter {
namespace mc {

/**
 * @brief RegionPos holds the position of a region in absolute coordinate in the world. The X and Z values are
 * used to set the filename, therefore are absolute world coordinates divided by 512 (16*32).
 */
class RegionPos {
public:
	int x, z;

	RegionPos();
	RegionPos(int x, int z);

	bool operator==(const RegionPos& other) const;
	bool operator!=(const RegionPos& other) const;
	bool operator<(const RegionPos& other) const;

	static RegionPos byFilename(const std::string& filename);
};

class BlockPos;

/**
 * @brief ChunkPos points to a position in a chunk. Therefore X and Z are never bigger than 15 or lower than 0.
   * However Y is still an absolute world Y as chunkas are not delimited in height. (yet...)
 */
class ChunkPos {
public:
	int x, z;

	ChunkPos();
	ChunkPos(int x, int z);
	ChunkPos(const BlockPos& block);

	int getLocalX() const;
	int getLocalZ() const;

	RegionPos getRegion() const;

	bool operator==(const ChunkPos& other) const;
	bool operator!=(const ChunkPos& other) const;
	bool operator<(const ChunkPos& other) const;
};

class LocalBlockPos;

/**
 * @brief BlockPos are absolute coordinate of a block in the world.
 */
class BlockPos {
public:
	int x, z, y;

	BlockPos();
	BlockPos(int x, int z, int y);

	bool operator==(const BlockPos& other) const;
	bool operator!=(const BlockPos& other) const;

	BlockPos& operator+=(const BlockPos& p);
	BlockPos& operator-=(const BlockPos& p);
	BlockPos operator+(const BlockPos& p2) const;
	BlockPos operator-(const BlockPos& p2) const;
};

extern const mc::BlockPos DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST, DIR_TOP, DIR_BOTTOM;

/**
 * @brief LocalBlockPos provides local coordinates inside a ChunkSection, which is a 16x16x16 world section.
 * All 3 coordinates are between 0 and 15 included.
 */
class LocalBlockPos {
public:
	int x, z, y;

	LocalBlockPos();
	LocalBlockPos(int x, int z, int y);
	LocalBlockPos(const BlockPos& pos);

	BlockPos toGlobalPos(const ChunkPos& chunk) const;
};

std::ostream& operator<<(std::ostream& stream, const BlockPos& block);
std::ostream& operator<<(std::ostream& stream, const RegionPos& region);
std::ostream& operator<<(std::ostream& stream, const ChunkPos& chunk);
std::ostream& operator<<(std::ostream& stream, const LocalBlockPos& block);

}
}

#endif
