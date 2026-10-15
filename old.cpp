#include <iostream>
#include <cmath>

// Define a basic Quaternion structure
struct Quaternion {
    double w, x, y, z;

    // Multiply two quaternions using the Hamilton product
    Quaternion multiply(const Quaternion& q) const {
        return Quaternion {
            w * q.w - x * q.x - y * q.y - z * q.z,
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w
        };
    }

    void print() const {
        std::cout << "w: " << w << ", x: " << x << ", y: " << y << ", z: " << z << "\n";
    }
};

// Helper function to create an axis-angle quaternion
Quaternion fromAxisAngle(double angle_degrees, double ax, double ay, double az) {
    // Convert degrees to radians and halve the angle
    double radians = angle_degrees * (M_PI / 180.0);
    double halfAngle = radians * 0.5;
    
    double sinHalf = std::sin(halfAngle);
    double cosHalf = std::cos(halfAngle);

    return Quaternion { cosHalf, ax * sinHalf, ay * sinHalf, az * sinHalf };
}

int main() {
    // 1. Create individual quaternions for 45 degrees on each axis
    Quaternion qX = fromAxisAngle(45.0, 1.0, 0.0, 0.0);
    Quaternion qY = fromAxisAngle(45.0, 0.0, 1.0, 0.0);
    Quaternion qZ = fromAxisAngle(45.0, 0.0, 0.0, 1.0);

    // 2. Combine them via Hamilton product (Order: Z * Y * X for standard intrinsic rollout)
    Quaternion qCombined = qZ.multiply(qY.multiply(qX));

    std::cout << "Final Combined Quaternion:\n";
    qCombined.print();

    return 0;
}
