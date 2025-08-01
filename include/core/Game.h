#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "world/WorldConfig.h"
#include "world/Block.h"

class Window;
class ChunkRenderer;
class CloudRenderer;
class SkyboxRenderer;
class SunRenderer;
class Camera;
class World;
class LoadingScreen;
class Crosshair;
class BlockOutline;
class Hotbar;
class RayVisualization;

class ItemEntity;
struct GLFWwindow;

class Game {
public:
    Game();
    ~Game();
    
    void run();
    
private:
    void initialize();
    void update(float deltaTime);
    void render();
    void cleanup();
    void processInput(float deltaTime);
    void handleBlockBreaking(); // Handle block breaking when left mouse is clicked
    void handleBlockPlacement(); // Handle block placement when right mouse is clicked
    
    // Item entity management
    void updateItemEntities(float deltaTime);
    void renderItemEntities();
    void spawnItemEntity(const glm::vec3& position, BlockType blockType);
    void checkItemCollection();
    
    std::unique_ptr<Window> m_window;
    std::unique_ptr<ChunkRenderer> m_chunkRenderer;
    std::unique_ptr<CloudRenderer> m_cloudRenderer;
    std::unique_ptr<SkyboxRenderer> m_skyboxRenderer;
    std::unique_ptr<SunRenderer> m_sunRenderer;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<World> m_world;
    std::unique_ptr<LoadingScreen> m_loadingScreen;
    std::unique_ptr<Crosshair> m_crosshair;
    std::unique_ptr<BlockOutline> m_blockOutline;
    std::unique_ptr<Hotbar> m_hotbar;
    std::unique_ptr<RayVisualization> m_rayVisualization;

    std::vector<std::unique_ptr<ItemEntity>> m_itemEntities;
    bool m_running;
    float m_lastFrameTime;
    
    // FPS tracking
    float m_frameCount;
    float m_fpsTimer;
    float m_currentFPS;
    
    // Loading state
    bool m_isLoading;
    float m_loadingStartTime;
    
    // Mouse handling
    bool m_firstMouse;
    float m_lastX, m_lastY;
    
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // 📏 Window resize handling for scalability
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
