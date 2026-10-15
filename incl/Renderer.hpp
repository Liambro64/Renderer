#pragma once
#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "../Project.hpp"

class WindowManager;

class Renderer
{
private:
	std::vector<uint32_t> buffer;
	std::vector<float> zBuffer;
public:
	Vector3 cameraPosition;
	float cameraPitch;
	float cameraYaw;
	Quaternion cameraRotation;
	float fieldOfViewRadians;
	Vector2 screenSize;
	std::vector<Object> objects;
	uint32_t backgroundColor;

	Renderer(Vector3 cameraPosition = {0, 0, 0}, Quaternion cameraRotation = {1, 0, 0, 0}, std::vector<Object> initialObjects = {}, float FOV = 90.0f, Vector2 screenSize = {800, 600}, uint32_t backgroundColor = 0x00888888);
	~Renderer();

	void RotateCamera(const Quaternion& rotation);
	void RotateCamera(float x, float y, float z);
	void RotateCamera(float x, float y);

	void MoveCamera(const Vector3& translation);
	void MoveCameraAbsolute(const Vector3& position);

	void addObject(const Object& object);

	void ScreenResize(Vector2 newSize);
	void ScreenResize(int w, int h);

	const std::vector<uint32_t>& Render();
	const std::vector<uint32_t>& RenderWireframe();
	const std::vector<uint32_t>& RenderWithWireframe(float outlineFraction = 2.0f);
	void drawLine(Vector2 start, Vector2 end, uint32_t color);
	void drawLineWithDepth(Vector2 start, Vector2 end, float z0, float z1, uint32_t color);
};

#endif // RENDERER_HPP