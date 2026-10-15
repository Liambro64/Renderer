#pragma once
#ifndef FILLED_WIREFRAME_RASTERIZER_HPP
#define FILLED_WIREFRAME_RASTERIZER_HPP

#include "../Project.hpp"

class FilledWireframeRasterizer : public Rasterizer
{
public:
	FilledWireframeRasterizer(std::vector<uint32_t> &buffer, std::vector<float> &zBuffer);
	FilledWireframeRasterizer();
	~FilledWireframeRasterizer();

	void Rasterize(int minX, int maxX, int minY, int maxY, const Vector3 iz, const Vector2 &projectedA, const Vector2 &projectedB, const Vector2 &projectedC, const Vector2 &screen, const Object &object, const Vector3 &inverseLength, const float triangleOutlinePixels);
	void Render(const Vector2 &screen, const Vector3 &cameraPosition, const Quaternion &cameraRotation, const Object &object, const Triangle &triangle, float fovYRadians) override;
};

#endif // FILLED_WIREFRAME_RASTERIZER_HPP