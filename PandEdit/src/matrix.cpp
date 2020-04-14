//  ===== Date Created: 14 April, 2020 ===== 

#include "matrix.hpp"

Matrix4 Matrix4::ortho(float left, float right, float top, float bottom, float nearPlane, float farPlane)
{
	Matrix4 result;
	
	int i = 0;

	result.data[i++] = 2.0f / (right - left);
	result.data[i++] = 0.0f;
	result.data[i++] = 0.0f;
	result.data[i++] = 0.0f;

	result.data[i++] = 0.0f;
	result.data[i++] = 2.0f / (top - bottom);
	result.data[i++] = 0.0f;
	result.data[i++] = 0.0f;

	result.data[i++] = 0.0f;
	result.data[i++] = 0.0f;
	result.data[i++] = -2.0f / (farPlane - nearPlane);
	result.data[i++] = 0.0f;

	result.data[i++] = -1.0f * ((right + left) / (right - left));
	result.data[i++] = -1.0f * ((top + bottom) / (top - bottom));
	result.data[i++] = -1.0f * ((farPlane + nearPlane) / (farPlane - nearPlane));
	result.data[i++] = 1.0f;

	return result;
}
