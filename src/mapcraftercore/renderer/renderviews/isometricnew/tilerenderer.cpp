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

#include "tilerenderer.h"

#include "../../biomes.h"
#include "../../blockimages.h"
#include "../../image.h"
#include "../../rendermode.h"
#include "../../tileset.h"
#include "../../../mc/blockstate.h"
#include "../../../mc/pos.h"
#include "../../../mc/worldcache.h"
#include "../../../util.h"

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace mapcrafter {
namespace renderer {

namespace old {

TileTopBlockIterator::TileTopBlockIterator(const TilePos& tile, int block_size,
		int tile_width)
		: block_size(block_size), is_end(false) {
	// row/col 0,0 are the top left chunk of the tile 0,0
	// each tile is four rows high, two columns wide

	// at first get the chunk, whose row and column is at the top right of the tile
	// top right chunk of a tile is the top left chunk of the tile x+1,y
	mc::ChunkPos topright_chunk = mc::ChunkPos::byRowCol(
			4 * tile_width * tile.getY(),
			2 * tile_width * (tile.getX() + 1), 0);

	// now get the first visible block from this chunk in this tile
	top = mc::LocalBlockPos(8, 6, mc::CHUNK_TOP * 16 - 1).toGlobalPos(topright_chunk);
	// and set this as start
	current = top;

	// calculate bounds of the tile
	min_row = top.getRow() + 1;
	max_row = top.getRow() + (64 * tile_width) + 4;
	max_col = top.getCol() + 2;
	min_col = max_col - (32 * tile_width);

	// calculate position of the first block, relative row/col in this tile are needed
	int row = current.getRow() - min_row;
	int col = current.getCol() - min_col;
	// every column is a 1/2 block and every row is a 1/4 block
	draw_x = col * block_size / 2;
	// -1/2 blocksize, because we would see the top side of the blocks in the tile if not
	draw_y = row * block_size / 4 - block_size / 2; // -16
}

TileTopBlockIterator::~TileTopBlockIterator() {

}

void TileTopBlockIterator::next() {
	if (is_end)
		return;

	// go one block to bottom right (z+1)
	current += mc::BlockPos(0, 1, 0);

	// check if row/col is too big
	if (current.getCol() > max_col || current.getRow() > max_row) {
		// move the top one block to the left
		top -= mc::BlockPos(1, 1, 0);
		// and set the current block to the top block
		current = top;

		// check if the current top block is out of the tile
		if (current.getCol() < min_col - 1) {
			// then move it by a few blocks to bottom right
			current += mc::BlockPos(0, min_col - current.getCol() - 1, 0);
		}
	}

	// now calculate the block position like in the constructor
	int row = current.getRow();
	int col = current.getCol();
	draw_x = (col - min_col) * block_size / 2;
	draw_y = (row - min_row) * block_size / 4 - block_size / 2; // -16

	// and set end if reached
	if (row == max_row && col == min_col)
		is_end = true;
	else if (row == max_row && col == min_col + 1)
		is_end = true;
}

bool TileTopBlockIterator::end() const {
	return is_end;
}

BlockRowIterator::BlockRowIterator(const mc::BlockPos& block) {
	current = block;
}

BlockRowIterator::~BlockRowIterator() {

}

void BlockRowIterator::next() {
	//current += mc::BlockPos(1, -1, -1);
	current.x++;
	current.z--;
	current.y--;
}

bool BlockRowIterator::end() const {
	return current.y < 0;
}

bool RenderBlock::operator<(const RenderBlock& other) const {
	return pos < other.pos;
}

}

NewIsometricTileRenderer::NewIsometricTileRenderer(const RenderView* render_view,
		mc::BlockStateRegistry& block_registry,
		BlockImages* images, int tile_width, mc::WorldCache* world,
		RenderMode* render_mode)
	: TileRenderer(render_view, block_registry, images, tile_width, world, render_mode) {
}

NewIsometricTileRenderer::~NewIsometricTileRenderer() {
}

int NewIsometricTileRenderer::getTileSize() const {
	return images->getBlockSize() * 16 * tile_width;
}

void NewIsometricTileRenderer::renderTopBlocks(const TilePos& tile_pos, std::set<TileImage>& tile_images) {
	int block_size = images->getBlockSize();
	for (old::TileTopBlockIterator it(tile_pos, block_size, tile_width); !it.end(); it.next()) {
		renderBlocks(it.draw_x, it.draw_y, it.current, mc::BlockPos(1, -1, -1), tile_images);
	}
}

}
}
