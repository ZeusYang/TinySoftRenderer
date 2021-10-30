#include "TRMathUtils.h"

#include "glm/gtc/matrix_transform.hpp"

namespace TinyRenderer
{
	glm::mat4 TRMathUtils::calcViewPortMatrix(int width, int height)
	{
		//Setup viewport matrix (ndc space -> screen space)
		glm::mat4 vpMat;
		float hwidth = width * 0.5f;
		float hheight = height * 0.5f;
		vpMat[0][0] = hwidth; vpMat[0][1] = 0.0f;     vpMat[0][2] = 0.0f; vpMat[0][3] = 0.0f;
		vpMat[1][0] = 0.0f;	  vpMat[1][1] = -hheight; vpMat[1][2] = 0.0f; vpMat[1][3] = 0.0f;
		vpMat[2][0] = 0.0f;   vpMat[2][1] = 0.0f;     vpMat[2][2] = 1.0f; vpMat[2][3] = 0.0f;
		vpMat[3][0] = hwidth; vpMat[3][1] = hheight;  vpMat[3][2] = 0.0f; vpMat[3][3] = 0.0f;
		return vpMat;
	}

	glm::mat4 TRMathUtils::calcViewMatrix(glm::vec3 camera, glm::vec3 target, glm::vec3 worldUp)
	{
		//Setup view matrix (world space -> camera space)
		glm::mat4 vMat;
		glm::vec3 zAxis = glm::normalize(camera - target);
		glm::vec3 xAxis = glm::normalize(glm::cross(worldUp, zAxis));
		glm::vec3 yAxis = glm::normalize(glm::cross(zAxis, xAxis));

		vMat[0][0] = xAxis.x; vMat[0][1] = yAxis.x; vMat[0][2] = zAxis.x; vMat[0][3] = 0.0f;
		vMat[1][0] = xAxis.y; vMat[1][1] = yAxis.y; vMat[1][2] = zAxis.y; vMat[1][3] = 0.0f;
		vMat[2][0] = xAxis.z; vMat[2][1] = yAxis.z; vMat[2][2] = zAxis.z; vMat[2][3] = 0.0f;
		vMat[3][0] = -glm::dot(xAxis, camera);
		vMat[3][1] = -glm::dot(yAxis, camera);
		vMat[3][2] = -glm::dot(zAxis, camera);
		vMat[3][3] = 1.0f;

		return vMat;
	}

	glm::mat4 TRMathUtils::calcPerspProjectMatrix(float fovy, float aspect, float near, float far)
	{
		//Setup perspective matrix (camera space -> homogeneous space)
		glm::mat4 pMat = glm::mat4(1.0f);

		float rFovy = fovy * 3.14159265358979323846 / 180;
		const float tanHalfFovy = std::tan(rFovy * 0.5f);
		float f_n = far - near;
		pMat[0][0] = 1.0f / (aspect*tanHalfFovy); pMat[0][1] = 0.0f;				pMat[0][2] = 0.0f;					pMat[0][3] = 0.0f;
		pMat[1][0] = 0.0f;						  pMat[1][1] = 1.0f / tanHalfFovy;  pMat[1][2] = 0.0f;					pMat[1][3] = 0.0f;
		pMat[2][0] = 0.0f;						  pMat[2][1] = 0.0f;			    pMat[2][2] = -(far + near) / f_n;	pMat[2][3] = -1.0f;
		pMat[3][0] = 0.0f;						  pMat[3][1] = 0.0f;				pMat[3][2] = -2.0f*near*far / f_n;	pMat[3][3] = 0.0f;

		return pMat;
	}

	glm::mat4 TRMathUtils::calcOrthoProjectMatrix(float left, float right, float bottom, float top, float near, float far)
	{
		//Setup orthogonal matrix (camera space -> homogeneous space)
		glm::mat4 pMat;
		pMat[0][0] = 2.0f / (right - left); pMat[0][1] = 0.0f;                  pMat[0][2] = 0.0f;                         pMat[0][3] = 0.0f;
		pMat[1][0] = 0.0f;				    pMat[1][1] = 2.0f / (top - bottom); pMat[1][2] = 0.0f;                         pMat[1][3] = 0.0f;
		pMat[2][0] = 0.0f;                  pMat[2][1] = 0.0f;                  pMat[2][2] = -2.0f / (far - near);         pMat[2][3] = 0.0f;
		pMat[3][0] = 0.0f;                  pMat[3][1] = 0.0f;                  pMat[3][2] = -(far + near) / (far - near); pMat[3][3] = 1.0f;
		return pMat;
	}
}