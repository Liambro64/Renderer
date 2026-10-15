#include "../Project.hpp"

Object::Object(std::vector<Vector3> vertices, std::vector<Triangle> triangles, Vector3 position, Quaternion rotation, Vector3 scale, uint32_t color, uint32_t outlineColor, Rasterizer *rasterizer)
	: vertices(vertices), triangles(triangles), position(position), rotation(rotation), scale(scale), color(color), outlineColor(outlineColor), rasterizer(rasterizer)
{
	
}