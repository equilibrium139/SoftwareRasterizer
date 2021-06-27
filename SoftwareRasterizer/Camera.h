#ifndef CAMERA_H
#define CAMERA_H

#include "Matrix.h"
#include "Vector.h"
#include "Utilities.h"

enum Camera_Movement {
    CAM_FORWARD,
    CAM_BACKWARD,
    CAM_LEFT,
    CAM_RIGHT
};

// Default camera values
const float YAW = 0.0f;
const float PITCH = 0.0f;
const float SPEED = 1.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    Vec3 position;
    Vec3 front;
    Vec3 up;
    Vec3 right;
    Vec3 worldUp;
    // euler Angles
    float yaw;
    float pitch;
    // camera options
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    // constructor with vectors
    Camera(Vec3 position = Vec3(0.0f, 0.0f, 0.0f), Vec3 up = Vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) :
         front(Vec3(0.0f, 0.0f, 1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM),
         position(position), worldUp(up), yaw(yaw), pitch(pitch)
    {
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) :
        front(Vec3(0.0f, 0.0f, 1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM), 
        position(posX, posY, posZ), worldUp(upX, upY, upZ), yaw(yaw), pitch(pitch)
    {
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    Mat4 GetViewMatrix() const
    {
        return LookAt(position, right, up, front);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;
        if (direction == CAM_FORWARD)
            position += front * velocity;
        else if (direction == CAM_BACKWARD)
            position -= front * velocity;
        else if (direction == CAM_LEFT)
            position -= right * velocity;
        else if (direction == CAM_RIGHT)
            position += right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        zoom -= (float)yoffset;
        if (zoom < 1.0f)
            zoom = 1.0f;
        if (zoom > 45.0f)
            zoom = 45.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        front.x = sin(Radians(yaw)) * cos(Radians(pitch));
        front.y = sin(Radians(pitch));
        front.z = cos(Radians(yaw)) * cos(Radians(pitch));
        front = Normalize(front);
        // also re-calculate the Right and Up vector
        right = Normalize(Cross(worldUp, front));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        up = Normalize(Cross(front, right));
    }
};

#endif // !CAMERA_H

