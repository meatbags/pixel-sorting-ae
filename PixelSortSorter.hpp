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

void copyPixel8(
	PF_Pixel8 *in,
	PF_Pixel8 *out
) {
	out->alpha = in->alpha;
	out->red = in->red;
	out->green = in->green;
	out->blue = in->blue;
}

struct PixelKey8 {
	PF_Pixel8 *pixel;
	double key;
};

struct Sorter8 {
	Vector vec = Vector(0, 0);
	int pixel_index;
	int length;

	Sorter8(double x, double y, Vector normal) {
		vec = projectGrid(x + 0.5, y + 0.5, normal);
	}

	double getKey(
		PF_Pixel8 *pixel,
		int key
	) {
		// get comparison key
		if (key == KEY_LIGHTNESS) {
			return (pixel->red + pixel->blue + pixel->green) / (3.0 * (double)PF_MAX_CHAN8);
		} else if (key == KEY_DARKNESS) {
			return 1.0 - (pixel->red + pixel->blue + pixel->green) / (3.0 * (double)PF_MAX_CHAN8);
		}
		
		// lightness
		return (pixel->red + pixel->blue + pixel->green) / (3.0 * (double)PF_MAX_CHAN8);
	}

	bool valid(
		PF_Pixel8 *pixel,
		int key,
		double threshold
	) {
		// check if pixel passes
		return (getKey(pixel, key) >= threshold);
	}

	int getPixels(
		PixelSortInfo *info,
		PixelKey8 *pixels
	) {
		// get array of valid pixels
		int pixel_count = 0;

		for (int i = 0; i < info->length; ++i) {
			if (!vec.inBounds(info->ref->width, info->ref->height) ||
				!valid(getPixel8(info->ref, (int)vec.x, (int)vec.y), info->key, info->threshold)) {
				break;
			}
			vec.sub(info->vec);
			pixel_count++;
		}

		// invalid chunk
		if (!pixel_count) {
			return pixel_count;
		}

		pixel_index = pixel_count - 1;

		// populate pixel array, generate keys
		for (int i = 0; i < info->length; ++i) {
			if (i <= pixel_index) {
				pixels[i].pixel = getPixel8(info->ref, (int)vec.x, (int)vec.y);
				pixels[i].key = getKey(pixels[i].pixel, info->key);
			} else {
				if (vec.inBounds(info->ref->width, info->ref->height)) {
					pixels[i].pixel = getPixel8(info->ref, (int)vec.x, (int)vec.y);
					pixels[i].key = getKey(pixels[i].pixel, info->key);

					if (valid(pixels[i].pixel, info->key, info->threshold)) {
						pixel_count++;
						vec.add(info->vec);
					} else {
						break;
					}
				} else {
					break;
				}
			}
		}

		return pixel_count;
	}

	void sortPixels(
		PixelKey8 *pixels,
		int order
	) {

	}

	PF_Err sort(
		PixelSortInfo *info,
		PF_Pixel8 *out,
		PixelKey8 *pixels
	) {
		PF_Err err = PF_Err_NONE;
		length = getPixels(info, pixels);

		if (length) {
			sortPixels(pixels, info->order);
			copyPixel8(pixels[pixel_index].pixel, out);
		} else {
			copyPixel8(getPixel8(info->ref, (int)vec.x, (int)vec.y), out);
		}

		return err;
	}
};

#endif