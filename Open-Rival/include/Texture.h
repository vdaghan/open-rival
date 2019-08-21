#ifndef TEXTURE_H
#define TEXTURE_H

#include <gl/glew.h>

namespace Rival {

    class Texture {

    public:

        Texture(const GLuint id, const int width, const int height);

        GLuint getId() const;

        int getWidth() const;

        int getHeight() const;

    private:

        const GLuint id;

        const int width;

        const int height;

    };

}

#endif // TEXTURE_H
