#ifndef PRINCE_WALK_SPRITE_H
#define PRINCE_WALK_SPRITE_H

#include <stdint.h>

#define PRINCE_WALK_SPRITE_WIDTH 22U
#define PRINCE_WALK_SPRITE_HEIGHT 28U
#define PRINCE_WALK_SPRITE_ROW_BYTES 3U
#define PRINCE_WALK_SPRITE_FRAME_COUNT 3U

const uint8_t *prince_walk_sprite_frame(uint8_t frame_index);

#endif
