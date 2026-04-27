#include "FlyingCamera.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

glm::mat4 FlyingCamera::GetViewMatrix() {
    glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.0f), position_);
    glm::mat4 cameraRotation = GetRotationMatrix();
    return glm::inverse(cameraTranslation * cameraRotation);
}

glm::mat4 FlyingCamera::GetRotationMatrix() {
    glm::quat pitchRotation = glm::angleAxis(pitch_, glm::vec3{1.0f, 0.0f, 0.0f});
    glm::quat yawRotation = glm::angleAxis(yaw_, glm::vec3{0.0f, 1.0f, 0.0f});

    return glm::mat4_cast(yawRotation) * glm::mat4_cast(pitchRotation);
}

glm::mat4 FlyingCamera::GetProjectionMatrix(float aspect) {
    float nearPlane = 0.1f;
    float farPlane = 10000.0f;
    auto projection = glm::perspective(glm::radians(70.f),
                                       aspect,
                                       nearPlane, farPlane);
    projection[1][1] *= -1;
    projection[2][2] = nearPlane / (farPlane - nearPlane);
    projection[3][2] = (farPlane * nearPlane) / (farPlane - nearPlane);
    return projection;
}

void FlyingCamera::ProcessSDLEvent(const SDL_Event &event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        switch (event.key.key) {
            case SDLK_W:
                moveForward = true;
                break;
            case SDLK_S:
                moveBackward = true;
                break;
            case SDLK_A:
                moveLeft = true;
                break;
            case SDLK_D:
                moveRight = true;
                break;
            default:
                break;
        }
    }
    if (event.type == SDL_EVENT_KEY_UP) {
        switch (event.key.key) {
            case SDLK_W:
                moveForward = false;
                break;
            case SDLK_S:
                moveBackward = false;
                break;
            case SDLK_A:
                moveLeft = false;
                break;
            case SDLK_D:
                moveRight = false;
                break;
            default:
                break;
        }
    }
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        if (event.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
            yaw_ -= event.motion.xrel * sensitivity_;
            pitch_ -= event.motion.yrel * sensitivity_;
            pitch_ = glm::clamp(pitch_, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);
        }
    }
}

void FlyingCamera::Update(float dt) {
    velocity_ = glm::vec3(0.0f);
    if (moveForward) velocity_.z -= 1.0f;
    if (moveBackward) velocity_.z += 1.0f;
    if (moveLeft) velocity_.x -= 1.0f;
    if (moveRight) velocity_.x += 1.0f;

    if (glm::length(velocity_) > 0.0f) {
        velocity_ = glm::normalize(velocity_);
    }

    auto rotation = GetRotationMatrix();
    position_ += glm::vec3(rotation * glm::vec4(velocity_ * speed_ * dt, 0.0f));
}
