#ifndef UNIT_H
#define UNIT_H

#include "Spritesheet.h"

namespace Rival {

    class Unit {

    public:

        enum Type {

            // Human
            Peasant,
            Bowman,
            LightCavalry,
            Knight,
            FireMaster,
            Thief,
            Ballista,
            ChariotOfWar,
            Wizard,
            Priest,
            SeaBarge,
            Battleship,
            PegasRider,
            Zeppelin,

            // Greenskin
            Serf,
            RockThrower,
            HordeRider,
            Warlord,
            GnomeBoomer,
            Rogue,
            Catapult,
            StormTrooper,
            PriestOfDoom,
            Necromancer,
            LandingCraft,
            TrollGalley,
            Warbat,
            Balloon,

            // Elf
            Yeoman,
            Archer,
            Druid,
            Centaur,
            DwarfMiner,
            Scout,
            Bombard,
            Arquebusier,
            Mage,
            Enchanter,
            Bark,
            Warship,
            SkyRider,
            MagicChopper,

            // Monsters
            Devil,
            Dragon,
            Golem,
            Gryphon,
            Hydra,
            SeaMonster,
            Skeleton,
            Snake
        };

        enum Facing {
            South,
            SouthWest,
            West,
            NorthWest,
            North,
            NorthEast,
            East,
            SouthEast
        };

        // Size of a Unit, in world units
        static const int width = 1;
        static const int height = 1;

        Unit(Type type,
                int player,
                int x,
                int y,
                Facing facing);

        void addedToScene(int newId);

        const Type getType() const;

        const Facing getFacing() const;

        void setFacing(Facing newFacing);

        void rotateLeft();

        void rotateRight();

        const bool isDeleted();

        void markForDeletion();

        const int getId();

        int getX() const;

        int getY() const;

    private:

        int id = -1;

        bool deleted = false;

        Type type;

        int player;

        /**
         * x co-ordinate of the Tile the Unit is occupying.
         */
        int x;

        /**
         * y co-ordinate of the Tile the Unit is occupying.
         */
        int y;

        Facing facing = Facing::South;

    };

}

#endif // UNIT_H
