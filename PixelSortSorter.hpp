#pragma once
#ifndef PIXEL_SORT_SORTER_H
#define PIXEL_SORT_SORTER_H

PF_Pixel16 *getPixel16(
	PF_EffectWorld *inputP,
	int x,
	int y
) {
	return ((PF_Pixel16 *)((char*)inputP->data + (y * inputP->rowbytes) + x * sizeof(PF_Pixel16)));
}

PF_Pixel8 *getPixel8(
	PF_EffectWorld *inputP,
	int x,
	int y
) {
	return ((PF_Pixel8 *)((char*)inputP->data + (y * inputP->rowbytes) + x * sizeof(PF_Pixel8)));
}

struct Sorter {
	Vector vec = Vector(0, 0);

	Sorter(double x, double y) {
		vec.x = x;
		vec.y = y;
	}

	PF_Err sample8(
		PixelSortInfo *info
	) {
		PF_Err err = PF_Err_NONE;

		return err;
	}

	PF_Err sort8(
		PF_Pixel8 *outP
	) {
		PF_Err err = PF_Err_NONE;

		return err;
	}
};

/*
struct Sampler {
	Vector normal = Vector(0, 0);
	Vector p1 = Vector(0, 0);
	Vector p2 = Vector(0, 0);
	Vector p3 = Vector(0, 0);
	Vector p4 = Vector(0, 0);
	PF_Pixel8 *sample8_1;
	PF_Pixel8 *sample8_2;
	PF_Pixel8 *sample8_3;
	PF_Pixel8 *sample8_4;
	PF_Pixel16 *sample16_1;
	PF_Pixel16 *sample16_2;
	PF_Pixel16 *sample16_3;
	PF_Pixel16 *sample16_4;

	Sampler(double x, double y, Vector n) {
		p1.x = x;
		p1.y = y;
		normal.x = n.x;
		normal.y = n.y;
	}

	double shapeDistance(
		Vector point,
		Vector dot,
		A_u_char mode
	) {
		double d;

		if (mode == 5) {
			// line
			double dp = (point.x - dot.x) * normal.x + (point.y - dot.y) * normal.y;
			d = hypot(normal.x * dp, normal.y * dp);
		} else if (mode == 4) {
			// ellipse
			double mag = hypot(point.x - dot.x, point.y - dot.y);

			if (mag == 0.0) {
				d = 0.0;
			} else {
				double dp = abs((point.x - dot.x) / mag * normal.x + (point.y - dot.y) / mag * normal.y);
				d = point.getDistanceTo(&dot);
				d = (d * (1 - SQRT2_HALF_MINUS_ONE)) + dp * d * SQRT2_MINUS_ONE;
			}
		} else {
			// all other shapes
			d = point.getDistanceTo(&dot);
		}

		return d;
	}

	double shapeRadius (
		double r,
		Vector point,
		Vector dot,
		A_u_char mode
	) {
		// apply easing, convert radius to shape
		r = pow(r, 2);
		// r = 3 * pow(r, 2) - 2 * pow(r, 3); ?

		if (mode != 1 && mode != 4 && mode != 5) {
			double theta = atan2(dot.y - point.y, dot.x - point.x);

			if (mode == 2) {
				// square
				double st = abs(sin(theta));
				double ct = abs(cos(theta));
				r += (st > ct) ? r - st * r : r - ct * r;
			} else if (mode == 3) {
				// euclidean dot
				double st = abs(sin(theta));
				double ct = abs(cos(theta));
				double square = r + ((st > ct) ? r - st * r : r - ct * r);
				if (r <= 0.5) {
					r = BLEND(r, square, r * 2);
				}
				else {
					double r_max = r + abs(SQRT2_MINUS_ONE * sin(2 * theta) * r);
					r = BLEND(square, r_max, pow((r - 0.5) * 2, 0.4));
				}
			} else if (mode == 6) {
				// diamond (smooth)
				r -= abs(SQRT2_HALF_MINUS_ONE * sin(2 * theta) * r);
			} else {
				// flower
				r *= SQRT2 * abs(sin(2 * theta));
			}
		}

		return r;
	}

	void writeChannel8(
		A_u_char *out,
		A_u_char sample,
		Vector point,
		Vector dot,
		A_u_char mode,
		double radius_max,
		double aa
	) {
		// write to channel if inside dot
		double radius = radius_max * shapeRadius(1.0 - sample / 255.0, point, dot, mode);
		double distance = shapeDistance(point, dot, mode);

		if (distance < radius) {
			*out = (A_u_char)max(0.0, (*out - (CLAMP(0.0, 1.0, (radius - distance) / aa) * 255)));
		}
	}

	void writeChannel16(
		A_u_short *out,
		A_u_short sample,
		Vector point,
		Vector dot,
		A_u_char mode,
		double radius_max,
		double aa
	) {
		// write to channel if inside dot
		double radius = radius_max * shapeRadius(1.0 - sample / (double)PF_MAX_CHAN16, point, dot, mode);
		double distance = shapeDistance(point, dot, mode);

		if (distance < radius) {
			*out = (A_u_short)max(0.0, (*out - (CLAMP(0.0, 1.0, (radius - distance) / aa) * PF_MAX_CHAN16)));
		}
	}

	PF_Err write8(
		int channel,
		A_u_char *ch,
		Vector point,
		A_u_char mode,
		double radius,
		double aa
	) {
		PF_Err err = PF_Err_NONE;

		// write target channel
		if (channel == 1) {
			writeChannel8(ch, sample8_1->red, point, p1, mode, radius, aa);
			writeChannel8(ch, sample8_2->red, point, p2, mode, radius, aa);
			writeChannel8(ch, sample8_3->red, point, p3, mode, radius, aa);
			writeChannel8(ch, sample8_4->red, point, p4, mode, radius, aa);
		} else if (channel == 2) {
			writeChannel8(ch, sample8_1->green, point, p1, mode, radius, aa);
			writeChannel8(ch, sample8_2->green, point, p2, mode, radius, aa);
			writeChannel8(ch, sample8_3->green, point, p3, mode, radius, aa);
			writeChannel8(ch, sample8_4->green, point, p4, mode, radius, aa);
		} else {
			writeChannel8(ch, sample8_1->blue, point, p1, mode, radius, aa);
			writeChannel8(ch, sample8_2->blue, point, p2, mode, radius, aa);
			writeChannel8(ch, sample8_3->blue, point, p3, mode, radius, aa);
			writeChannel8(ch, sample8_4->blue, point, p4, mode, radius, aa);
		}

		return err;
	}

	PF_Err write16(
		int channel,
		A_u_short *ch,
		Vector point,
		A_u_char mode,
		double radius,
		double aa
	) {
		PF_Err err = PF_Err_NONE;

		// write target channel
		if (channel == 1) {
			writeChannel16(ch, sample16_1->red, point, p1, mode, radius, aa);
			writeChannel16(ch, sample16_2->red, point, p2, mode, radius, aa);
			writeChannel16(ch, sample16_3->red, point, p3, mode, radius, aa);
			writeChannel16(ch, sample16_4->red, point, p4, mode, radius, aa);
		}
		else if (channel == 2) {
			writeChannel16(ch, sample16_1->green, point, p1, mode, radius, aa);
			writeChannel16(ch, sample16_2->green, point, p2, mode, radius, aa);
			writeChannel16(ch, sample16_3->green, point, p3, mode, radius, aa);
			writeChannel16(ch, sample16_4->green, point, p4, mode, radius, aa);
		}
		else {
			writeChannel16(ch, sample16_1->blue, point, p1, mode, radius, aa);
			writeChannel16(ch, sample16_2->blue, point, p2, mode, radius, aa);
			writeChannel16(ch, sample16_3->blue, point, p3, mode, radius, aa);
			writeChannel16(ch, sample16_4->blue, point, p4, mode, radius, aa);
		}

		return err;
	}

	PF_Err sample8(
		PixelSortInfo *info
	) {
		PF_Err err = PF_Err_NONE;
		double width = info->samp_pb.src->width - 1;
		double height = info->samp_pb.src->height - 1;
		
		// find nearest sample points
		Vector s1 = getGridPoint(p1.x, p1.y, info->origin, info->normal_0, info->grid_step, info->grid_half_step);
		s1.clamp(width, height);
		sample8_1 = getPixel8(info->ref, (int)s1.x, (int)s1.y);
		Vector s2 = getGridPoint(p2.x, p2.y, info->origin, info->normal_0, info->grid_step, info->grid_half_step);
		s2.clamp(width, height);
		sample8_2 = getPixel8(info->ref, (int)s2.x, (int)s2.y);
		Vector s3 = getGridPoint(p3.x, p3.y, info->origin, info->normal_0, info->grid_step, info->grid_half_step);
		s3.clamp(width, height);
		sample8_3 = getPixel8(info->ref, (int)s3.x, (int)s3.y);
		Vector s4 = getGridPoint(p4.x, p4.y, info->origin, info->normal_0, info->grid_step, info->grid_half_step);
		s4.clamp(width, height);
		sample8_4 = getPixel8(info->ref, (int)s4.x, (int)s4.y);

		return err;
	}

	PF_Err sample16(
		PixelSortInfo *info
	) {
		PF_Err err = PF_Err_NONE;
		double width = info->samp_pb.src->width - 1;
		double height = info->samp_pb.src->height - 1;

		// find nearest sample points
		Vector s1 = getGridPoint(p1.x, p1.y, info->origin, info->normal_0, info->grid_step, info->grid_half_step);
		s1.clamp(width, height);
		sample16_1 = getPixel16(info->ref, (int)s1.x, (int)s1.y);
		Vector s2 = getGridPoint(p2.x, p2.y, info->origin, info->normal_0, info->grid_step, info->grid_half_step);
		s2.clamp(width, height);
		sample16_2 = getPixel16(info->ref, (int)s2.x, (int)s2.y);
		Vector s3 = getGridPoint(p3.x, p3.y, info->origin, info->normal_0, info->grid_step, info->grid_half_step);
		s3.clamp(width, height);
		sample16_3 = getPixel16(info->ref, (int)s3.x, (int)s3.y);
		Vector s4 = getGridPoint(p4.x, p4.y, info->origin, info->normal_0, info->grid_step, info->grid_half_step);
		s4.clamp(width, height);
		sample16_4 = getPixel16(info->ref, (int)s4.x, (int)s4.y);

		return err;
	}
};

Sampler getSampler(double x, double y, Vector origin, Vector normal, double step, double half_step) {
	// project point onto rotated grid, get containing cell
	double dot_normal = normal.x * (x - origin.x) + normal.y * (y - origin.y);
	double dot_line = -normal.y * (x - origin.x) + normal.x * (y - origin.y);
	double offset_x = normal.x * dot_normal;
	double offset_y = normal.y * dot_normal;
	double offset_normal = fmod(hypot(offset_x, offset_y), step);
	double normal_dir = (dot_normal < 0) ? 1 : -1;
	double normal_scale = ((offset_normal < half_step) ? -offset_normal : step - offset_normal) * normal_dir;
	double offset_line = fmod(hypot((x - offset_x) - origin.x, (y - offset_y) - origin.y), step);
	double line_dir = (dot_line < 0) ? 1 : -1;
	double line_scale = ((offset_line < half_step) ? -offset_line : step - offset_line) * line_dir;

	// set first point to nearest gridpoint
	Sampler res = Sampler(
		x - normal.x * normal_scale + normal.y * line_scale,
		y - normal.y * normal_scale - normal.x * line_scale,
		normal
	);

	// calculate cell corners
	double normal_step = normal_dir * ((offset_normal < half_step) ? step : -step);
	double line_step = line_dir * ((offset_line < half_step) ? step : -step);
	res.p2.x = res.p1.x - normal.x * normal_step;
	res.p2.y = res.p1.y - normal.y * normal_step;
	res.p3.x = res.p1.x + normal.y * line_step;
	res.p3.y = res.p1.y - normal.x * line_step;
	res.p4.x = res.p1.x - normal.x * normal_step + normal.y * line_step;
	res.p4.y = res.p1.y - normal.y * normal_step - normal.x * line_step;

	return res;
}
*/

#endif