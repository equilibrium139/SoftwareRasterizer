#ifndef CLIPPING_H
#define CLIPPING_H

#include <array>
#include <bitset>
#include <cstdint>
#include <vector>
#include <utility>
#include "Triangle.h"
#include "Vector.h"

struct Plane {
	Vec3 p;
	Vec3 n;
};

using ClipFlags = std::uint8_t;

enum FrustumPlaneIndices {
	RIGHT_PLANE, LEFT_PLANE, TOP_PLANE, BOTTOM_PLANE, NEAR_PLANE, FAR_PLANE
};

struct VertClipData {
	std::bitset<6> outside;
	VertClipData(const Vec4& v) {
		outside[RIGHT_PLANE] = v.x > v.w;
		outside[LEFT_PLANE] = v.x < -v.w;
		outside[TOP_PLANE] = v.y > v.w;
		outside[BOTTOM_PLANE] = v.y < -v.w;
		outside[NEAR_PLANE] = v.z < 0;
		outside[FAR_PLANE] = v.z > v.w;
	}
};

std::vector<ClipSpaceTriangle> ClipAndCull(const std::vector<Face>& faces, const std::vector<Vec4>& clipSpaceVertices);

inline bool ShouldCull(VertClipData a, VertClipData b, VertClipData c) {
	return
		(a.outside[RIGHT_PLANE] && b.outside[RIGHT_PLANE] && c.outside[RIGHT_PLANE]) ||     
		(a.outside[LEFT_PLANE] && b.outside[LEFT_PLANE] && c.outside[LEFT_PLANE]) || 
		(a.outside[TOP_PLANE] && b.outside[TOP_PLANE] && c.outside[TOP_PLANE]) ||	 
		(a.outside[BOTTOM_PLANE] && b.outside[BOTTOM_PLANE] && c.outside[BOTTOM_PLANE]) || 
		(a.outside[NEAR_PLANE] && b.outside[NEAR_PLANE] && c.outside[NEAR_PLANE]) ||
		(a.outside[FAR_PLANE] && b.outside[FAR_PLANE] && c.outside[FAR_PLANE]);
}

// Only a needs to be clipped to near plane.
inline std::pair<ClipSpaceTriangle, ClipSpaceTriangle> Clip1Vertex(const Vec4& a, Vec2 aUV, const Vec4& b, Vec2 bUV, const Vec4& c, Vec2 cUV) {
	const float abAlpha = -a.z / (b.z - a.z);
	const float acAlpha = -a.z / (c.z - a.z);
	Vec4 abBegin = a + (b - a) * abAlpha;
	Vec4 acBegin = a + (c - a) * acAlpha;
	Vec2 abBeginUV = aUV + (bUV - aUV) * abAlpha;
	Vec2 acBeginUV = aUV + (cUV - aUV) * acAlpha;
	return {
		{abBegin, b, c, abBeginUV, bUV, cUV},
		{abBegin, c, acBegin, abBeginUV, cUV, acBeginUV},
	};
}

// a and b need to be clipped to near plane
inline ClipSpaceTriangle Clip2Vertices(const Vec4& a, Vec2 aUV, const Vec4& b, Vec2 bUV, const Vec4& c, Vec2 cUV) {
	const float acAlpha = -a.z / (c.z - a.z);
	const float bcAlpha = -b.z / (c.z - b.z);
	Vec4 acBegin = a + (c - a) * acAlpha; // Replaces a
	Vec4 bcBegin = b + (c - b) * bcAlpha; // Replaces b
	Vec2 acBeginUV = aUV + (cUV - aUV) * acAlpha;
	Vec2 bcBeginUV = bUV + (cUV - bUV) * bcAlpha;
	return {
		acBegin, bcBegin, c, acBeginUV, bcBeginUV, cUV
	};
}

#endif // !CLIPPING_H
