#pragma once
#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <vector>
#include <cstdint>
#include <unistd.h>
#include <thread>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>

struct Vector3
{
	float x;
	float y;
	float z;
	static Vector3 cross(const Vector3 &a, const Vector3 &b)
	{
		return {
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x};
	}

	Vector3 operator+(const Vector3 &other) const
	{
		return {x + other.x, y + other.y, z + other.z};
	}
	Vector3 operator-(const Vector3 &other) const
	{
		return {x - other.x, y - other.y, z - other.z};
	}
	Vector3 operator*(const Vector3 &other) const
	{
		return {x * other.x, y * other.y, z * other.z};
	}
	Vector3 operator*(float scalar) const
	{
		return {x * scalar, y * scalar, z * scalar};
	}
	Vector3 operator+=(const Vector3 &other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	float distance(const Vector3 &other) const
	{
		return std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y) + (z - other.z) * (z - other.z));
	}
	float dot(const Vector3 &other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}
	Vector3 cross(const Vector3 &other) const
	{
		return {
			y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x};
	}
};

struct Vector2
{
	float x;
	float y;
};

struct Quaternion
{
	float w;
	float x;
	float y;
	float z;
	Vector3 toVector3() const
	{
		return {x, y, z};
	}
	static Quaternion eulerToQuaternion(float x, float y, float z)
	{
		float c1 = std::cos(y / 2);
		float s1 = std::sin(y / 2);
		float c2 = std::cos(z / 2);
		float s2 = std::sin(z / 2);
		float c3 = std::cos(x / 2);
		float s3 = std::sin(x / 2);

		Quaternion q;
		q.w = c1 * c2 * c3 - s1 * s2 * s3;
		q.x = c1 * c2 * s3 + s1 * s2 * c3;
		q.y = s1 * c2 * c3 + c1 * s2 * s3;
		q.z = c1 * s2 * c3 - s1 * c2 * s3;
		return q;
	}
	Quaternion conjugate() const
	{
		return {w, -x, -y, -z};
	}
	Vector3 operator*(const Vector3 &v) const
	{
		Vector3 u = this->toVector3();

		// t = 2 * cross(u, v)
		Vector3 t = Vector3::cross(u, v);
		t.x *= 2.0f;
		t.y *= 2.0f;
		t.z *= 2.0f;

		// v' = v + w * t + cross(u, t)
		Vector3 cross_u_t = Vector3::cross(u, t);

		return
		{
			v.x + this->w *t.x + cross_u_t.x,
				v.y + this->w *t.y + cross_u_t.y,
				v.z + this->w *t.z + cross_u_t.z
		};
	}
	Quaternion operator*(const Quaternion &q) const
	{
		return {
			w * q.w - x * q.x - y * q.y - z * q.z,
			w * q.x + x * q.w + y * q.z - z * q.y,
			w * q.y - x * q.z + y * q.w + z * q.x,
			w * q.z + x * q.y - y * q.x + z * q.w};
	}
	Quaternion &operator*=(const Quaternion &q)
	{
		Quaternion result = {
			w * q.w - x * q.x - y * q.y - z * q.z,
			w * q.x + x * q.w + y * q.z - z * q.y,
			w * q.y - x * q.z + y * q.w + z * q.x,
			w * q.z + x * q.y - y * q.x + z * q.w};
		w = result.w;
		x = result.x;
		y = result.y;
		z = result.z;
		return *this;
	}
	void normalize()
	{
		const float length = std::sqrt(w * w + x * x + y * y + z * z);
		if (length > 0.0f)
		{
			w /= length;
			x /= length;
			y /= length;
			z /= length;
		}
	}
};

struct Triangle
{
	int a;
	int b;
	int c;
};

#include "incl/Rasterizer.hpp"
#include "incl/WireframeRasterizer.hpp"
#include "incl/FilledWireframeRasterizer.hpp"
#include "incl/Object.hpp"
#include "incl/Renderer.hpp"
#include "incl/WindowManager.hpp"

#endif // PROJECT_HPP