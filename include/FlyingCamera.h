#pragma once

#include <SDL3/SDL_events.h>

#include "glm/glm.hpp"

class FlyingCamera {
public:
    FlyingCamera(const glm::vec3& pos) : position_{pos} {}
    glm::mat4 GetViewMatrix();

    glm::mat4 GetRotationMatrix();

    glm::mat4 GetProjectionMatrix(float aspect);

    void ProcessSDLEvent(const SDL_Event &event);

    glm::vec3 GetPos() const
    {
        return position_;
    }

    void Update(float dt);
    
    float& GetSpeed() { return speed_; }
    float& GetSensitivity() { return sensitivity_; }

    void SetPosition(const glm::vec3& pos) { position_ = pos; }
    void SetRotation(float pitch, float yaw) { pitch_ = pitch; yaw_ = yaw; }
    float GetPitch() const { return pitch_; }
    float GetYaw() const { return yaw_; }

private:
    glm::vec3 velocity_{};
    glm::vec3 position_;
    float pitch_{};
    float yaw_{};
    float speed_{5.0f};
    float sensitivity_{0.005f};

    bool moveForward{false};
    bool moveBackward{false};
    bool moveLeft{false};
    bool moveRight{false};
};
