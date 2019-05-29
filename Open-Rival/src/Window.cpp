#include "pch.h"
#include "Window.h"

#include <iostream>

#include <SDL.h>

namespace Rival {

    Window::Window(const int width, const int height, const char* title) :
        width(width), height(height), title(title) {}

    void Window::create() {

        windowRef = SDL_CreateWindow(title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        if (windowRef == NULL) {
            std::cerr << "Window could not be created!" << std::endl;
            throw std::runtime_error(SDL_GetError());
        }
    }

    void Window::destroy() {
        SDL_DestroyWindow(windowRef);
        windowRef = NULL;
    }

    void Window::use() {

        glContext = SDL_GL_CreateContext(windowRef);

        if (glContext == NULL) {
            std::cerr << "OpenGL context could not be created!" << std::endl;
            throw std::runtime_error(SDL_GetError());
        }
    }

    void Window::swapBuffers() const {
        SDL_GL_SwapWindow(windowRef);
    }

}
