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

#ifndef RENDER_ROTATION_H_
#define RENDER_ROTATION_H_

#include "../mc/pos.h"

namespace mapcrafter {

namespace renderer {

class RenderRotation {
  public:
	/**
	 * @brief The different type of rotation available
	 */
	typedef enum {
		TOP_LEFT     = 0,
		TOP_RIGHT    = 1,
		BOTTOM_RIGHT = 2,
		BOTTOM_LEFT  = 3,
		ALL          = 4,
	} Direction;

	/**
	 * @brief Multiple type of constructor to accomodate multiple situations
	 */
	RenderRotation() : rotation(ALL) { updateVectors(); }
	RenderRotation(int rotation) : rotation((Direction)rotation) { updateVectors(); }
	RenderRotation(const RenderRotation& rot) : rotation(rot.rotation) { updateVectors(); }

	void setRotation(Direction dir) {
		rotation = dir;
		updateVectors();
	}

	/**
	 * @brief Convert to an integer, so to be used like index in a vector, map, array, ...
	 * @param dir
	 */
	operator int() const { return (int)this->rotation; };
	operator int() { return (int)this->rotation; };
	explicit operator Direction() const { return this->rotation; };
	explicit operator Direction() { return this->rotation; };

	// All directions rotated
	const Direction     getRotation() const { return rotation; }
	const mc::BlockPos& getNorth() const { return north; }
	const mc::BlockPos& getEast() const { return east; }
	const mc::BlockPos& getSouth() const { return south; }
	const mc::BlockPos& getWest() const { return west; }
	const mc::BlockPos& getTop() const { return top; }
	const mc::BlockPos& getBottom() const { return bottom; }

	const mc::BlockPos rotate(const mc::BlockPos pos) const {
		switch (getRotation()) {
			default:
			case TOP_LEFT:
				return pos;
			case TOP_RIGHT:
				return mc::BlockPos(+pos.z, -pos.x, pos.y);
			case BOTTOM_RIGHT:
				return mc::BlockPos(-pos.x, -pos.z, pos.y);
			case BOTTOM_LEFT:
				return mc::BlockPos(-pos.z, +pos.x, pos.y);
		}
	}

	const mc::ChunkPos rotate(const mc::ChunkPos pos) const {
		switch (getRotation()) {
			default:
			case TOP_LEFT:
				return pos;
			case TOP_RIGHT:
				return mc::ChunkPos(+pos.z, -pos.x);
			case BOTTOM_RIGHT:
				return mc::ChunkPos(-pos.x, -pos.z);
			case BOTTOM_LEFT:
				return mc::ChunkPos(-pos.z, +pos.x);
		}
	}

  private:
	void updateVectors() {
		north  = rotate(mc::DIR_NORTH);
		east   = rotate(mc::DIR_EAST);
		south  = rotate(mc::DIR_SOUTH);
		west   = rotate(mc::DIR_WEST);
		top    = rotate(mc::DIR_TOP);
		bottom = rotate(mc::DIR_BOTTOM);
	}

	Direction    rotation;
	mc::BlockPos north;
	mc::BlockPos east;
	mc::BlockPos south;
	mc::BlockPos west;
	mc::BlockPos top;
	mc::BlockPos bottom;
};

} /* namespace renderer */
} /* namespace mapcrafter */

#endif /* RENDER_ROTATION_H_ */
