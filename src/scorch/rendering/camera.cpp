#include "camera.h"
#include <scorch/utils/matrix_math.h>

namespace ScorchEngine {

	void SECamera::setOrthographicProjection(
		float left, float right, float top, float bottom, float zNear /*Near clipping plane*/, float zFar /*Far clipping plane*/) {
		projectionMatrix = glm::mat4{ 1.0f };
		projectionMatrix[0][0] = 2.f / (right - left);
		projectionMatrix[1][1] = 2.f / (bottom - top);
		projectionMatrix[2][2] = 1.f / (zFar - zNear);
		projectionMatrix[3][0] = -(right + left) / (right - left);
		projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		projectionMatrix[3][2] = -zNear / (zFar - zNear);

		inverseProjectionMatrix = glm::inverse(projectionMatrix);
	}

	void SECamera::setPerspectiveProjection(float fovy, float aspect, float zNear /*Near clipping plane*/, float zFar /*Far clipping plane*/) {
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
		const float tanHalfFovy = tan(fovy / 2.f);
		projectionMatrix = glm::mat4{ 0.0f };
		projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		projectionMatrix[1][1] = 1.f / (tanHalfFovy);
		projectionMatrix[2][2] = zFar / (zFar - zNear);
		projectionMatrix[2][3] = 1.f;
		projectionMatrix[3][2] = -(zFar * zNear) / (zFar - zNear);

		inverseProjectionMatrix = glm::inverse(projectionMatrix);
	}
/* *
* Set camera view based on the camera's position and direction
*/
	void SECamera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
		const glm::vec3 w{ glm::normalize(direction) };
		const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
		const glm::vec3 v{ glm::cross(w, u) };

		viewMatrix = glm::mat4{ 1.f };
		viewMatrix[0][0] = u.x;
		viewMatrix[1][0] = u.y;
		viewMatrix[2][0] = u.z;
		viewMatrix[0][1] = v.x;
		viewMatrix[1][1] = v.y;
		viewMatrix[2][1] = v.z;
		viewMatrix[0][2] = w.x;
		viewMatrix[1][2] = w.y;
		viewMatrix[2][2] = w.z;
		viewMatrix[3][0] = -glm::dot(u, position);
		viewMatrix[3][1] = -glm::dot(v, position);
		viewMatrix[3][2] = -glm::dot(w, position);

		inverseViewMatrix = glm::mat4{ 1.f };
		inverseViewMatrix[0][0] = u.x;
		inverseViewMatrix[0][1] = u.y;
		inverseViewMatrix[0][2] = u.z;
		inverseViewMatrix[1][0] = v.x;
		inverseViewMatrix[1][1] = v.y;
		inverseViewMatrix[1][2] = v.z;
		inverseViewMatrix[2][0] = w.x;
		inverseViewMatrix[2][1] = w.y;
		inverseViewMatrix[2][2] = w.z;
		inverseViewMatrix[3][0] = position.x;
		inverseViewMatrix[3][1] = position.y;
		inverseViewMatrix[3][2] = position.z;
	}
/* *
* Set camera view based on the camera's position and target
*/
	void SECamera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
		assert((target - position) != glm::vec3(0) && "direction cannot be 0");
		setViewDirection(position, target - position, up);
	}
/* *
* Set camera view based on the camera's position and rotation
*/
	void SECamera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
		inverseViewMatrix = Math::calculateYXZ(position, rotation, glm::vec3(1.0f));

		const glm::vec3 u = inverseViewMatrix[0];
		const glm::vec3 v = inverseViewMatrix[1];
		const glm::vec3 w = inverseViewMatrix[2];
		viewMatrix = glm::mat4{ 1.f };
		viewMatrix[0][0] = u.x;
		viewMatrix[1][0] = u.y;
		viewMatrix[2][0] = u.z;
		viewMatrix[0][1] = v.x;
		viewMatrix[1][1] = v.y;
		viewMatrix[2][1] = v.z;
		viewMatrix[0][2] = w.x;
		viewMatrix[1][2] = w.y;
		viewMatrix[2][2] = w.z;

		viewMatrix[3][0] = -glm::dot(u, position);
		viewMatrix[3][1] = -glm::dot(v, position);
		viewMatrix[3][2] = -glm::dot(w, position);
	}
	void SECamera::setViewYXZ(glm::mat4 transformMatrix) {
		inverseViewMatrix = transformMatrix;

		viewMatrix = glm::mat4{ 1.f };

		const glm::vec3 u = inverseViewMatrix[0];
		const glm::vec3 v = inverseViewMatrix[1];
		const glm::vec3 w = inverseViewMatrix[2];

		viewMatrix[0][0] = inverseViewMatrix[0][0];
		viewMatrix[1][0] = inverseViewMatrix[0][1];
		viewMatrix[2][0] = inverseViewMatrix[0][2];
		viewMatrix[0][1] = inverseViewMatrix[1][0];
		viewMatrix[1][1] = inverseViewMatrix[1][1];
		viewMatrix[2][1] = inverseViewMatrix[1][2];
		viewMatrix[0][2] = inverseViewMatrix[2][0];
		viewMatrix[1][2] = inverseViewMatrix[2][1];
		viewMatrix[2][2] = inverseViewMatrix[2][2];

		glm::vec3 position = inverseViewMatrix[3];
		viewMatrix[3][0] = -glm::dot(u, position);
		viewMatrix[3][1] = -glm::dot(v, position);
		viewMatrix[3][2] = -glm::dot(w, position);
	}
}