#pragma once
#include <SDL3/SDL_events.h>
#include <glm/glm.hpp>

class OrbitCamera {
public:
    OrbitCamera(float yaw, float pitch, float distance) : yaw_(yaw), pitch_(pitch), distance_(distance) {
    }

    void HandleSDLInput(const SDL_Event *event);

    glm::mat4 GetViewMatrix();

    glm::mat4 GetProjectionMatrix(float aspect);

    glm::mat4 GetViewProjectionMatrix(float aspect);

    glm::vec3 GetPos() const {
        glm::vec3 eyePos;
        eyePos.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
        eyePos.y = std::sin(glm::radians(pitch_));
        eyePos.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));

        eyePos *= distance_;
        return eyePos;
    }

    float GetYaw() const { return yaw_; }
    float GetPitch() const { return pitch_; }
    float GetDistance() const { return distance_; }

    void SetState(float yaw, float pitch, float distance) {
        yaw_ = yaw;
        pitch_ = pitch;
        distance_ = distance;
    }

private:
    float yaw_;
    float pitch_;
    float distance_;

    bool isRotating_{false};
    bool isZooming_{false};

    bool usePerspective_{true};
};
