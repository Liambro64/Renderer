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
      zBuffer(screenSize.x * screenSize.y, std::numeric_limits<float>::infinity())
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

    for (const auto &object : objects)
    {
        Rasterizer *rasterizer = object.rasterizer;
        rasterizer->SetBuffer(buffer, zBuffer);
        for (const auto &triangle : object.triangles)
        {
            rasterizer->Render(screenSize, cameraPosition, cameraRotation, object, triangle, fieldOfViewRadians);
        }
    }

    return buffer;
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
