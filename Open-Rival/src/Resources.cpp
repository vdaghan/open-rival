#include "pch.h"
#include "Resources.h"

#include "RenderUtils.h"

namespace Rival {

    const std::string Resources::mapsDir = "res\\maps\\";
    const std::string Resources::txDir = "res\\textures\\";

    Resources::Resources() :
            textures(loadTextures()),
            paletteTexture(initPaletteTexture()),
            unitSpritesheets(initUnitSpritesheets()),
            buildingSpritesheets(initBuildingSpritesheets()),
            tileSpritesheets(initTileSpritesheets()),
            mapBorderSpritesheet(initMapBorderSpritesheet()) {}

    Resources::~Resources() {
        // Delete Textures
        for (const Texture& texture : textures) {
            const GLuint texId = texture.getId();
            glDeleteTextures(1, &texId);
        }
    }

    std::vector<Texture> Resources::loadTextures() {

        std::vector<Texture> textures;
        textures.reserve(numTextures);

        std::vector<std::string> textureNames = {
            // Units - Human
            "unit_human_ballista.tga",
            "unit_human_battleship.tga",
            "unit_human_bowman.tga",
            "unit_human_chariot_of_war.tga",
            "unit_human_fire_master.tga",
            "unit_human_knight.tga",
            "unit_human_light_cavalry.tga",
            "unit_human_peasant.tga",
            "unit_human_pegas_rider.tga",
            "unit_human_priest.tga",
            "unit_human_sea_barge.tga",
            "unit_human_thief.tga",
            "unit_human_wizard.tga",
            "unit_human_zeppelin.tga",

            // Units - Greenskin
            "unit_greenskin_balloon.tga",
            "unit_greenskin_catapult.tga",
            "unit_greenskin_gnome_boomer.tga",
            "unit_greenskin_horde_rider.tga",
            "unit_greenskin_landing_craft.tga",
            "unit_greenskin_necromancer.tga",
            "unit_greenskin_priest_of_doom.tga",
            "unit_greenskin_rock_thrower.tga",
            "unit_greenskin_rogue.tga",
            "unit_greenskin_serf.tga",
            "unit_greenskin_storm_trooper.tga",
            "unit_greenskin_troll_galley.tga",
            "unit_greenskin_warbat.tga",
            "unit_greenskin_warlord.tga",

            // Units - Elf
            "unit_elf_archer.tga",
            "unit_elf_arquebusier.tga",
            "unit_elf_bark.tga",
            "unit_elf_bombard.tga",
            "unit_elf_centaur.tga",
            "unit_elf_druid.tga",
            "unit_elf_dwarf_miner.tga",
            "unit_elf_enchanter.tga",
            "unit_elf_mage.tga",
            "unit_elf_magic_chopper.tga",
            "unit_elf_scout.tga",
            "unit_elf_sky_rider.tga",
            "unit_elf_warship.tga",
            "unit_elf_yeoman.tga",

            // Units - Monsters
            "unit_monster_devil.tga",
            "unit_monster_dragon.tga",
            "unit_monster_golem.tga",
            "unit_monster_gryphon.tga",
            "unit_monster_hydra.tga",
            "unit_monster_sea_monster.tga",
            "unit_monster_skeleton.tga",
            "unit_monster_snake.tga",

            // Tiles
            "tiles_meadow.tga",
            "tiles_wilderness.tga",
            "tiles_fog.tga",

            // UI
            "ui_cursor_select.tga",
            "ui_map_border.tga",

            // Buildings
            "buildings_elf.tga",
            "buildings_greenskin.tga",
            "buildings_human.tga"
        };

        for (auto const& textureName : textureNames) {
            textures.push_back(Texture::loadTexture(txDir + textureName));
        }

        return textures;
    }

    Texture Resources::initPaletteTexture() {
        return Palette::createPaletteTexture();
    }

    std::map<BuildingType, Spritesheet> Resources::initBuildingSpritesheets() {

        std::map<BuildingType, Spritesheet> buildingSpritesheets;
        int nextIndex = txIndexBuildings;

        auto createSpritesheets = [&](int first, int last) {
            for (auto it{ first }; it <= last; ++it) {
                buildingSpritesheets.emplace(std::piecewise_construct,
                    std::forward_as_tuple(static_cast<BuildingType>(it)),
                    std::forward_as_tuple(
                        textures.at(nextIndex),
                        RenderUtils::buildingWidthPx,
                        RenderUtils::buildingHeightPx));
            }
        };

        createSpritesheets(firstElfBuildingType, lastElfBuildingType);
        ++nextIndex;
        createSpritesheets(firstGreenskinBuildingType, lastGreenskinBuildingType);
        ++nextIndex;
        createSpritesheets(firstHumanBuildingType, lastHumanBuildingType);

        return buildingSpritesheets;
    }

    std::map<UnitType, Spritesheet> Resources::initUnitSpritesheets() {

        std::map<UnitType, Spritesheet> unitSpritesheets;
        int nextIndex = txIndexUnits;

        for (auto it{ firstUnitType }; it <= lastUnitType; ++it) {
            unitSpritesheets.emplace(std::piecewise_construct,
                std::forward_as_tuple(static_cast<UnitType>(it)),
                std::forward_as_tuple(
                    textures.at(nextIndex),
                    RenderUtils::unitWidthPx,
                    RenderUtils::unitHeightPx));
            nextIndex++;
        }

        return unitSpritesheets;
    }

    std::map<int, Spritesheet> Resources::initTileSpritesheets() {

        std::map<int, Spritesheet> tileSpritesheets;
        int nextIndex = txIndexTiles;

        // 0 = Meadow
        // 1 = Wilderness
        // 2 = Fog
        for (int i = 0; i < 3; i++) {
            tileSpritesheets.emplace(std::piecewise_construct,
                std::forward_as_tuple(i),
                std::forward_as_tuple(
                    textures.at(nextIndex),
                    RenderUtils::tileSpriteWidthPx,
                    RenderUtils::tileSpriteHeightPx));
            nextIndex++;
        }

        return tileSpritesheets;
    }

    Spritesheet Resources::initMapBorderSpritesheet() {
        Spritesheet mapBorderSpritesheet(
                textures.at(txIndexUi + 1),
                RenderUtils::tileSpriteWidthPx,
                RenderUtils::tileSpriteHeightPx);
        return mapBorderSpritesheet;
    }

    const Spritesheet& Resources::getTileSpritesheet(int index) const {
        return tileSpritesheets.at(index);
    }

    const std::map<UnitType, Spritesheet>& Resources::getUnitSpritesheets() const {
        return unitSpritesheets;
    }

    const std::map<BuildingType, Spritesheet>& Resources::getBuildingSpritesheets() const {
        return buildingSpritesheets;
    }

    const Spritesheet& Resources::getMapBorderSpritesheet() const {
        return mapBorderSpritesheet;
    }

    const Texture& Resources::getPalette() const {
        return paletteTexture;
    }

}
