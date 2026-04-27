#include "OrbitCamera.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <algorithm>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace {
    constexpr float kRotateSpeed = 0.2f;
    constexpr float kZoomSpeed = 0.1f;
}

void OrbitCamera::HandleSDLInput(const SDL_Event *event) {
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            isRotating_ = true;
        } else if (event->button.button == SDL_BUTTON_RIGHT) {
            isZooming_ = true;
        }
    } else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event->button.button == SDL_BUTTON_LEFT) {
            isRotating_ = false;
        } else if (event->button.button == SDL_BUTTON_RIGHT) {
            isZooming_ = false;
        }
    } else if (event->type == SDL_EVENT_MOUSE_MOTION) {
        float deltaX = event->motion.xrel;
        float deltaY = event->motion.yrel;
        if (isRotating_) {
            yaw_ += deltaX * kRotateSpeed;
            pitch_ -= deltaY * kRotateSpeed;
            pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
        }
        if (isZooming_) {
            distance_ += deltaX * kZoomSpeed;
        }
    } else if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_P) {
            usePerspective_ = !usePerspective_;
        }
    }
}

glm::mat4 OrbitCamera::GetViewMatrix() {
    return glm::lookAt(GetPos(), {0, 0, 0}, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 OrbitCamera::GetProjectionMatrix(float aspect) {
    glm::mat4 projection;
    if (usePerspective_) {
        float nearPlane = 0.1f;
        float farPlane = 10000.0f;

        projection = glm::perspective(glm::radians(70.f),
                                      aspect,
                                      nearPlane, farPlane);
        projection[1][1] *= -1;
        projection[2][2] = nearPlane / (farPlane - nearPlane);
        projection[3][2] = (farPlane * nearPlane) / (farPlane - nearPlane);
    } else {
        projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, 1000.0f, 0.1f);
    }
    return projection;
}

glm::mat4 OrbitCamera::GetViewProjectionMatrix(float aspect) {
    return GetProjectionMatrix(aspect) * GetViewMatrix();
}
