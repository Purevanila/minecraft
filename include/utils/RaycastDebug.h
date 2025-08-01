#pragma once

// Set this to true to enable raycast debugging output
// Set to false to disable all debugging prints
#define RAYCAST_DEBUG_ENABLED false

// Debug print macro - only prints if debugging is enabled
#if RAYCAST_DEBUG_ENABLED
    #include <iostream>
    #define RAYCAST_DEBUG(msg) std::cout << "[DEBUG RAYCAST] " << msg << std::endl
    #define GAME_DEBUG(msg) std::cout << "[DEBUG GAME] " << msg << std::endl
#else
    #define RAYCAST_DEBUG(msg) ((void)0)
    #define GAME_DEBUG(msg) ((void)0)
#endif
