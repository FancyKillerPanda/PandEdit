//  ===== Date Created: 14 April, 2020 ===== 

#if !defined(MATRIX_HPP)
#define MATRIX_HPP

class Matrix4
{
public:
	float data[16];

public:
	static Matrix4 ortho(float left, float right, float top, float bottom, float nearPlane, float farPlane);
};

#endif
