#pragma once

#include "Library/Image/Image.h"

/**
 * Ours: EPX / Scale2x pixel-art scaling of an indexed image. Works on palette indices, so the result still goes
 * through the palette shader (lighting, colour variants). Edges between differently coloured runs get rounded,
 * flat areas stay flat.
 *
 * @param image                         Indexed image.
 * @return                              The image twice as big.
 */
GrayscaleImage scale2x(GrayscaleImageView image);

/** Scale3x, the 3x variant of the same algorithm. */
GrayscaleImage scale3x(GrayscaleImageView image);
