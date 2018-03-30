#pragma once
#ifndef PIXEL_SORT_SORTER_H
#define PIXEL_SORT_SORTER_H

#define KEY_BRIGHTNESS 1
#define KEY_RED 2
#define KEY_GREEN 3
#define KEY_BLUE 4
#define ORDER_DESC 1
#define ORDER_ASC 2
#define ORDER_DIP 3
#define ORDER_RISE 4
#define ORDER_PEPPER 5

struct PixelKey8 {
	PF_Pixel8 *pixel;
	double key;
};

PF_Pixel8 *getPixel8(
	PF_EffectWorld *inputP,
	int x,
	int y
) {
	return ((PF_Pixel8 *)((char*)inputP->data + (y * inputP->rowbytes) + x * sizeof(PF_Pixel8)));
}

void copyPixel8(
	PF_Pixel8 *from,
	PF_Pixel8 *to
) {
	if (from == nullptr) {
		to->alpha = PF_MAX_CHAN8;
		to->red = (A_u_char)0;
		to->green = (A_u_char)0;
		to->blue = (A_u_char)0;
	} else {
		to->alpha = from->alpha;
		to->red = from->red;
		to->green = from->green;
		to->blue = from->blue;
	}
}

double getKey8(
	PF_Pixel8 *pixel,
	int key
) {
	if (key == KEY_BRIGHTNESS) {
		return (pixel->red + pixel->blue + pixel->green) / (3.0 * (double)PF_MAX_CHAN8);
	} else if (key == KEY_RED) {
		return pixel->red / (double)PF_MAX_CHAN8;
	} else if (key == KEY_GREEN) {
		return pixel->green / (double)PF_MAX_CHAN8;
	} else if (key == KEY_BLUE) {
		return pixel->blue / (double)PF_MAX_CHAN8;
	}

	// default = brightness
	return (pixel->red + pixel->blue + pixel->green) / (3.0 * (double)PF_MAX_CHAN8);
}

void swapPixels8(
	PixelKey8 *pixels,
	int a,
	int b
) {
	PixelKey8 c = pixels[a];
	pixels[a] = pixels[b];
	pixels[b] = c;
}

void quickSort8(
	PixelKey8 *pixels,
	int p,
	int q
) {
	if (p < q) {
		// get partition
		int pivot = p;
		double compare = pixels[p].key;

		for (int i = pivot + 1; i < q; i++) {
			if (pixels[i].key >= compare) {
				pivot++;
				swapPixels8(pixels, pivot, i);
			}

		}

		swapPixels8(pixels, pivot, p);

		// sort partitions
		quickSort8(pixels, p, pivot);
		quickSort8(pixels, pivot + 1, q);
	}
}

void sortPixels8(
	PixelKey8 *pixels,
	int length,
	int order
) {
	quickSort8(pixels, 0, length);

	// re-order (sorted output is ORDER_DESC)
	if (order == ORDER_ASC) {
		int mid = length / 2;
		for (int i = 0; i < mid; ++i) {
			swapPixels8(pixels, i, length - 1 - i);
		}
	} else if (order == ORDER_DIP || order == ORDER_RISE) {
		if (length > 2) {
			int mid = length / 2;
			for (int i = 1; i < mid; ++i) {
				swapPixels8(pixels, i, i * 2);
			}
			quickSort8(pixels, mid, length);

			// flip first or second half
			int quart = mid / 2;
			if (order == ORDER_DIP) {	
				for (int i = 0; i < quart; ++i) {
					swapPixels8(pixels, i, mid - 1 - i);
				}
			} else {
				for (int i = 0; i < quart; ++i) {
					swapPixels8(pixels, mid + i, length - 1 - i);
				}
			}
		}
	} else if (order == ORDER_PEPPER) {
		if (length > 3) {
			for (int i = 2; i < length - 2; i += 3) {
				swapPixels8(pixels, i, i + 1);
			}
		}
	}
}

bool setKey8(
	PixelKey8 *pixel,
	int key,
	double lower,
	double upper
) {
	// check if pixel passes
	pixel->key = getKey8(pixel->pixel, key);

	return (pixel->key >= lower && pixel->key <= upper);
}

struct Sorter8 {
	Vector vec = Vector(0, 0);
	A_long orig_x;
	A_long orig_y;
	int pixel_index;
	int chunk_length;

	Sorter8(A_long x, A_long y, Vector normal) {
		orig_x = x;
		orig_y = y;
		vec = projectGrid(x + 0.5, y + 0.5, normal);
	}

	void generateChunk8(
		PixelSortInfo *info,
		PixelKey8 *pixels
	) {
		// get array of valid pixels
		int pixel_count = 0;

		for (int i = 0; i < info->length; ++i) {
			if (vec.inBounds(info->ref->width, info->ref->height)) {
				int index = info->length - 1 - i;
				pixels[index].pixel = getPixel8(info->ref, (int)vec.x, (int)vec.y);

				if (setKey8(&pixels[index], info->key, info->threshold_lower, info->threshold_upper)) {
					pixel_count++;
					vec.sub(info->vec);
				}
				else {
					break;
				}
			}
			else {
				break;
			}
		}

		if (pixel_count == 0) {
			// invalid chunk
			chunk_length = 0;
		} else {
			pixel_index = pixel_count - 1;
			int mid = info->length / 2;
			int len = (pixel_count == info->length) ? pixel_count : pixel_count + 1;

			// reset vec, swap existing pixels	
			for (int i = 0; i < len; ++i) {
				vec.add(info->vec);
				if (i <= mid) {
					swapPixels8(pixels, i, info->length - 1 - i);
				}
			}

			// populate remaining chunk
			for (int i = pixel_count; i < info->length; ++i) {
				if (vec.inBounds(info->ref->width, info->ref->height)) {
					pixels[i].pixel = getPixel8(info->ref, (int)vec.x, (int)vec.y);

					if (setKey8(&pixels[i], info->key, info->threshold_lower, info->threshold_upper)) {
						pixel_count++;
						vec.add(info->vec);
					}
				}
				else {
					break;
				}
			}

			chunk_length = pixel_count;
		}
	}

	PF_Err sort(
		PixelSortInfo *info,
		PF_Pixel8 *out,
		PixelKey8 *pixels
	) {
		PF_Err err = PF_Err_NONE;
		generateChunk8(info, pixels);

		if (chunk_length > 1) {
			sortPixels8(pixels, chunk_length, info->order);
			copyPixel8(pixels[pixel_index].pixel, out);
		} else {
			copyPixel8(getPixel8(info->ref, orig_x, orig_y), out);
		}

		return err;
	}
};

#endif