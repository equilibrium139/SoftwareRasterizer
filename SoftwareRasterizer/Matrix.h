#pragma once

class Mat4 {
public:
	float* operator[] (int i) { return m[i]; }
	const float* operator[] (int i) const { return m[i]; }
	float m[4][4];
};

inline Mat4 operator*(const Mat4& lhs, const Mat4& rhs) {
	Mat4 product;
	for (int r = 0; r < 4; r++) {
		for (int c = 0; c < 4; c++) {
			product[r][c] = lhs[r][0] * rhs[0][c] +
							lhs[r][1] * rhs[1][c] +
							lhs[r][2] * rhs[2][c] + 
							lhs[r][3] * rhs[3][c];
		}
	}
	return product;
}

