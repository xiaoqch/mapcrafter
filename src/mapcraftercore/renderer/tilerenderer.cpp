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

#include "blockimages.h"
#include "rendermode.h"
#include "renderview.h"
#include "tileset.h"
#include "mcrandom.h"
#include "../mc/blockstate.h"
#include "../mc/pos.h"
#include "../util.h"

namespace mapcrafter {
namespace renderer {

bool TileImage::operator<(const TileImage& other) const {
	if (pos == other.pos) {
		return z_index < other.z_index;
	}
	return pos < other.pos;
}

TileRenderer::TileRenderer(const RenderView* render_view, mc::BlockStateRegistry& block_registry,
		BlockImages* images, int tile_width, mc::WorldCache* world, RenderMode* render_mode)
	: block_registry(block_registry), images(images), block_images(dynamic_cast<RenderedBlockImages*>(images)),
	  tile_width(tile_width), world(world), current_chunk(nullptr),
	  render_mode(render_mode),
	  render_biomes(true), shadow_edges({0, 0, 0, 0, 0}) {
	assert(block_images);
	render_mode->initialize(render_view, images, world, &current_chunk);
}

TileRenderer::~TileRenderer() {
}

void TileRenderer::setRenderBiomes(bool render_biomes) {
	this->render_biomes = render_biomes;
}

void TileRenderer::setShadowEdges(std::array<uint8_t, 5> shadow_edges) {
	this->shadow_edges = shadow_edges;
}

void TileRenderer::renderTile(const TilePos& tile_pos, RGBAImage& tile) {
	tile.setSize(getTileWidth(), getTileHeight());

	std::set<TileImage> tile_images;
	renderTopBlocks(tile_pos, tile_images);

	for (auto it = tile_images.begin(); it != tile_images.end(); ++it) {
		tile.alphaBlit(it->image, it->x, it->y);
	}
}

int TileRenderer::getTileWidth() const {
	return getTileSize();
}

int TileRenderer::getTileHeight() const {
	return getTileSize();
}

void TileRenderer::renderBlocks(int x, int y, mc::BlockPos top, const mc::BlockPos& dir, std::set<TileImage>& tile_images) {
	// const int water_id =
	// 	block_registry.getBlockID(
	// 		mc::BlockState("minecraft:water"));
	// const BlockImage& waterlog_image =
	// 	block_images->getBlockImage(
	// 		block_registry.getBlockID(
	// 			mc::BlockState::parse("minecraft:water_mask", "level=8")));
	const BlockImage& waterlog_full_image =
		block_images->getBlockImage(
			block_registry.getBlockID(
				mc::BlockState::parse("minecraft:water_mask", "level=7")));

	// Pre-allocate rendering buffers
	TileImage tile_image;
	tile_image.image.setSize(waterlog_full_image.image().width, waterlog_full_image.image().height);
	RGBAImage waterLogTinted(tile_image.image.width, tile_image.image.height);

	for (; top.y >= mc::CHUNK_LOW*16 ; top += dir) {
		// get current chunk position
		mc::ChunkPos current_chunk_pos(top);

		// check if current chunk is not null
		// and if the chunk wasn't replaced in the cache (i.e. position changed)
		if (current_chunk == nullptr || current_chunk->getPos() != current_chunk_pos) {
			//if (!state.world->hasChunkSection(current_chunk, top.current.y))
			//	continue;
			current_chunk = world->getChunk(current_chunk_pos);
		}
		if (current_chunk == nullptr) {
			continue;
		}

		// get local block position
		mc::LocalBlockPos local(top);

		uint16_t id = current_chunk->getBlockID(local);
		const BlockImage* block_image = &block_images->getBlockImage(id);

		// Early rejection if nothing to draw
		if ((block_image->is_empty && !block_image->is_waterlogged)
			|| render_mode->isHidden(top, *block_image)) {
			continue;
		}

		// What's on each side ?
		uint16_t id_top   = current_chunk->getBlockID(mc::LocalBlockPos(local.x,local.z,local.y+1));
		uint16_t id_south = getBlock(top + mc::DIR_SOUTH).id;
		uint16_t id_west  = getBlock(top + mc::DIR_WEST).id;
		const BlockImage* block_image_top = &block_images->getBlockImage(id_top);
		const BlockImage* block_image_south = &block_images->getBlockImage(id_south);
		const BlockImage* block_image_west = &block_images->getBlockImage(id_west);

		auto is_full_water = [](const BlockImage* b) -> bool {
			return b->is_empty &&
				b->is_waterlogged;
		};
		auto is_waterlogged = [](const BlockImage* b) -> bool {
			return b->is_waterlogged;
		};

		bool water_top = is_waterlogged(block_image_top);
		bool water_south = is_waterlogged(block_image_south);
		bool water_west = is_waterlogged(block_image_west);

		// Early rejection if water with waterloged with neighbours
		if (is_full_water(block_image)
			&& water_top
			&& water_south
			&& water_west)
			continue;

		// Retrieve the image to print
		// mc::BlockPos global_pos = mc::LocalBlockPos(local.x, local.z, local.y).toGlobalPos(current_chunk->getPos());
		// MCRandom rnd(global_pos.x,global_pos.y,global_pos.z);
		MCRandom rnd(local.x,local.y,local.z);
		int alt = 0; // abs(rnd.nextLong());
		const RGBAImage& image = block_image->image(alt);
		const RGBAImage& uv_image = block_image->uv_image(alt);

		// Prep the tile
		tile_image.x = x;
		tile_image.y = y;
		tile_image.pos = top;
		tile_image.z_index = 0;
		tile_image.image.setSize(image.width,image.height);

		// Only display if there's something to print
		// This applies for water blocks, where we print
		// the water on the next step
		if (!block_image->is_empty) {

			bool strip_up = false;
			bool strip_left = false;
			bool strip_right = false;
			if (block_image->can_partial) {
				strip_up    = id == id_top;
				strip_right = id == id_south;
				strip_left  = id == id_west;
			} else if (!block_image->is_transparent) {
				strip_up    = block_images->getBlockImage(id_top).is_transparent   == false;
				strip_right = block_images->getBlockImage(id_south).is_transparent == false;
				strip_left  = block_images->getBlockImage(id_west).is_transparent  == false;
			}

			if (strip_up || strip_left || strip_right) {
				for (int i=0; i<tile_image.image.width*tile_image.image.height; i++) {
					RGBAPixel puv = uv_image.data[i];
					RGBAPixel p = image.data[i];
					switch(rgba_blue(puv)) {
						case FACE_UP_INDEX:
							if (strip_up) {
								p = 0;
							}
							break;
						case FACE_LEFT_INDEX:
							if (strip_left) {
								p = 0;
							}
							break;
						case FACE_RIGHT_INDEX:
							if (strip_right) {
								p = 0;
							}
							break;
					}
					tile_image.image.data[i] = p;
				}
			} else {
				std::copy(image.data.begin(), image.data.end(), tile_image.image.data.begin());
			}

			if (block_image->is_biome) {
				block_images->prepareBiomeBlockImage(tile_image.image, *block_image, getBiomeColor(top, *block_image, current_chunk));
			}

		} else {
			// Clear out the tile from previous rendering
			std::fill(tile_image.image.data.begin(), tile_image.image.data.end(), 0);
		}

		if (block_image->is_waterlogged) {
			assert( !(water_top && water_south && water_west) );

			const RGBAImage* waterlog;
			const RGBAImage* waterlog_uv;
			if (water_top) {
				// This will be displayed as full water
				waterlog = &waterlog_full_image.image();
				waterlog_uv = &waterlog_full_image.uv_image();
			} else {
				std::string level = "6"; //block_registry.getBlockState(id).getProperty("level", "4");
				const BlockImage& waterlog_image =
					block_images->getBlockImage(
						block_registry.getBlockID(
							mc::BlockState::parse("minecraft:water_mask", std::string("level=")+level )));
				// That one will be displayed a bit lower to look like a shore line
				waterlog = &waterlog_image.image();
				waterlog_uv = &waterlog_image.uv_image();
			}

			uint32_t biome_color = getBiomeColor(top, waterlog_full_image, current_chunk);
			// biome_color = rgba( rgba_alpha(biome_color)>>2, rgba_red(biome_color), rgba_green(biome_color), rgba_blue(biome_color));

			std::vector<RGBAPixel>::const_iterator pit      = waterlog->data.begin();
			std::vector<RGBAPixel>::const_iterator pitend   = waterlog->data.end();
			std::vector<RGBAPixel>::const_iterator puvit    = waterlog_uv->data.begin();
			// std::vector<RGBAPixel>::const_iterator pimguvit = uv_image.data.begin();
			std::vector<RGBAPixel>::iterator pdestit        = waterLogTinted.data.begin();

			// if ((water_top || water_south || water_west) == false) {
			// 	// fast lane
			// 	// Nothing to clip, just render the whole water block with biome color
			// 	while (pit != pitend)
			// 	{
			// 		RGBAPixel p = *pit;
			// 		if (p) {
			// 			p = rgba_multiply(p, biome_color);
			// 		}
			// 		*pdestit = p;
			// 		pit ++;
			// 		puvit ++;
			// 		pimguvit ++;
			// 		pdestit ++;
			// 	}
			// } else {
				// Clip the some faces, and multiply by biome color
				while (pit != pitend)
				{
					RGBAPixel p = *pit;
					if (p) {
						RGBAPixel puv = *puvit;
						switch(rgba_blue(puv)){
							case FACE_UP_INDEX:
								if(water_top) {
									p = 0;
								}
								break;
							case FACE_LEFT_INDEX:
								if(water_west) {
									p = 0;
								}
								break;
							case FACE_RIGHT_INDEX:
								if(water_south) {
									p = 0;
								}
								break;
						}
						if (p) {
							p = rgba_multiply(p, biome_color);
						}
					}
					*pdestit = p;
					pit ++;
					puvit ++;
					// pimguvit ++;
					pdestit ++;
				}
			// }

		// if (rgba_alpha(uv_pixel) < rgba_alpha(top_uv_pixel)) {
		// 	blend(pixel, top_pixel);
		// } else {
		// 	// The top pixel is behind the block one, so use the alpha of
		// 	// the destination pixel to blend the top pixel behind
		// 	RGBAPixel tmp_pix = pixel;
		// 	pixel = top_pixel;
		// 	blend(pixel, tmp_pix);
		// }


			blockImageBlendZBuffered(tile_image.image, uv_image, waterLogTinted, *waterlog_uv);
		}

		if (block_image->shadow_edges > 0) {
			auto shadow_edge = [this, top](const mc::BlockPos& dir) {
				const BlockImage& b = block_images->getBlockImage(getBlock(top + dir).id);
				//return b.is_transparent && !(b.is_waterlogged || b.is_waterlogged);
				return b.shadow_edges == 0;
			};
			uint8_t north = shadow_edges[0] && shadow_edge(mc::DIR_NORTH);
			uint8_t south = shadow_edges[1] && shadow_edge(mc::DIR_SOUTH);
			uint8_t east = shadow_edges[2] && shadow_edge(mc::DIR_EAST);
			uint8_t west = shadow_edges[3] && shadow_edge(mc::DIR_WEST);
			uint8_t bottom = shadow_edges[4] && shadow_edge(mc::DIR_BOTTOM);

			if (north + south + east + west + bottom != 0) {
				int f = block_image->shadow_edges;
				north *= shadow_edges[0] * f;
				south *= shadow_edges[1] * f;
				east *= shadow_edges[2] * f;
				west *= shadow_edges[3] * f;
				bottom *= shadow_edges[4] * f;
				blockImageShadowEdges(tile_image.image, uv_image,
					north, south, east, west, bottom);
			}
		}

		// let the render mode do their magic with the block image
		//render_mode->draw(node.image, node.pos, id, data);
		render_mode->draw(tile_image.image, *block_image, tile_image.pos, id);
		tile_images.insert(tile_image);

		// if this block is not transparent, then stop looking for more blocks
		if (!block_image->is_transparent) {
			break;
		}
	}
}

mc::Block TileRenderer::getBlock(const mc::BlockPos& pos, int get) {
	return world->getBlock(pos, current_chunk, get);
}

uint32_t TileRenderer::getBiomeColor(const mc::BlockPos& pos, const BlockImage& block, const mc::Chunk* chunk) {
	const int radius = 2;
	float f = ((2*radius+1)*(2*radius+1));
	float r = 0.0, g = 0.0, b = 0.0;

	for (int dx = -radius; dx <= radius; dx++) {
		for (int dz = -radius; dz <= radius; dz++) {
			mc::BlockPos other = pos + mc::BlockPos(dx, dz, 0);
			mc::ChunkPos chunk_pos(other);

			uint16_t biome_id;
			mc::LocalBlockPos local(other);
			if (chunk_pos != chunk->getPos()) {
				mc::Chunk* other_chunk = world->getChunk(chunk_pos);
				if (other_chunk == nullptr) {
					f -= 1.0f;
					continue;
				}
				biome_id = other_chunk->getBiomeAt(local);
			} else {
				biome_id = chunk->getBiomeAt(local);
			}
			Biome biome = getBiome(biome_id);
			uint32_t c = biome.getColor(other, block.biome_color, block.biome_colormap);
			r += (float) rgba_red(c);
			g += (float) rgba_green(c);
			b += (float) rgba_blue(c);
		}
	}

	return rgba(r / f, g / f, b / f, 255);
}

}
}
