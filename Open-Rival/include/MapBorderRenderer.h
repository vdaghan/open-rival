#ifndef MAP_BORDER_RENDERER_H
#define MAP_BORDER_RENDERER_H

#include <gl/glew.h>
#include <vector>

#include "Camera.h"
#include "Race.h"
#include "RenderUtils.h"
#include "Spritesheet.h"
#include "SpriteRenderable.h"
#include "Texture.h"

namespace Rival {

    class MapBorderRenderer {

    public:

        MapBorderRenderer(
                Race race,
                int mapWidth,
                int mapHeight,
                const Spritesheet& spritesheet,
                const Texture& paletteTexture);

        void createLeftEdge(
                std::vector<GLfloat>& positions,
                std::vector<GLfloat>& texCoords,
                int mapHeight);

        void createTopEdge(
                std::vector<GLfloat>& positions,
                std::vector<GLfloat>& texCoords,
                int mapWidth);

        void createRightEdge(
                std::vector<GLfloat>& positions,
                std::vector<GLfloat>& texCoords,
                int mapWidth,
                int mapHeight);

        void createBottomEdge(
                std::vector<GLfloat>& positions,
                std::vector<GLfloat>& texCoords,
                int mapWidth,
                int mapHeight);

        void createCorners(
                std::vector<GLfloat>& positions,
                std::vector<GLfloat>& texCoords,
                int mapWidth,
                int mapHeight);

        void addDataToBuffers(
                std::vector<GLfloat>& positions,
                std::vector<GLfloat>& texCoords,
                int txIndex,
                float tileX,
                float tileY);

        void render();

    private:

        const Texture& paletteTexture;

        // The maximum number of border segments we can render
        int maxSegmentsToRender;

        SpriteRenderable renderable;

        int numSegments = 0;

    };

}

#endif // MAP_BORDER_RENDERER_H
