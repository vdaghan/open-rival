#include "pch.h"

#define WIN32_LEAN_AND_MEAN

#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <windows.h>

#include "audio-extractor.h"
#include "image-extractor.h"
#include "registry.h"
#include "setup-utils.h"
#include "texture-builder.h"

namespace fs = std::filesystem;

void copyVideos(std::wstring videoDir, std::string outputDir) {
    for (const fs::directory_entry& entry : fs::directory_iterator(videoDir)) {
        const fs::path path = entry.path();
        std::filesystem::copy(path, outputDir + "\\" + path.filename().string(),
                               std::filesystem::copy_options::update_existing );
    }
}

/**
 * Setup program to prepare the game's assets.
 *
 * This is pretty rough around the edges, and would benefit from better error
 * handling, more thorough logging, and consistent handling of paths.
 *
 * The code is very non-portable.
 */
int main( int argc, char* argv[] ) {

    std::wstring gameDir;
    bool findDirectoryFromRegistry = true;
    for ( int count = 1; count < argc; ++count ) {
        if ( "-h" == argv[count] ) {
            std::cout << "-d {directory}: loads all files from {directory}" << '\n';
            return 0;
        }
        if ( 0 == strcmp( "-d", argv[count] ) && count + 1 <= argc ) {
            std::string tmpStr( argv[count + 1] );
            gameDir = std::wstring( tmpStr.begin(), tmpStr.end() );
            findDirectoryFromRegistry = false;
            std::wcout << "Reading files from directory " << gameDir << '\n';
            ++count;
        }
    }

    if ( findDirectoryFromRegistry ) {
        std::wcout << "Reading files from registry." << '\n';
        // Read original game directory from registry
        try {
            gameDir = Registry::RegGetString(
                HKEY_CURRENT_USER,
                L"Software\\Titus Games\\Rival Realms",
                L"Game Directory" );
            std::wcout << "Found game at: " << gameDir << "\n";
        } catch ( const Registry::RegistryError& e ) {
            std::cerr << "Failed to find registry entry:\n"
                << "HKEY_CURRENT_USER\\Software\\Titus Games\\Rival Realms\n"
                << "Error code: " << e.errorCode() << "\n";
            return -1;
        }
    }

    // Create the output directories
    std::wcout << "Creating output directories\n";
    if (!create_directory("setup\\images")) {
        std::cerr << "Could not create \"images\" directory\n";
        return -1;
    }
    if (!create_directory("res\\sound")) {
        std::cerr << "Could not create \"sound\" directory\n";
        return -1;
    }
    if (!create_directory("res\\textures")) {
        std::cerr << "Could not create \"textures\" directory\n";
        return -1;
    }
    if (!create_directory("res\\video")) {
        std::cerr << "Could not create \"video\" directory\n";
        return -1;
    }

    // Extract sounds
    try {
        std::wcout << "Extracting audio\n";
        extractAudio(gameDir + L"\\DATA\\SOUNDS.DAT", "res\\sound");
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to extract sounds: " << e.what() << "\n";
        return -1;
    }

    // Extract images
    try {
        std::wcout << "Extracting images\n";
        extractImages(gameDir + L"\\DATA\\IMAGES.DAT", "setup\\images");
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to extract images: " << e.what() << "\n";
        return -1;
    }

    // Build textures
    try {
        std::wcout << "Building textures\n";
        buildTextures("setup\\definitions", "setup\\images", "res\\textures");
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to build textures: " << e.what() << "\n";
        return -1;
    }

    // Copy videos
    try {
        copyVideos(gameDir + L"\\MOVIES", "res\\video");
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to copy videos: " << e.what() << "\n";
        return -1;
    }
}
