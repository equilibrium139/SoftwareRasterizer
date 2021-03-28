#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>


template<typename T>
class Vec3
{
public:
	union {
		struct {
			T x, y, z;
		};
		struct {
			T r, g, b;
		};
		T v[3];
	};

	Vec3() {}
	explicit Vec3(T x) : x(x), y(x), z(x) {}
	Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
	
	Vec3 operator-() const { return Vec3{ -x, -y, -z }; }
	T operator[](int i) const { return v[i]; }
	T& operator[](int i) { return v[i]; }

	Vec3& operator+=(const Vec3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
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

	Vec3& operator *=(T rhs) {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}

	Vec3& Normalize() {
		T len = v.length();
		if (len > 0) {
			T invLen = 1 / len;
			*this *= invLen;
		}
		return *this;
	}

	T length() const {
		return std::sqrt(x*x + y*y + z*z);
	}

	T lengthSquared() const {
		return x*x + y*y + z*z;
	}
};

template<typename T>
inline std::ostream& operator<<(std::ostream & os, const Vec3<T> & v) {
	return os << v.x << ' ' << v.y << ' ' << v.z;
}

template<typename T>
inline Vec3<T> operator+(const Vec3<T>& lhs, const Vec3<T>& rhs) {
	return Vec3<T> {
		lhs.x + rhs.x,
		lhs.y + rhs.y,
		lhs.z + rhs.z
	};
}

template<typename T>
inline Vec3<T> operator-(const Vec3<T>& lhs, const Vec3<T>& rhs) {
	return Vec3<T>{
		lhs.x - rhs.x,
		lhs.y - rhs.y,
		lhs.z - rhs.z
	};
}

template<typename T>
inline Vec3<T> operator*(const Vec3<T>& lhs, const Vec3<T>& rhs) {
	return Vec3<T>{
		lhs.x * rhs.x,
		lhs.y * rhs.y,
		lhs.z * rhs.z
	};
}

template<typename T>
inline Vec3<T> operator*(const Vec3<T>& lhs,  T rhs) {
	return Vec3<T>{
		lhs.x * rhs,
		lhs.y * rhs,
		lhs.z * rhs
	};
}

template<typename T>
inline Vec3<T> operator*(T lhs, const Vec3<T>& rhs) {
	return rhs * lhs;
}

template<typename T>
inline Vec3<T> operator/(const Vec3<T>& lhs, T rhs) {
	return lhs * (1.0f/rhs);
}

template<typename T>
inline T Dot(const Vec3<T>& lhs, const Vec3<T>& rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template<typename T>
inline Vec3<T> Cross(const Vec3<T>& lhs, const Vec3<T>& rhs) {
	return {
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x
	};
}

template<typename T>
inline Vec3<T> Normalize(const Vec3<T>& v) {
	Vec3<T> vCopy = v;
	vCopy.Normalize();
	return vCopy;
}

template<typename T>
inline bool NearZero(const Vec3<T>& vec) {
	const T e = 1e-8f;
	return (std::fabs(vec.x) < e) && (std::fabs(vec.y) < e) && (std::fabs(vec.z) < e);
}

template<typename T>
Vec3<T> Reflect(const Vec3<T>& v, const Vec3<T>& n) {
	return v - 2 * Dot(v, n) * n;
}

template<typename T>
Vec3<T> Refract(const Vec3<T>& uv, const Vec3<T>& n, T etaiOverEtat) {
	T cosTheta = std::fmin(Dot(-uv, n), 1.0f);
	Vec3<T> rOutPerp = etaiOverEtat * (uv + cosTheta * n);
	Vec3<T> rOutParallel = -std::sqrt(std::fabs(1.0 - rOutPerp.lengthSquared())) * n;
	return rOutPerp + rOutParallel;
}

using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;
using Color = Vec3f;
using Point = Vec3f;

#endif // VEC3_H