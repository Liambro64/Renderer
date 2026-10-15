#pragma once
#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "../Project.hpp"



class Object
{
public:
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	std::vector<Vector3> vertices;
	std::vector<Triangle> triangles;

	uint32_t color;
	uint32_t outlineColor;

	Rasterizer *rasterizer;

	Object(std::vector<Vector3> vertices, std::vector<Triangle> triangles, Vector3 position = {0, 0, 0}, Quaternion rotation = {1, 0, 0, 0}, Vector3 scale = {1, 1, 1}, uint32_t color = 0x00FF00, uint32_t outlineColor = 0x00000000, Rasterizer *rasterizer = nullptr);
};

#endif // OBJECT_HPP