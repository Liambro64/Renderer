#include "../Project.hpp"

FilledWireframeRasterizer::FilledWireframeRasterizer(std::vector<uint32_t> &buffer, std::vector<float> &zBuffer)
	: Rasterizer(buffer, zBuffer)
{
}
FilledWireframeRasterizer::FilledWireframeRasterizer()
	: Rasterizer()
{
}
void FilledWireframeRasterizer::Rasterize(int minX, int maxX, int minY, int maxY, const Vector3 iz, const Vector2 &projectedA, const Vector2 &projectedB, const Vector2 &projectedC, const Vector2 &screen, const Object &object, const Vector3 &inverseLength, const float triangleOutlinePixels)
{

	// compute the edge deltas for the triangle for rasterization
	auto [dx0, dy0] = Deltas(projectedC, projectedB);
	auto [dx1, dy1] = Deltas(projectedA, projectedC);
	auto [dx2, dy2] = Deltas(projectedB, projectedA);
	const Vector2 start = {minX + 0.5f, minY + 0.5f};
	float rowE0 = determinant(projectedB, projectedC, start);
	float rowE1 = determinant(projectedC, projectedA, start);
	float rowE2 = determinant(projectedA, projectedB, start);

	int sw = static_cast<int>(screen.x);
	int sh = static_cast<int>(screen.y);
	for (int y = minY; y <= maxY; ++y)
	{
		float e0 = rowE0, e1 = rowE1, e2 = rowE2;
		uint32_t *colorRow = &(*buffer)[y * sw];
		float *depthRow = &(*zBuffer)[y * sw];
		for (int x = minX; x <= maxX; ++x)
		{
			if (e0 >= 0 && e1 >= 0 && e2 >= 0)
			{
				const float depth = 1.0f / (e0 * iz.x + e1 * iz.y + e2 * iz.z);
				if (depth < depthRow[x])
				{
					const bool isOnEdge =
						(e0 * inverseLength.z <= triangleOutlinePixels) ||
						(e1 * inverseLength.x <= triangleOutlinePixels) ||
						(e2 * inverseLength.y <= triangleOutlinePixels);

					colorRow[x] = isOnEdge ? object.outlineColor : object.color;
					depthRow[x] = depth;
				}
			}
			e0 += dx0;
			e1 += dx1;
			e2 += dx2;
		}
		rowE0 += dy0;
		rowE1 += dy1;
		rowE2 += dy2;
	}
}

void FilledWireframeRasterizer::Render(const Vector2 &screen, const Vector3 &cameraPosition, const Quaternion &cameraRotation, const Object &object, const Triangle &triangle, float fovYRadians)
{
	// Cull back-facing triangles
	if (BackfaceCulling(cameraPosition, object.vertices[triangle.a], object.vertices[triangle.b], object.vertices[triangle.c], object.position))
		return;

	// Transform verticies into camera space
	Vector3 cameraSpaceA = CameraTranslation(cameraPosition, cameraRotation, object, object.vertices[triangle.a]);
	Vector3 cameraSpaceB = CameraTranslation(cameraPosition, cameraRotation, object, object.vertices[triangle.b]);
	Vector3 cameraSpaceC = CameraTranslation(cameraPosition, cameraRotation, object, object.vertices[triangle.c]);

	// after this, you can change the change the math between camera space and projected space
	Vector2 projectedA, projectedB, projectedC;

	// does 2 things: checks if the point is in the view of the camera and projects it to 2D screen space
	if (!ProjectCameraSpacePointTo2D(cameraSpaceA, fovYRadians, screen, projectedA) ||
		!ProjectCameraSpacePointTo2D(cameraSpaceB, fovYRadians, screen, projectedB) ||
		!ProjectCameraSpacePointTo2D(cameraSpaceC, fovYRadians, screen, projectedC))
		return;

	const int sw = static_cast<int>(screen.x);
	const int sh = static_cast<int>(screen.y);

	// find the area of the triangle and change the winding order if necessary
	float area = determinant(projectedA, projectedB, projectedC);
	ChangeWindingOrder(projectedA, projectedB, projectedC, cameraSpaceB, cameraSpaceC, area);

	// computes the bounding box of the triangle and clamps it to the screen.
	int minX, maxX, minY, maxY;
	BoundingBox(projectedA, projectedB, projectedC, minX, maxX, minY, maxY);
	ScreenClamp(minX, maxX, minY, maxY, screen);
	if (area <= 0 || minX > maxX || minY > maxY)
		return;
	float inverseArea = 1.0f / area;


	// compute the inverse depth for each vertex for perspective-correct interpolation
	const float izA = inverseArea / -cameraSpaceA.z;
	const float izB = inverseArea / -cameraSpaceB.z;
	const float izC = inverseArea / -cameraSpaceC.z;

	// compute the starting point for rasterization within the bounding box
	const float invLengthBC = 1.0f / std::hypot(projectedC.x - projectedB.x, projectedC.y - projectedB.y);
	const float invLengthCA = 1.0f / std::hypot(projectedA.x - projectedC.x, projectedA.y - projectedC.y);
	const float invLengthAB = 1.0f / std::hypot(projectedB.x - projectedA.x, projectedB.y - projectedA.y);
	const float size = 0.1f;
	const float minimumTriangleAltitude = std::min(
		{area * size * invLengthBC,
		 area * size * invLengthCA,
		 area * size * invLengthAB});
	const float triangleOutlinePixels =
		0.2f * minimumTriangleAltitude;

	// call the rasterization function
	Rasterize(minX, maxX, minY, maxY, {izA, izB, izC}, projectedA, projectedB, projectedC, screen, object, {invLengthBC, invLengthCA, invLengthAB}, triangleOutlinePixels);
}

FilledWireframeRasterizer::~FilledWireframeRasterizer()
{
}