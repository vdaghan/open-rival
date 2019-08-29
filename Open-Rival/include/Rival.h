#ifndef RIVAL_H
#define RIVAL_H

#include <gl/glew.h>
#include <SDL.h>
#include <iostream>
#include <string>

#include "Scene.h"
#include "Sprite.h"
#include "Texture.h"
#include "UnitRenderer.h"
#include "Window.h"

namespace Rival {

    class Rival {

    public:

        static const int windowWidth = 800;
        static const int windowHeight = 600;

        /**
         * Initialises the game.
         */
        void initialize();

        /**
         * Runs the game.
         */
        void start();

    private:

        const char* windowTitle = "Rival Realms";
        const int numTextures = 50;

        std::unique_ptr<std::vector<Texture>> textures =
                std::make_unique<std::vector<Texture>>();
        std::unique_ptr<std::map<Unit::Type, Sprite>> unitSprites =
                std::make_unique<std::map<Unit::Type, Sprite>>();
        std::unique_ptr<Texture> paletteTexture;
        std::unique_ptr<Scene> scene;
        std::unique_ptr<UnitRenderer> unitRenderer;

        /**
         * Window used to display the game.
         */
        std::unique_ptr<Window> window;

        /**
         * The window renderer.
         */
        SDL_Renderer* renderer = nullptr;

        /**
         * Initializes SDL.
         */
        void initSDL() const;

        /**
         * Initializes GLEW.
         */
        void initGLEW() const;

        /**
         * Initializes OpenGL.
         */
        void initGL();

        /**
         * Loads the Textures and initializes the Sprites.
         */
        void initSprites();

        /**
         * Loads a Unit's Texture and initializes its Sprite.
         */
        void loadUnit(Unit::Type type, std::string imageFilename);

        /**
         * Loads the game's textures.
         */
        const Texture loadTexture(const std::string filename) const;

        /**
         * Handles keyDown events.
         */
        void keyDown(const SDL_Keycode keyCode) const;

        /**
         * Renders the current frame.
         */
        void render();

        /**
         * Updates the game.
         */
        void update();

        /**
         * Frees media and shuts down SDL.
         */
        void exit();

    };

}

#endif // RIVAL_H
