#pragma once
#include <numbers>

static float pi = 3.14159265359f;

struct vec3 {
	constexpr vec3(
		const float x = 0.f,
		const float y = 0.f,
		const float z = 0.f) noexcept : 
		x(x), y(y), z(z) { }

	constexpr const vec3& operator-(const vec3 other) const noexcept {
		return vec3{ x - other.x, y - other.y, z - other.z };
	}
	constexpr const vec3& operator+(const vec3 other) const noexcept {
		return vec3{ x + other.x, y + other.y, z + other.z };
	}
	constexpr const vec3& operator/(const float factor) const noexcept {
		return vec3{ x / factor, y / factor, z / factor };
	}

	constexpr const vec3& toang() const noexcept {
		return vec3{
			std::atan2(-z, std::hypotf(x, y)) * (180.f / pi),
			std::atan2(x, y) * (180.f / pi),
			0.f
		};
	}

	float x, y, z;
};

struct view_matrix_t {
	float* operator[ ](int index) {
		return matrix[index];
	}

	float matrix[4][4];
};

bool world_to_screen(const vec3& screen_size, const vec3& pos, vec3& out, view_matrix_t matrix) {
	out.x = matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3];
	out.y = matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3];

	float w = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

	if (w < 0.01f)
		return false;

	float inv_w = 1.f / w;
	out.x *= inv_w;
	out.y *= inv_w;

	float x = screen_size.x * .5f;
	float y = screen_size.y * .5f;

	x += 0.5f * out.x * screen_size.x + 0.5f;
	y -= 0.5f * out.y * screen_size.y + 0.5f;

	out.x = x;
	out.y = y;

	return true;
}