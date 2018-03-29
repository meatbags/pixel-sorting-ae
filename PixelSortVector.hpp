#pragma once
#ifndef PIXEL_SORT_VECTOR_H
#define PIXEL_SORT_VECTOR_H

#define D2FIX(x) (((long)x)<<16)
#define FIX2D(x) (x / ((double)(1L << 16)))
#define PI2 6.28318531
#define PI 3.14159265
#define HALF_PI 1.57079632
#define CLAMP(mn, mx, x) (max(mn, min(mx, x)))
#define BLEND(a, b, x) (a * (1 - x) + b * x)
//#define SQRT2 1.41421356
//#define SQRT2_HALF 0.70710678
//#define SQRT2_MINUS_ONE 0.41421356
//#define SQRT2_HALF_MINUS_ONE 0.20710678

struct Vector {
	double x;
	double y;
	Vector(double x_start, double y_start): x(x_start), y(y_start) {}
	
	void set(double x_set, double y_set) {
		x = x_set;
		y = y_set;
	}

	double getMagnitude() {
		return hypot(x, y);
	}

	double getDistanceTo(Vector *p) {
		return hypot(p->x - x, p->y - y);
	}

	void clamp(double x_max, double y_max) {
		x = CLAMP(0.0, x_max, x);
		y = CLAMP(0.0, y_max, y);
	}
};

Vector projectGrid(double x, double y, Vector normal) {
	// project point onto rotated grid, origin (0, 0)
	double n_dot = normal.x * x + normal.y * y;
	double l_dot = -normal.y * x + normal.x * y;
	double offset_x = normal.x * n_dot;
	double offset_y = normal.y * l_dot;
	double offset_normal = fmod(hypot(offset_x, offset_y), 1.0);
	double normal_scale = ((offset_normal < 0.5) ? -offset_normal : 1.0 - offset_normal) * ((n_dot < 0) ? 1 : -1);
	double offset_line = fmod(hypot(x - offset_x, y - offset_y), 1.0);
	double line_scale = ((offset_line < 0.5) ? -offset_line : 1.0 - offset_line) * ((l_dot < 0) ? 1 : -1);
	
	return Vector(
		x - normal.x * normal_scale + normal.y * line_scale,
		y - normal.y * normal_scale - normal.x * line_scale
	);
}

#endif