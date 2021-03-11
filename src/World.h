#pragma once

#include <memory>
#include "ECS/EnTT.h"

#include "GameMap.h"
#include "GameManager.h"

#include "GUI/UIState.h"
#include "GUI/Ingame.h"

#include "Parsers/Gm1File.h"

#include "Rendering/Texture.h"
#include "Rendering/Camera.h"

#include "Events/Event.h"
#include "Events/Keyboard.h"
#include "Events/Mouse.h"
#include "Events/Touch.h"

#include "ECS/System/RenderSystem.h"
#include "ECS/System/AnimationFrameSystem.h"
#include "ECS/System/TestTeleportingDeerSystem.h"

namespace Sourcehold {
    namespace Parsers {
        class TgxFile;
        class Gm1File;
    }
    namespace Game {
        struct Scroll {
            bool shouldScroll = false;
            bool setByKeyboard = false;
            bool setByMouse = false;

            explicit operator bool() {
                return shouldScroll;
            }
        };

        struct ScrollInformation {
            Scroll up;
            Scroll down;
            Scroll right;
            Scroll left;
        };

        using namespace GUI;
        using namespace Parsers;
        using namespace Events;

        // Just testing. Nothing to see here, move along! //
        class Unit {
        public:
            Unit(int x, int y, const char *f) : x_(x), y_(y) {
                file = GetGm1(std::string("gm/") + f + std::string(".gm1"));
            }
            virtual ~Unit() = default;

            virtual void Update([[maybe_unused]]double dt) {};

            void Render() {
                Camera& cam = Camera::instance();

                int px = x_ * 30 - cam.pos_x_;
                int py = y_ * 15 - cam.pos_y_;

                SDL_Rect r = file->GetTextureAtlas()->Get(static_cast<uint32_t>(index));
                Rendering::Render(*file->GetTextureAtlas(), px, py, &r);
            }

            int x_, y_;
        protected:
            int index=0;
            std::shared_ptr<Gm1File> file;
        };

        /**
         * Handles everything related to the game world, including
         * loading, rendering and moving the camera
         */
        class Building;
        class Entity;
        class World :
            public GameMap,
            private EventConsumer<Keyboard>,
            private EventConsumer<Mouse>,
            private EventConsumer<Touch>
        {
            ScrollInformation scroll;
            IngameGUI gui;
            std::vector<Unit*> units;
            entt::registry registry;
        public:
            World();
            World(const World&) = delete;
            ~World();

            UIState Play();
        protected:
            void UpdateCamera(float dt);
        private:
            void onEventReceive(Keyboard& keyEvent) override;
            void onEventReceive(Mouse& mouseEvent) override;
            void onEventReceive(Touch& touchEvent) override;
        };
    }
}
