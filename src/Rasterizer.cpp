#include "../Project.hpp"

Rasterizer::Rasterizer(std::vector<uint32_t> &buffer, std::vector<float> &zBuffer)
	: buffer(&buffer), zBuffer(&zBuffer)
{
}
Rasterizer::Rasterizer()
{
}

void Rasterizer::SetBuffer(std::vector<uint32_t> &buffer, std::vector<float> &zBuffer)
{
	this->buffer = &buffer;
	this->zBuffer = &zBuffer;
}

bool Rasterizer::BackfaceCulling(const Vector3 &cameraPosition, const Vector3 &A, const Vector3 &B, const Vector3 &C, const Vector3 &objectPosition)
{
	Vector3 edge1 = B - A;
	Vector3 edge2 = C - A;
	Vector3 faceNormal = Vector3::cross(edge2, edge1);
	return faceNormal.dot(cameraPosition - (A + objectPosition)) >= 0;
}

Vector3 Rasterizer::CameraTranslation(const Vector3 &cameraPosition, const Quaternion &cameraRotation, const Object &object,const Vector3 &point)
{
	return cameraRotation * (object.rotation * (point * object.scale) + object.position - cameraPosition);
}

bool Rasterizer::ProjectCameraSpacePointTo2D(const Vector3 &cameraSpace, float fovYRadians, const Vector2 &screen, Vector2 &projected)
{
	if (cameraSpace.z >= 0.0f)
		return false;
	float aspectRatio = screen.x / screen.y;
	float fLenY = 1.0f / tan(fovYRadians / 2.0f);
	float fLenX = fLenY / aspectRatio;
	float depth = -cameraSpace.z;

	float ndcX = (cameraSpace.x * fLenX) / depth;
	float ndcY = (cameraSpace.y * fLenY) / depth;

	projected.x = (ndcX + 1.0f) * 0.5f * screen.x;
	projected.y = (1.0f - ndcY) * 0.5f * screen.y;
	return true;
}

float Rasterizer::determinant(const Vector2 &a, const Vector2 &b, const Vector2 &p)
{
	return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

void Rasterizer::ChangeWindingOrder(Vector2 &a, Vector2 &b, Vector2 &c, Vector3 &cameraSpaceB, Vector3 &cameraSpaceC, float &area)
{
	if (area < 0)
	{
		std::swap(b, c);
		std::swap(cameraSpaceB, cameraSpaceC);
		area = -area;
	}
}
void Rasterizer::BoundingBox(const Vector2 &a, const Vector2 &b, const Vector2 &c, int &minX, int &maxX, int &minY, int &maxY)
{
	minX = static_cast<int>(std::floor(std::min({a.x, b.x, c.x})));
	maxX = static_cast<int>(std::ceil(std::max({a.x, b.x, c.x})));
	minY = static_cast<int>(std::floor(std::min({a.y, b.y, c.y})));
	maxY = static_cast<int>(std::ceil(std::max({a.y, b.y, c.y})));
}
void Rasterizer::ScreenClamp(int &minX, int &maxX, int &minY, int &maxY, const Vector2 &screen)
{
	minX = std::max(0, minX);
	maxX = std::min(static_cast<int>(screen.x) - 1, maxX);
	minY = std::max(0, minY);
	maxY = std::min(static_cast<int>(screen.y) - 1, maxY);
}

Vector2 Rasterizer::Deltas(const Vector2 &a, const Vector2 &b)
{
	return Vector2{-(a.y - b.y), (a.x - b.x)};
}

void Rasterizer::Rasterize(int minX, int maxX, int minY, int maxY, const Vector3 iz, const Vector2 &projectedA, const Vector2 &projectedB, const Vector2 &projectedC, const Vector2 &screen, const Object &object)
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
	// loop over y pixels within the bounding box
	for (int y = minY; y <= maxY; ++y)
	{
		float e0 = rowE0, e1 = rowE1, e2 = rowE2;
		uint32_t *colorRow = &(*buffer)[y * sw];
		float *depthRow = &(*zBuffer)[y * sw];
		// loop over x pixels within the bounding box
		for (int x = minX; x <= maxX; ++x)
		{
			if (e0 >= 0 && e1 >= 0 && e2 >= 0)
			{
				float depth = 1.0f / (e0 * iz.x + e1 * iz.y + e2 * iz.z);
				// here, you can control the color and depth of the pixel
				if (depth < depthRow[x])
				{
					colorRow[x] = object.color;
					depthRow[x] = depth;
				}
			}
			// step edge functions to the next pixel in the row
			e0 += dx0;
			e1 += dx1;
			e2 += dx2;
		}
		// step edge functions to the next row
		rowE0 += dy0;
		rowE1 += dy1;
		rowE2 += dy2;
	}
}

void Rasterizer::Render(const Vector2 &screen, const Vector3 &cameraPosition, const Quaternion &cameraRotation, const Object &object, const Triangle &triangle, float fovYRadians)
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

	// call the rasterization function
	Rasterize(minX, maxX, minY, maxY, {izA, izB, izC}, projectedA, projectedB, projectedC, screen, object);
}

Rasterizer::~Rasterizer()
{
}