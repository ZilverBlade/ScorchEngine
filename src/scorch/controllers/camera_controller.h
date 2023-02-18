#pragma once

#include <scorch/window.h>
#include <scorch/ecs/actor.h>
#include <glm/glm.hpp>

namespace ScorchEngine {
    namespace Controllers {
	    class CameraController {
	    public:
            void moveTranslation(SEWindow& window, float dt, Actor actor);
            void moveOrientation(SEWindow& window, float dt, Actor actor);

            //bool mouseScrollEvent(Events::MouseScrollEvent& e);
   
            float moveSpeed{ 5.f };
            
            float sensitivity{ 15.f };
        private:
            struct KeyMappings {
                int moveLeft = GLFW_KEY_A;
                int moveRight = GLFW_KEY_D;
                int moveForward = GLFW_KEY_W;
                int moveBackward = GLFW_KEY_S;
                int moveUp = GLFW_KEY_SPACE;
                int moveDown = GLFW_KEY_RIGHT_CONTROL;
                int slowDown = GLFW_KEY_LEFT_CONTROL;
                int speedUp = GLFW_KEY_LEFT_SHIFT;
            } keys{};

            struct ButtonMappings {
                int canRotate = GLFW_MOUSE_BUTTON_RIGHT;
            } buttons{};
             glm::vec3 orientation{ .0f, .0f, 6.28318530718f };
             glm::vec3 upVec{ 0.f, 1.f, 0.f };
             bool firstClick = true;
             Actor actor{};

             float speedModifier = 1.f;
	    };

    }
}