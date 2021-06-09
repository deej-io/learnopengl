#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

enum class direction {
    forward,
    backward,
    left,
    right
};

constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.1f;
constexpr float FOV = 45.0f;

class Camera {
    glm::vec3 position_;
    glm::vec3 forward_ = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 world_up_ = up_;
    glm::vec3 right_;

    float yaw_;
    float pitch_;

    float movement_speed_ = SPEED;
    float mouse_sensitivity_ = SENSITIVITY;
    float fov_ = FOV;

public:
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH)
    : position_(position), yaw_(yaw), pitch_(pitch) {
        update_camera_vectors();
    }

    glm::mat4 view() const {
        return glm::lookAt(position_, position_ + forward_, up_);
    }

    float fov() {
        return fov_;
    }

    void handle_keyboard(direction direction, float delta_time) {
        const float velocity = movement_speed_ * delta_time;

        switch (direction) {
        case direction::forward:
            position_ += forward_ * velocity;
            break;
        case direction::backward:
            position_ -= forward_ * velocity;
            break;
        case direction::left:
            position_ -= right_ * velocity;
            break;
        case direction::right:
            position_ += right_ * velocity;
            break;
        }
    }

    void handle_mouse(float xoffset, float yoffset, bool constrain_pitch = true) {
        xoffset *= mouse_sensitivity_;
        yoffset *= mouse_sensitivity_;

        yaw_ += xoffset;
        pitch_ -= yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrain_pitch) {
            pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
        }

        update_camera_vectors();
    }

    void handle_scroll(float yoffset) {
        fov_ -= std::clamp(yoffset, 1.0f, 45.f);
    }

private:
    void update_camera_vectors() {
        forward_ = glm::normalize(glm::vec3{
            std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_)),
            std::sin(glm::radians(pitch_)),
            std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_)),
        });
        right_ = glm::normalize(glm::cross(forward_, world_up_));
    }
};

#endif
