#ifndef BUILDING_H
#define BUILDING_H

#include "BuildingAnimation.h"
#include "Spritesheet.h"

namespace Rival {

    class Building {

    public:

        // Size of a Building, in world units
        static const int width = 4;
        static const int height = 4;

        Building(BuildingType type);

        void addedToWorld(int newId,
                          int newPlayer,
                          int newX,
                          int newY,
                          WallVariant newWallVariant);

        int getCurrentSpriteIndex() const;

        const BuildingType getType() const;

        void setAnimation(BuildingAnimationType buildingAnimationType);

        const bool isDeleted() const;

        void markForDeletion();

        const int getId() const;

        int getX() const;

        int getY() const;

        void tick();

    private:

        int id;

        bool deleted;

        BuildingType type;

        WallVariant wallVariant;

        BuildingAnimation animation;

        int player;

        /**
         * x co-ordinate of the Tile the Building is occupying.
         */
        int x;

        /**
         * y co-ordinate of the Tile the Building is occupying.
         */
        int y;

    };

}

#endif // BUILDING_H
