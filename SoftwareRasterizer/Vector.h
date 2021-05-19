#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>
#include <iostream>

using Color = std::uint32_t;

struct Vec2 {
	float x, y;
};

struct Vec2i {
	int x, y;
};

inline bool operator==(Vec2i lhs, Vec2i rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline Vec2 operator-(Vec2 a, Vec2 b) {
	return { a.x - b.x, a.y - b.y };
}

struct Vec4 {
	float x, y, z, w;
};

class Vec3
{
public:
	float x, y, z;

	Vec3() {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vec3 operator-() const { return Vec3{ -x, -y, -z }; }

	Vec3& operator+=(const Vec3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	Vec3& operator-=(const Vec3& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	Vec3& operator*=(const Vec3& rhs) {
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}

	Vec3& operator/=(const Vec3& rhs) {
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		return *this;
	}

	Vec3& operator *=(float rhs) {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	float length() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	float lengthSquared() const {
		return x * x + y * y + z * z;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
	return os << v.x << ' ' << v.y << ' ' << v.z;
}

inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs) {
	return Vec3{
		lhs.x + rhs.x,
		lhs.y + rhs.y,
		lhs.z + rhs.z
	};
}

inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs) {
	return Vec3{
		lhs.x - rhs.x,
		lhs.y - rhs.y,
		lhs.z - rhs.z
	};
}

inline Vec3 operator*(const Vec3& lhs, const Vec3& rhs) {
	return Vec3{
		lhs.x * rhs.x,
		lhs.y * rhs.y,
		lhs.z * rhs.z
	};
}

inline Vec3 operator*(const Vec3& lhs, float rhs) {
	return Vec3{
		lhs.x * rhs,
		lhs.y * rhs,
		lhs.z * rhs
	};
}

inline Vec3 operator*(float lhs, const Vec3& rhs) {
	return rhs * lhs;
}

inline Vec3 operator/(const Vec3& lhs, float rhs) {
	return lhs * (1.0f / rhs);
}

inline float Dot(const Vec3& lhs, const Vec3& rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline Vec3 Cross(const Vec3& lhs, const Vec3& rhs) {
	return {
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x
	};
}

inline Vec3 Normalize(const Vec3& v) {
	return v / v.length();
}

inline bool NearZero(const Vec3& vec) {
	const float e = 1e-8f;
	return (std::fabs(vec.x) < e) && (std::fabs(vec.y) < e) && (std::fabs(vec.z) < e);
}

inline Vec3 Reflect(const Vec3& v, const Vec3& n) {
	return v - 2 * Dot(v, n) * n;
}

inline Vec3 Refract(const Vec3& uv, const Vec3& n, float etaiOverEtat) {
	float cosTheta = std::fmin(Dot(-uv, n), 1.0f);
	Vec3 rOutPerp = etaiOverEtat * (uv + cosTheta * n);
	Vec3 rOutParallel = -std::sqrt(std::fabs(1.0f - rOutPerp.lengthSquared())) * n;
	return rOutPerp + rOutParallel;
}

inline Vec3 RotateX(Vec3 point, float angle) {
	return {
		point.x,
		point.y * std::cos(angle) + point.z * std::sin(angle),
		-point.y * std::sin(angle) + point.z * std::cos(angle)
	};
}

inline Vec3 RotateY(Vec3 point, float angle) {
	return {
		point.x * std::cos(angle) - point.z * std::sin(angle),
		point.y,
		point.x * std::sin(angle) + point.z * std::cos(angle)
	};
}

inline Vec3 RotateZ(Vec3 point, float angle) {
	return {
		point.x * std::cos(angle) - point.y * std::sin(angle),
		point.x * std::sin(angle) + point.y * std::cos(angle),
		point.z
	};
}

#endif // VECTOR_H