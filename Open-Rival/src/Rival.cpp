#include "pch.h"
#include "Rival.h"

// Delete <cstdlib>,<chrono> and <thread> if random number generation is no longer necessary
#include <cstdlib>
#include <chrono>
#include <thread>
// Delete <iostream> if std::cout is no longer utilised for quick-debugging
#include <iostream>

#include <gl/glew.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <stdexcept>

#include "Image.h"
#include "Palette.h"
#include "Race.h"
#include "RenderUtils.h"
#include "ScenarioBuilder.h"
#include "ScenarioData.h"
#include "ScenarioReader.h"
#include "Shaders.h"
#include "Spritesheet.h"
#include "TimerUtils.h"

namespace Rival {

    static const double aspectRatio =
            static_cast<double>(Rival::windowWidth) / Rival::windowHeight;

    void Rival::initialize() {

        // Initialize SDL
        initSDL();

        // Create the Window
        window = std::make_unique<Window>(
                windowWidth, windowHeight, windowTitle);
        window->use();

        // Initialize OpenGL
        initGLEW();
        initGL();

        // Load resources
        res = std::make_unique<Resources>();

        // Load some scenario
        ScenarioReader reader(Resources::mapsDir + "example.sco");
        ScenarioData scenarioData = reader.readScenario();
        ScenarioBuilder scenarioBuilder(scenarioData);
        scenario = scenarioBuilder.build();

        // Create our framebuffer
        gameFbo = std::make_unique<Framebuffer>(
                Framebuffer::generate(
                        framebufferWidth, framebufferHeight, true));

        // Create the FramebufferRenderer
        gameFboRenderer = std::make_unique<FramebufferRenderer>(*gameFbo);
        gameFboRenderer->init();

        // Create the Camera
        const float cameraWidth = RenderUtils::pxToWorld_X(windowWidth);
        camera = std::make_unique<Camera>(
                0.0f, 0.0f, cameraWidth, aspectRatio, *scenario);

        // Create the MousePicker
        mousePicker = std::make_unique<MousePicker>(*camera, scenario);

        // Pick which tile Spritesheet to use based on the map type
        const Spritesheet& tileSpritesheet = scenario->isWilderness()
                ? res->getTileSpritesheet(1)
                : res->getTileSpritesheet(0);

        // Create the TileRenderer
        tileRenderer = std::make_unique<TileRenderer>(
                tileSpritesheet,
                res->getPalette());

        // Hardcode this, for now
        Race race = Race::Human;

        // Create the MapBorderRenderer
        mapBorderRenderer = std::make_unique<MapBorderRenderer>(
                race,
                scenario->getWidth(),
                scenario->getHeight(),
                res->getMapBorderSpritesheet(),
                res->getPalette());

        // Create the UnitRenderer
        unitRenderer = std::make_unique<UnitRenderer>(
                res->getUnitSpritesheets(),
                res->getPalette());

        // Create the BuildingRenderer
        buildingRenderer = std::make_unique<BuildingRenderer>(
                res->getBuildingSpritesheets(),
                res->getPalette());
    }

    void Rival::initSDL() const {

        // This must be called before SDL_Init since we're not using SDL_main
        // as an entry point
        SDL_SetMainReady();

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialise!\n";
            throw std::runtime_error(SDL_GetError());
        }

        // Use OpenGL 3.1 core
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE);
    }

    void Rival::initGLEW() const {

        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();

        if (glewError != GLEW_OK) {
            std::cerr << "Error initialising GLEW:"
                    << glewGetErrorString(glewError) << "\n";
            throw std::runtime_error("Failed to initialise GLEW");
        }
    }

    void Rival::initGL() {

        // Try to enable vsync
        if (SDL_GL_SetSwapInterval(1) < 0) {
            printf("Unable to enable vsync! SDL Error: %s\n", SDL_GetError());
            vsyncEnabled = false;
        }

        // Set clear color
        glClearColor(0.5f, 0.5f, 1.f, 1.f);

        // Enable back-face culling
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);

        // Enable texturing
        glEnable(GL_TEXTURE_2D);

        // Enable alpha blending
        // See also:
        // https://www.khronos.org/opengl/wiki/Transparency_Sorting#Alpha_test
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        initialiseShaders();
    }

    void Rival::start() {

        // Event handler
        SDL_Event e;

        bool exiting = false;
        Uint32 nextUpdateDue = SDL_GetTicks();

        // Game loop
        while (!exiting) {
            Uint32 frameStartTime = SDL_GetTicks();

            // Is the next update due?
            if (vsyncEnabled || nextUpdateDue <= frameStartTime) {

                // Handle events on the queue
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        exiting = true;
                    } else if (e.type == SDL_KEYDOWN) {
                        keyDown(e.key.keysym.sym);
                    } else if (e.type == SDL_MOUSEWHEEL) {
                        mouseWheelMoved(e.wheel);
                    }
                }

                // Update the game logic, as many times as necessary to keep it
                // in-sync with the refresh rate.
                //
                // For example:
                //  - For a 30Hz monitor, this will run twice per render.
                //  - For a 60Hz monitor, this will run once per render.
                //  - For a 120Hz monitor, this will run every other render.
                //
                // If vsync is disabled, this should run once per render.
                while (nextUpdateDue <= frameStartTime) {
                    update();
                    nextUpdateDue += TimerUtils::timeStepMs;
                }

                // Render the game, once per iteration.
                // With vsync enabled, this matches the screen's refresh rate.
                // Otherwise, this matches our target FPS.
                render();

                // Update the window with our newly-rendered game.
                // If vsync is enabled, this will block execution until the
                // next swap interval.
                window->swapBuffers();

            } else {
                // Next update is not yet due.
                // Sleep for the shortest possible time, so as not to risk overshooting!
                SDL_Delay(1);
            }
        }

        // Free resources and exit SDL
        exit();
    }

    void Rival::update() {
        //std::cout << "scenario -> " << scenario->getWidth() << ", " << scenario->getHeight() << std::endl;

        mousePicker->handleMouse();

        //std::cout << mousePicker->getTileX() << ", " << mousePicker->getTileY() << "\n";

        /// \todo Inspect the reason why sprites do not render correctly when this part is omitted
        std::map<int, std::unique_ptr<Building>>& buildings = scenario->getBuildings();
        for (auto const& kv : buildings) {
            const std::unique_ptr<Building>& building = kv.second;
            building->tick();
        }

        std::map<int, std::unique_ptr<Unit>>& units = scenario->getUnits();
        for (auto const& kv : units) {
            const std::unique_ptr<Unit>& unit = kv.second;
            //unit->rotateRight();
            unit->tick();

            // Test for animation subsystem
            static const double randMax = RAND_MAX;
            auto randGen = [&]() {
                double r = static_cast<double>(rand());
                return r / randMax;
            };
            auto r = randGen();
            if (r > 0.999) {
                switch (unit->getType()) {
                    case UnitType::Peasant:
                        r = randGen();
                        if (r <= 1.0 / 6.0) {
                            unit->setAnimation(UnitAnimationType::Standing);
                        } else if (r <= 2.0 / 6.0) {
                            unit->setAnimation(UnitAnimationType::HoldingBag);
                        } else if (r <= 3.0 / 6.0) {
                            unit->setAnimation(UnitAnimationType::Moving);
                        } else if (r <= 4.0 / 6.0) {
                            unit->setAnimation(UnitAnimationType::MovingWithBag);
                        } else if (r <= 5.0 / 6.0) {
                            unit->setAnimation(UnitAnimationType::Attacking);
                        } else {
                            // Currently we can't automatically generate dying animation
                            //   because that animation type does not provide all
                            //   8 directions in our tga files.
                            // We need to modify eg. unit_human_peasant.def
                            //   to repeat 4-direction animations in all
                            //   8 directions
                            //unit->setAnimation( UnitAnimationType::Dying );
                        }
                        break;
                    case UnitType::Bowman:
                    case UnitType::Ballista:
                    case UnitType::FireMaster:
                    case UnitType::Knight:
                    case UnitType::Wizard:
                        r = randGen();
                        if (r <= 1.0 / 3.0) {
                            unit->setAnimation(UnitAnimationType::Standing);
                        } else if (r <= 2.0 / 3.0) {
                            unit->setAnimation(UnitAnimationType::Moving);
                        } else {
                            unit->setAnimation(UnitAnimationType::Attacking);
                        }
                        // Proof of concept for rational animation ticks
                        r = randGen();
                        if (r <= 0.33) {
                            unit->setSpeedCoefficient(1, 2);
                        } else if (r <= 0.67) {
                            unit->setSpeedCoefficient(1, 1);
                        } else {
                            unit->setSpeedCoefficient(2, 1);
                        }
                }
            }
        }
    }

    void Rival::render() {

        // Render to our framebuffer.
        // Here the viewport specifies the region of the framebuffer texture
        // that we render onto, in pixels. We use the camera size here; if the
        // camera is wider, more pixels are visible, and thus we need a larger
        // render target.
        glBindFramebuffer(GL_FRAMEBUFFER, gameFbo->getId());
        int viewportWidth = static_cast<int>(
                RenderUtils::worldToPx_X(camera->getWidth()));
        int viewportHeight = static_cast<int>(
                RenderUtils::worldToPx_Y(camera->getHeight()));
        glViewport(0, 0, viewportWidth, viewportHeight);
        renderGame(viewportWidth, viewportHeight);

        // Render the framebuffer to the screen.
        // Here the viewport specifies the region of the game window that we
        // render onto, in pixels.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, windowWidth, windowHeight);
        renderFramebuffer();
    }

    void Rival::renderGame(int viewportWidth, int viewportHeight) {

        // Clear framebuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth testing so we can rely on z co-ordinates for sprite
        // ordering
        glEnable(GL_DEPTH_TEST);

        // Use indexed texture shader
        glUseProgram(indexedTextureShader.programId);

        // Determine view matrix.
        // OpenGL uses right-handed rule:
        //  - x points right
        //  - y points up
        //  - z points out of the screen
        // The camera co-ordinates are in world units (tiles), but our vertices
        // are positioned using pixels. Therefore we need to convert the camera
        // co-ordinates to pixels, too.
        float cameraX = RenderUtils::worldToPx_X(camera->getX());
        float cameraY = RenderUtils::worldToPx_Y(camera->getY());
        glm::mat4 view = glm::lookAt(
            glm::vec3(cameraX, cameraY, 0),     // camera position
            glm::vec3(cameraX, cameraY, -1),    // look at
            glm::vec3(0, 1, 0)                  // up vector
        );

        // Determine projection matrix.
        // This should match the viewport size exactly, in order to achieve
        // pixel-perfect rendering.
        float left = -viewportWidth / 2.0f;
        float top = -viewportHeight / 2.0f;
        float right = viewportWidth / 2.0f;
        float bottom = viewportHeight / 2.0f;
        float near = 1.0f;
        float far = 1024.0f;
        glm::mat4 projection = glm::ortho(left, right, bottom, top, near, far);

        // Combine matrices
        glm::mat4 viewProjMatrix = projection * view;

        // Set uniform values
        glUniformMatrix4fv(indexedTextureShader.viewProjMatrixUniformLocation,
                1, GL_FALSE, &viewProjMatrix[0][0]);
        glUniform1i(indexedTextureShader.texUnitUniformLocation, 0);
        glUniform1i(indexedTextureShader.paletteTexUnitUniformLocation, 1);

        // Render Tiles
        tileRenderer->render(
                *camera,
                scenario->getTiles(),
                scenario->getWidth(),
                scenario->getHeight());

        // Render Map Borders
        mapBorderRenderer->render();

        // Render Units
        unitRenderer->render(*camera, scenario->getUnits());

        // Render Units
        buildingRenderer->render(*camera, scenario->getBuildings());
    }

    void Rival::renderFramebuffer() {

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Disable depth testing since we are rendering a single texture to the
        // entire screen
        glDisable(GL_DEPTH_TEST);

        // Use screen shader
        glUseProgram(screenShader.programId);

        // Set uniform values
        glUniform1i(screenShader.texUnitUniformLocation, 0);

        // Render framebuffer to screen.
        // At a zoom level of 1, this will result in pixel-perfect rendering.
        // A higher zoom level will result in a smaller sample, which will
        // then be stretched to fill the viewport.
        float srcWidth = windowWidth / camera->getZoom();
        float srcHeight = windowHeight / camera->getZoom();
        gameFboRenderer->render(srcWidth, srcHeight);
    }

    void Rival::exit() {

        // Destroy window
        window.reset();

        // Quit SDL
        SDL_Quit();
    }

    void Rival::keyDown(const SDL_Keycode keyCode) const {
        switch (keyCode) {
        case SDLK_UP:
            camera->translate(0.0f, -0.5f);
            break;

        case SDLK_DOWN:
            camera->translate(0.0f, 0.5f);
            break;

        case SDLK_LEFT:
            camera->translate(-0.5f, 0.0f);
            break;

        case SDLK_RIGHT:
            camera->translate(0.5f, 0.0f);
            break;

        default:
            break;
        }
    }

    void Rival::mouseWheelMoved(const SDL_MouseWheelEvent evt) const {

        // Determine the normalized scroll amount
        int dy = evt.y;
        if (evt.direction == SDL_MOUSEWHEEL_FLIPPED) {
            dy *= -1;
        }

        int mouseX;
        int mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        const float mouseXwu(RenderUtils::pxToWorld_X(mouseX));
        const float mouseYwu(RenderUtils::pxToWorld_Y(mouseY));
        const float midXwu(RenderUtils::pxToWorld_X(0.5 * windowWidth));
        const float midYwu(RenderUtils::pxToWorld_Y(0.5 * windowHeight));
        const float vecXwu(mouseXwu - midXwu);
        const float vecYwu(mouseYwu - midYwu);

        if (dy > 0) {
            camera->modZoom(Camera::zoomInterval);
        } else if (dy < 0) {
            camera->modZoom(-Camera::zoomInterval);
        }
        camera->translate(0.25 * vecXwu, 0.25 * vecYwu);
    }

}
