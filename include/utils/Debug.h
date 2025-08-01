#pragma once

// Debug configuration - only enable in debug builds
#ifdef DEBUG
    #define DEBUG_OUTLINE_POSITIONING 1
    #define DEBUG_RAYCAST_VERBOSE 1
    #define DEBUG_GAME_VERBOSE 1
#else
    #define DEBUG_OUTLINE_POSITIONING 0
    #define DEBUG_RAYCAST_VERBOSE 0  
    #define DEBUG_GAME_VERBOSE 0
#endif

// Debug macros that compile to nothing in release builds
#if DEBUG_OUTLINE_POSITIONING
    #define DEBUG_OUTLINE(msg) std::cout << "[DEBUG OUTLINE] " << msg << std::endl
#else
    #define DEBUG_OUTLINE(msg) ((void)0)
#endif

#if DEBUG_GAME_VERBOSE
    #define DEBUG_GAME_VERBOSE_PRINT(msg) std::cout << "[DEBUG GAME] " << msg << std::endl
#else
    #define DEBUG_GAME_VERBOSE_PRINT(msg) ((void)0)
#endif
