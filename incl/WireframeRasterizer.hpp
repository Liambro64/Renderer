#pragma once
#ifndef WIREFRAME_RASTERIZER_HPP
#define WIREFRAME_RASTERIZER_HPP

#include "../Project.hpp"

class WireframeRasterizer : public Rasterizer
{
public:
	WireframeRasterizer(std::vector<uint32_t> &buffer, std::vector<float> &zBuffer);
	WireframeRasterizer();
	~WireframeRasterizer();

	void drawLineWithDepth(Vector2 start, Vector2 end, float z0, float z1, uint32_t color, const Vector2 &screen);
	void Rasterize(const Vector2 &projectedA, const Vector2 &projectedB, const Vector2 &projectedC, Vector3 depths, const Object &object, const Vector2 &screen);
	void Render(const Vector2 &screen, const Vector3 &cameraPosition, const Quaternion &cameraRotation, const Object &object, const Triangle &triangle, float fovYRadians) override;
};

#endif // WIREFRAME_RASTERIZER_HPP