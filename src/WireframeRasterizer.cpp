#include "../Project.hpp"

WireframeRasterizer::WireframeRasterizer(std::vector<uint32_t> &buffer, std::vector<float> &zBuffer)
	: Rasterizer(buffer, zBuffer)
{
}

WireframeRasterizer::~WireframeRasterizer()
{
}

void WireframeRasterizer::drawLineWithDepth(Vector2 start, Vector2 end, float z0, float z1, uint32_t color, const Vector2 &screen)
{
	int x0 = static_cast<int>(std::round(start.x));
	int y0 = static_cast<int>(std::round(start.y));
	int x1 = static_cast<int>(std::round(end.x));
	int y1 = static_cast<int>(std::round(end.y));

	int dx = std::abs(x1 - x0);
	int dy = std::abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	float w0 = 1.0f / z0;
	float w1 = 1.0f / z1;

	int steps = (dx > dy) ? dx : dy;

	float current_w = w0;

	while (true)
	{
		float current_z = 1.0f / current_w;
		if (x0 >= 0 && x0 < screen.x && y0 >= 0 && y0 < screen.y)
		{
			if (current_z < (*zBuffer)[y0 * (int)screen.x + x0])
			{
				(*buffer)[y0 * (int)screen.x + x0] = color;
				(*zBuffer)[y0 * (int)screen.x + x0] = current_z;
			}
		}
		if (x0 == x1 && y0 == y1)
			break;
		int e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
		current_w += (w1 - w0) / steps;
	}
}
void WireframeRasterizer::Rasterize(const Vector2 &projectedA, const Vector2 &projectedB, const Vector2 &projectedC, Vector3 depths, const Object &object, const Vector2 &screen)
{
	drawLineWithDepth(projectedA, projectedB, depths.x, depths.y, 0xFFFFFFFF, screen);
	drawLineWithDepth(projectedB, projectedC, depths.y, depths.z, 0xFFFFFFFF, screen);
	drawLineWithDepth(projectedC, projectedA, depths.z, depths.x, 0xFFFFFFFF, screen);
}
void WireframeRasterizer::Render(const Vector2 &screen, const Vector3 &cameraPosition, const Quaternion &cameraRotation, const Object &object, const Triangle &triangle, float fovYRadians)
{
	Vector3 cameraSpaceA = CameraTranslation(cameraPosition, cameraRotation, object, object.vertices[triangle.a]);
	Vector3 cameraSpaceB = CameraTranslation(cameraPosition, cameraRotation, object, object.vertices[triangle.b]);
	Vector3 cameraSpaceC = CameraTranslation(cameraPosition, cameraRotation, object, object.vertices[triangle.c]);

	Vector2 projectedA, projectedB, projectedC;

	if (ProjectCameraSpacePointTo2D(cameraSpaceA, fovYRadians, screen, projectedA) &&
		ProjectCameraSpacePointTo2D(cameraSpaceB, fovYRadians, screen, projectedB) &&
		ProjectCameraSpacePointTo2D(cameraSpaceC, fovYRadians, screen, projectedC))
	{
		drawLineWithDepth(projectedA, projectedB, -cameraSpaceA.z, -cameraSpaceB.z, object.outlineColor, screen);
		drawLineWithDepth(projectedB, projectedC, -cameraSpaceB.z, -cameraSpaceC.z, object.outlineColor, screen);
		drawLineWithDepth(projectedC, projectedA, -cameraSpaceC.z, -cameraSpaceA.z, object.outlineColor, screen);
	}
}