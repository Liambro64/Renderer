#pragma once
#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP

#include "../Project.hpp"

class Object;

class Rasterizer {
public:
	std::vector<uint32_t> *buffer;
	std::vector<float> *zBuffer;
	Rasterizer(std::vector<uint32_t> &buffer, std::vector<float> &zBuffer);
	Rasterizer();
	~Rasterizer();

	void SetBuffer(std::vector<uint32_t> &buffer, std::vector<float> &zBuffer);
	bool BackfaceCulling(const Vector3 &cameraPosition, const Vector3 &A, const Vector3 &B, const Vector3 &C, const Vector3 &objectPosition);
	Vector3 CameraTranslation(const Vector3 &cameraPosition, const Quaternion &cameraRotation, const Object &object, const Vector3 &point);
	bool ProjectCameraSpacePointTo2D(const Vector3 &cameraSpace, float fovYRadians, const Vector2 &screen, Vector2 &projected);
	float determinant(const Vector2 &a, const Vector2 &b, const Vector2 &p);
	void ChangeWindingOrder(Vector2 &a, Vector2 &b, Vector2 &c, Vector3 &cameraSpaceA, Vector3 &cameraSpaceB, float &area);
	void BoundingBox(const Vector2 &a, const Vector2 &b, const Vector2 &c, int &minX, int &maxX, int &minY, int &maxY);
	void ScreenClamp(int &minX, int &maxX, int &minY, int &maxY, const Vector2 &screen);
	Vector2 Deltas(const Vector2 &a, const Vector2 &b);
	virtual void Rasterize(int minX, int maxX, int minY, int maxY, const Vector3 iz, const Vector2 &projectedA, const Vector2 &projectedB, const Vector2 &projectedC, const Vector2 &screen, const Object &object);
	virtual void Render(const Vector2 &screen, const Vector3 &cameraPosition, const Quaternion &cameraRotation, const Object &object, const Triangle &triangle, float fovYRadians);
};

#endif // RASTERIZER_HPP