#include "../Project.hpp"

bool project3DPointTo2D(const Vector3 &worldPoint,
                        const Vector3 &camPos,
                        const Quaternion &camRot,
                        float fovYRadians,
                        const Vector2 &screen,
                        Vector2 &out2D)
{
    Vector3 translated = worldPoint - camPos;

    Vector3 cameraSpace = camRot * translated;

    if (cameraSpace.z >= 0.0f)
    {
        return false;
    }

    float aspectRatio = screen.x / screen.y;
    float fLenY = 1.0f / std::tan(fovYRadians * 0.5f);
    float fLenX = fLenY / aspectRatio;

    float clipZ = -cameraSpace.z;
    float ndcX = (cameraSpace.x * fLenX) / clipZ;
    float ndcY = (cameraSpace.y * fLenY) / clipZ;

    out2D.x = (ndcX + 1.0f) * 0.5f * screen.x;
    out2D.y = (1.0f - ndcY) * 0.5f * screen.y;

    return true;
}

bool projectCameraSpacePointTo2D(
    const Vector3 &cameraSpace,
    float fovYRadians,
    const Vector2 &screen,
    Vector2 &out2D)
{
    if (cameraSpace.z >= 0.0f)
    {
        return false;
    }

    const float aspectRatio = screen.x / screen.y;
    const float fLenY = 1.0f / std::tan(fovYRadians * 0.5f);
    const float fLenX = fLenY / aspectRatio;
    const float depth = -cameraSpace.z;

    const float ndcX = (cameraSpace.x * fLenX) / depth;
    const float ndcY = (cameraSpace.y * fLenY) / depth;

    out2D.x = (ndcX + 1.0f) * 0.5f * screen.x;
    out2D.y = (1.0f - ndcY) * 0.5f * screen.y;
    return true;
}

Renderer::Renderer(Vector3 cameraPosition, Quaternion cameraRotation, std::vector<Object> initialObjects, float FOV, Vector2 screenSize, uint32_t backgroundColor)
    : cameraPosition(cameraPosition),
      cameraRotation(cameraRotation),
      fieldOfViewRadians(FOV * 3.14159265f / 180.0f), // Convert FOV from degrees to radians
      screenSize(screenSize),
      objects(initialObjects),
      backgroundColor(backgroundColor),
      buffer(screenSize.x * screenSize.y, backgroundColor),
      zBuffer(screenSize.x * screenSize.y, std::numeric_limits<float>::infinity()),
      baseRasterizer(buffer, zBuffer)
{
    cameraPitch = 0.0f;
    cameraYaw = 0.0f;
}

Renderer::~Renderer()
{
}

void Renderer::RotateCamera(const Quaternion &rotation)
{
    cameraRotation *= rotation;
    cameraRotation.normalize();
}

void Renderer::RotateCamera(float x, float y, float z)
{
    cameraRotation *= Quaternion::eulerToQuaternion(x, y, z);
    cameraRotation.normalize();
}
void Renderer::RotateCamera(float x, float y)
{
    cameraPitch += x;
    cameraYaw += y;

    Quaternion pitchRotation = Quaternion::eulerToQuaternion(cameraPitch, 0.0f, 0.0f);
    Quaternion yawRotation = Quaternion::eulerToQuaternion(0.0f, cameraYaw, 0.0f);

    cameraRotation = pitchRotation * yawRotation;
    cameraRotation.normalize();
}

void Renderer::MoveCamera(const Vector3 &translation)
{
    cameraPosition += cameraRotation.conjugate() * translation;
}

void Renderer::MoveCameraAbsolute(const Vector3 &position)
{
    cameraPosition += position;
}

void Renderer::addObject(const Object &object)
{
    objects.push_back(object);
}

Vector3 interpolate(const Vector3 &a, const Vector3 &b, float t)
{
    return a * (1.0f - t) + b * t;
}

float determinant(const Vector2 &a, const Vector2 &b, const Vector2 &p)
{
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

uint64_t edgeKey(int firstVertex, int secondVertex)
{
    const uint32_t low = static_cast<uint32_t>(std::min(firstVertex, secondVertex));
    const uint32_t high = static_cast<uint32_t>(std::max(firstVertex, secondVertex));
    return (static_cast<uint64_t>(low) << 32) | high;
}
const std::vector<uint32_t> &Renderer::Render()
{
    std::fill(buffer.begin(), buffer.end(), backgroundColor);
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::infinity());
    Rasterizer baseRasterizer(buffer, zBuffer);

    for (const auto &object : objects)
    {
        Rasterizer *rasterizer = object.rasterizer;
        if (!rasterizer)
            rasterizer = &baseRasterizer;
        else
            rasterizer->SetBuffer(buffer, zBuffer);
        for (const auto &triangle : object.triangles)
        {
            rasterizer->Render(screenSize, cameraPosition, cameraRotation, object, triangle, fieldOfViewRadians);
        }
    }

    return buffer;
}

const std::vector<uint32_t> &Renderer::OldRender()
{
    std::fill(buffer.begin(), buffer.end(), backgroundColor);
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::infinity());

    for (const auto &object : objects)
    {
        for (const auto &triangle : object.triangles)
        {
            
            Vector3 edge1 = object.vertices[triangle.b] - object.vertices[triangle.a];
            Vector3 edge2 = object.vertices[triangle.c] - object.vertices[triangle.a];
            Vector3 faceNormal = Vector3::cross(edge2, edge1);
            if (faceNormal.dot(cameraPosition - (object.vertices[triangle.a] + object.position)) >= 0)
                continue;

            Vector3 cameraSpaceA = cameraRotation * (object.rotation * (object.vertices[triangle.a] * object.scale) + object.position - cameraPosition);
            Vector3 cameraSpaceB = cameraRotation * (object.rotation * (object.vertices[triangle.b] * object.scale) + object.position - cameraPosition);
            Vector3 cameraSpaceC = cameraRotation * (object.rotation * (object.vertices[triangle.c] * object.scale) + object.position - cameraPosition);

            Vector2 projectedA, projectedB, projectedC;

            if (projectCameraSpacePointTo2D(cameraSpaceA, fieldOfViewRadians, screenSize, projectedA) &&
                projectCameraSpacePointTo2D(cameraSpaceB, fieldOfViewRadians, screenSize, projectedB) &&
                projectCameraSpacePointTo2D(cameraSpaceC, fieldOfViewRadians, screenSize, projectedC))
            {
                float triangleArea = determinant(projectedA, projectedB, projectedC);
                // Rasterize the triangle into the buffer using z-buffering
                if (triangleArea < 0)
                {
                    std::swap(projectedB, projectedC);
                    std::swap(cameraSpaceB, cameraSpaceC);
                    triangleArea = -triangleArea;
                }
                // bounding box for the triangle
                int minX = static_cast<int>(std::floor(std::min({projectedA.x, projectedB.x, projectedC.x})));
                int maxX = static_cast<int>(std::ceil(std::max({projectedA.x, projectedB.x, projectedC.x})));
                int minY = static_cast<int>(std::floor(std::min({projectedA.y, projectedB.y, projectedC.y})));
                int maxY = static_cast<int>(std::ceil(std::max({projectedA.y, projectedB.y, projectedC.y})));

                // bounding box clamping to screen dimensions
                const int sw = static_cast<int>(screenSize.x);
                const int sh = static_cast<int>(screenSize.y);
                minX = std::max(minX, 0);
                minY = std::max(minY, 0);
                maxX = std::min(maxX, sw - 1);
                maxY = std::min(maxY, sh - 1);
                if (triangleArea <= 0.0f || minX > maxX || minY > maxY)
                    continue;

                // Edge functions are linear in x/y, so step them incrementally
                const float invArea = 1.0f / triangleArea;
                const float dx0 = -(projectedC.y - projectedB.y), dy0 = projectedC.x - projectedB.x;
                const float dx1 = -(projectedA.y - projectedC.y), dy1 = projectedA.x - projectedC.x;
                const float dx2 = -(projectedB.y - projectedA.y), dy2 = projectedB.x - projectedA.x;
                const float izA = invArea / -cameraSpaceA.z;
                const float izB = invArea / -cameraSpaceB.z;
                const float izC = invArea / -cameraSpaceC.z;

                const Vector2 start = {minX + 0.5f, minY + 0.5f};
                float rowE0 = determinant(projectedB, projectedC, start);
                float rowE1 = determinant(projectedC, projectedA, start);
                float rowE2 = determinant(projectedA, projectedB, start);

                for (int y = minY; y <= maxY; ++y)
                {
                    float e0 = rowE0, e1 = rowE1, e2 = rowE2;
                    uint32_t *colorRow = &buffer[y * sw];
                    float *depthRow = &zBuffer[y * sw];
                    for (int x = minX; x <= maxX; ++x)
                    {
                        if (e0 >= 0 && e1 >= 0 && e2 >= 0)
                        {
                            float depth = 1.0f / (e0 * izA + e1 * izB + e2 * izC);
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
        }
    }

    return buffer;
}
void Renderer::drawLine(Vector2 start, Vector2 end, uint32_t color)
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

    while (true)
    {
        if (x0 >= 0 && x0 < screenSize.x && y0 >= 0 && y0 < screenSize.y)
        {
            buffer[y0 * screenSize.x + x0] = color;
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
    }
}

void Renderer::drawLineWithDepth(Vector2 start, Vector2 end, float z0, float z1, uint32_t color)
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
        if (x0 >= 0 && x0 < screenSize.x && y0 >= 0 && y0 < screenSize.y)
        {
            if (current_z < zBuffer[y0 * screenSize.x + x0])
            {
                buffer[y0 * screenSize.x + x0] = color;
                zBuffer[y0 * screenSize.x + x0] = current_z;
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
void Renderer::ScreenResize(Vector2 newSize)
{
    screenSize = newSize;
    buffer.resize(screenSize.x * screenSize.y, backgroundColor);
    zBuffer.resize(screenSize.x * screenSize.y, std::numeric_limits<float>::infinity());
}
void Renderer::ScreenResize(int w, int h)
{
    screenSize = Vector2{w, h};
    buffer.resize(w * h, backgroundColor);
    zBuffer.resize(w * h, std::numeric_limits<float>::infinity());
}

const std::vector<uint32_t> &Renderer::RenderWireframe()
{
    std::fill(buffer.begin(), buffer.end(), backgroundColor);
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::infinity());

    for (const auto &object : objects)
    {
        for (const auto &triangle : object.triangles)
        {
            Vector3 cameraSpaceA = cameraRotation * (object.rotation * (object.vertices[triangle.a] * object.scale) + object.position - cameraPosition);
            Vector3 cameraSpaceB = cameraRotation * (object.rotation * (object.vertices[triangle.b] * object.scale) + object.position - cameraPosition);
            Vector3 cameraSpaceC = cameraRotation * (object.rotation * (object.vertices[triangle.c] * object.scale) + object.position - cameraPosition);

            Vector2 projectedA, projectedB, projectedC;

            if (projectCameraSpacePointTo2D(cameraSpaceA, fieldOfViewRadians, screenSize, projectedA) &&
                projectCameraSpacePointTo2D(cameraSpaceB, fieldOfViewRadians, screenSize, projectedB) &&
                projectCameraSpacePointTo2D(cameraSpaceC, fieldOfViewRadians, screenSize, projectedC))
            {
                drawLineWithDepth(projectedA, projectedB, -cameraSpaceA.z, -cameraSpaceB.z, object.outlineColor);
                drawLineWithDepth(projectedB, projectedC, -cameraSpaceB.z, -cameraSpaceC.z, object.outlineColor);
                drawLineWithDepth(projectedC, projectedA, -cameraSpaceC.z, -cameraSpaceA.z, object.outlineColor);
            }
        }
    }

    return buffer;
}

const std::vector<uint32_t> &Renderer::RenderWithWireframe(float outlineFraction)
{
    std::fill(buffer.begin(), buffer.end(), backgroundColor);
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::infinity());

    for (const auto &object : objects)
    {

        for (size_t triangleIndex = 0; triangleIndex < object.triangles.size(); ++triangleIndex)
        {
            const Triangle &triangle = object.triangles[triangleIndex];
            Vector3 edge1 = object.vertices[triangle.b] - object.vertices[triangle.a];
            Vector3 edge2 = object.vertices[triangle.c] - object.vertices[triangle.a];
            Vector3 faceNormal = Vector3::cross(edge2, edge1);
            if (faceNormal.dot(cameraPosition - (object.vertices[triangle.a] + object.position)) >= 0)
                continue;
            Vector3 cameraSpaceA = cameraRotation * (object.rotation * (object.vertices[triangle.a] * object.scale) + object.position - cameraPosition);
            Vector3 cameraSpaceB = cameraRotation * (object.rotation * (object.vertices[triangle.b] * object.scale) + object.position - cameraPosition);
            Vector3 cameraSpaceC = cameraRotation * (object.rotation * (object.vertices[triangle.c] * object.scale) + object.position - cameraPosition);

            Vector2 projectedA, projectedB, projectedC;
            if (projectCameraSpacePointTo2D(cameraSpaceA, fieldOfViewRadians, screenSize, projectedA) &&
                projectCameraSpacePointTo2D(cameraSpaceB, fieldOfViewRadians, screenSize, projectedB) &&
                projectCameraSpacePointTo2D(cameraSpaceC, fieldOfViewRadians, screenSize, projectedC))
            {
                float triangleArea = determinant(projectedA, projectedB, projectedC);
                // Rasterize the triangle into the buffer using z-buffering
                if (triangleArea < 0)
                {
                    std::swap(projectedB, projectedC);
                    std::swap(cameraSpaceB, cameraSpaceC);
                    triangleArea = -triangleArea;
                }
                int minX = static_cast<int>(std::floor(std::min({projectedA.x, projectedB.x, projectedC.x})));
                int maxX = static_cast<int>(std::ceil(std::max({projectedA.x, projectedB.x, projectedC.x})));
                int minY = static_cast<int>(std::floor(std::min({projectedA.y, projectedB.y, projectedC.y})));
                int maxY = static_cast<int>(std::ceil(std::max({projectedA.y, projectedB.y, projectedC.y})));

                const int sw = static_cast<int>(screenSize.x);
                const int sh = static_cast<int>(screenSize.y);
                minX = std::max(minX, 0);
                minY = std::max(minY, 0);
                maxX = std::min(maxX, sw - 1);
                maxY = std::min(maxY, sh - 1);
                if (triangleArea <= 0.0f || minX > maxX || minY > maxY)
                    continue;

                const float invArea = 1.0f / triangleArea;
                const float dx0 = -(projectedC.y - projectedB.y), dy0 = projectedC.x - projectedB.x;
                const float dx1 = -(projectedA.y - projectedC.y), dy1 = projectedA.x - projectedC.x;
                const float dx2 = -(projectedB.y - projectedA.y), dy2 = projectedB.x - projectedA.x;
                const float izA = invArea / -cameraSpaceA.z;
                const float izB = invArea / -cameraSpaceB.z;
                const float izC = invArea / -cameraSpaceC.z;
                const float invLengthBC = 1.0f / std::hypot(projectedC.x - projectedB.x, projectedC.y - projectedB.y);
                const float invLengthCA = 1.0f / std::hypot(projectedA.x - projectedC.x, projectedA.y - projectedC.y);
                const float invLengthAB = 1.0f / std::hypot(projectedB.x - projectedA.x, projectedB.y - projectedA.y);
                const float size = 0.1f;
                const float minimumTriangleAltitude = std::min(
                    {triangleArea * size * invLengthBC,
                     triangleArea * size * invLengthCA,
                     triangleArea * size * invLengthAB});
                const float triangleOutlinePixels =
                    std::clamp(outlineFraction, 0.0f, 0.25f) * minimumTriangleAltitude;
                const Vector2 start = {minX + 0.5f, minY + 0.5f};
                float rowE0 = determinant(projectedB, projectedC, start);
                float rowE1 = determinant(projectedC, projectedA, start);
                float rowE2 = determinant(projectedA, projectedB, start);

                for (int y = minY; y <= maxY; ++y)
                {
                    float e0 = rowE0, e1 = rowE1, e2 = rowE2;
                    uint32_t *colorRow = &buffer[y * sw];
                    float *depthRow = &zBuffer[y * sw];
                    for (int x = minX; x <= maxX; ++x)
                    {
                        if (e0 >= 0 && e1 >= 0 && e2 >= 0)
                        {
                            const float depth = 1.0f / (e0 * izA + e1 * izB + e2 * izC);
                            if (depth < depthRow[x])
                            {
                                const bool isOnEdge =
                                    (e0 * invLengthBC <= triangleOutlinePixels) ||
                                    (e1 * invLengthCA <= triangleOutlinePixels) ||
                                    (e2 * invLengthAB <= triangleOutlinePixels);

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
        }
    }

    return buffer;
}