#pragma once
#ifndef PIXEL_SORT_SORTER_H
#define PIXEL_SORT_SORTER_H

#define KEY_LIGHTNESS 1
#define KEY_DARKNESS 2

PF_Pixel8 *getPixel8(
	PF_EffectWorld *inputP,
	int x,
	int y
) {
	return ((PF_Pixel8 *)((char*)inputP->data + (y * inputP->rowbytes) + x * sizeof(PF_Pixel8)));
}

struct PixelIndex8 {
	PF_Pixel8 *pixel;
	int index;
};

struct Sorter8 {
	Vector vec = Vector(0, 0);

	Sorter8(double x, double y, Vector normal) {
		vec = projectGrid(x, y, normal);
	}

	bool valid(
		PF_Pixel8 *pixel,
		int key,
		double threshold
	) {
		// check if pixel passes
		double value;

		if (key == KEY_LIGHTNESS) {
			value = (pixel->red + pixel->blue + pixel->green) / (3.0 * (double)PF_MAX_CHAN8);
		} else {
			value = 1.0 - (pixel->red + pixel->blue + pixel->green) / (3.0 * (double)PF_MAX_CHAN8);
		}
		
		return (value >= threshold);
	}

	int getPixels(
		PixelSortInfo *info,
		PixelIndex8 *pixels
	) {
		// get array of valid pixels
		int valid_count = 0;

		for (int i = 0; i < info->length; ++i) {
			if (!vec.inBounds(info->ref->width, info->ref->height) ||
				!valid(getPixel8(info->ref, (int)vec.x, (int)vec.y), info->key, info->threshold)) {
				break;
			}
			vec.sub(info->vec);
			valid_count++;
		}

		// invalid chunk
		if (!valid_count) {
			return valid_count;
		}

		// populate pixel array
		for (int i = 0; i < info->length; ++i) {
			if (i < valid_count) {
				pixels[i].pixel = getPixel8(info->ref, (int)vec.x, (int)vec.y);
			} else {
				if (vec.inBounds(info->ref->width, info->ref->height)) {
					pixels[i].pixel = getPixel8(info->ref, (int)vec.x, (int)vec.y);
					if (valid(pixels[i].pixel, info->key, info->threshold)) {
						valid_count++;
						vec.add(info->vec);
					} else {
						break;
					}
				} else {
					break;
				}
			}
		}

		return valid_count;
	}

	PF_Err sort(
		PixelSortInfo *info,
		PF_Pixel8 *outP,
		PixelIndex8 *pixels
	) {
		PF_Err err = PF_Err_NONE;
		int len = getPixels(info, pixels);



		return err;
	}
};

/*
PF_Pixel16 *getPixel16(
	PF_EffectWorld *inputP,
	int x,
	int y
) {
	return ((PF_Pixel16 *)((char*)inputP->data + (y * inputP->rowbytes) + x * sizeof(PF_Pixel16)));
}
*/

#endif