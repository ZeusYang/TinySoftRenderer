#ifndef TRMATHUTILS_H
#define TRMATHUTILS_H

#include "glm/glm.hpp"

namespace TinyRenderer
{

	class TRMathUtils final
	{
	public:

		//Transformation functions
		static glm::mat4 calcViewPortMatrix(int width, int height);
		static glm::mat4 calcViewMatrix(glm::vec3 camera, glm::vec3 target, glm::vec3 worldUp);
		static glm::mat4 calcPerspProjectMatrix(float fovy, float aspect, float near, float far);
		static glm::mat4 calcOrthoProjectMatrix(float left, float right, float bottom, float top, float near, float far);

	};
}

#endif