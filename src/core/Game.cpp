#include "core/Game.h"
#include "engine/graphics/Window.h"
#include "engine/graphics/ChunkRenderer.h"
#include "engine/graphics/CloudRenderer.h"
#include "engine/graphics/SkyboxRenderer.h"
#include "engine/graphics/Camera.h"
#include "engine/graphics/OpenGL.h"
#include "engine/AssetManager.h"
#include "world/Block.h"
#include "world/BlockDefinition.h"
#include "utils/RaycastUtil.h"
#include <cmath>
#include "world/World.h"
#include "world/features/TreeFeature.h"
#include "world/WorldConfig.h"
#include "ui/LoadingScreen.h"
#include "ui/Crosshair.h"
#include "ui/BlockOutline.h"
#include "ui/Hotbar.h"

#include "entities/ItemEntity.h"
#include <algorithm>

// External declaration for global world config
extern WorldConfig g_worldConfig;
#include <GLFW/glfw3.h>
#include <iostream>

Game::Game() : m_running(false), m_lastFrameTime(0.0f), m_frameCount(0.0f), m_fpsTimer(0.0f), m_currentFPS(0.0f), m_isLoading(false), m_loadingStartTime(0.0f) {
    
}

Game::~Game() {
    // Explicitly reset in reverse order of creation
    m_loadingScreen.reset();
    m_hotbar.reset();
    m_blockOutline.reset();
    m_crosshair.reset();
    m_world.reset();
    m_chunkRenderer.reset();
    m_skyboxRenderer.reset();
    m_cloudRenderer.reset();
    m_camera.reset();
    m_window.reset();
    cleanup();
}

void Game::run() {
    initialize();
    
    m_running = true;
    while (m_running && !m_window->shouldClose()) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
        
        // Poll for window events
        m_window->pollEvents();
        
        update(deltaTime);
        render();
        
        // Swap front and back buffers
        m_window->swapBuffers();
    }
}

void Game::initialize() {
    // Load world configuration
    if (!g_worldConfig.loadFromFile("world_config.ini")) {
        // Config file not found, using defaults and creating new file
        g_worldConfig.resetToDefaults();
        g_worldConfig.saveToFile("world_config.ini");
    }
    
    // Create window (1280x720 is a good default size)
    m_window = std::make_unique<Window>(1280, 720, "Minecraft Clone");
    
    // Set up window resize callback
    int width, height;
    m_window->getFramebufferSize(width, height);
    glViewport(0, 0, width, height);
    
    // Set up resize callback for automatic scaling
    glfwSetWindowUserPointer(m_window->getHandle(), this);
    m_window->setFramebufferSizeCallback(framebufferSizeCallback);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Enable face culling for performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Set clear color to black (skybox will provide the sky)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Initialize improved systems
    
    // Initialize Block Definition Registry
    BlockDefinitionRegistry::getInstance().initializeDefaultBlocks();
    
    // Initialize Asset Manager and preload assets
    AssetManager::getInstance().preloadAssets();
    
    // Initialize legacy block registry for compatibility
    BlockRegistry::getInstance().initializeDefaultBlocks();
    
    // Create camera - position it higher to see more of the world
    m_camera = std::make_unique<Camera>(glm::vec3(0.0f, 20.0f, 0.0f));
    
    // Create improved chunk renderer that supports leaves
    m_chunkRenderer = std::make_unique<ChunkRenderer>();
    if (!m_chunkRenderer->initialize()) {
        throw std::runtime_error("Failed to initialize chunk renderer");
    }
    
    // Create the infinite world
    m_world = std::make_unique<World>();
    m_world->setRenderDistance(g_worldConfig.rendering.renderDistance);  // Use config value
    
    // Create debug overlay
    // Create loading screen
    m_loadingScreen = std::make_unique<LoadingScreen>();
    if (!m_loadingScreen->initialize()) {
        throw std::runtime_error("Failed to initialize loading screen");
    }

    // Create crosshair
    m_crosshair = std::make_unique<Crosshair>();
    if (!m_crosshair->initialize()) {
        throw std::runtime_error("Failed to initialize crosshair");
    }

    // Create block outline
    m_blockOutline = std::make_unique<BlockOutline>();
    if (!m_blockOutline->initialize()) {
        throw std::runtime_error("Failed to initialize block outline");
    }

    // Create hotbar
    m_hotbar = std::make_unique<Hotbar>();
    if (!m_hotbar->initialize()) {
        throw std::runtime_error("Failed to initialize hotbar");
    }


    /*
    m_inventory = std::make_unique<Inventory>();
    if (!m_inventory->initialize()) {
        throw std::runtime_error("Failed to initialize inventory");
    }
    
    // Connect inventory to hotbar for integration
    if (m_hotbar) {
        m_inventory->setHotbar(m_hotbar.get());
    }
    */

    // Create cloud renderer
    m_cloudRenderer = std::make_unique<CloudRenderer>();
    if (!m_cloudRenderer->initialize()) {
        throw std::runtime_error("Failed to initialize cloud renderer");
    }
    
    // Configure cloud renderer with settings from config
    m_cloudRenderer->setCloudHeight(g_worldConfig.clouds.height);
    m_cloudRenderer->setCloudSpeed(g_worldConfig.clouds.speed);
    m_cloudRenderer->setCloudDensity(g_worldConfig.clouds.density);

    // Create skybox renderer
    m_skyboxRenderer = std::make_unique<SkyboxRenderer>();
    if (!m_skyboxRenderer->initialize()) {
        throw std::runtime_error("Failed to initialize skybox renderer");
    }

    // Record loading start time
    m_loadingStartTime = glfwGetTime();    // Setup mouse input
    m_window->enableMouseCapture();
    m_window->setMouseCallback(mouseCallback);
    m_window->setMouseButtonCallback(mouseButtonCallback);
    glfwSetScrollCallback(m_window->getHandle(), scrollCallback);
    
    // Game initialization complete
}

void Game::update(float deltaTime) {
    // Update FPS counter
    m_frameCount++;
    m_fpsTimer += deltaTime;
    
    if (m_fpsTimer >= 1.0f) {  // Update FPS every second
        m_currentFPS = m_frameCount / m_fpsTimer;
        m_frameCount = 0.0f;
        m_fpsTimer = 0.0f;
    }
    
    // Process input
    processInput(deltaTime);
    
    // Update world based on camera position
    if (m_world && m_camera) {
        m_world->update(m_camera->getPosition());
    }
    
    // Update clouds
    if (m_cloudRenderer) {
        m_cloudRenderer->update(deltaTime);
    }

    // Update block outline
    if (m_blockOutline && m_camera && m_world) {
        m_blockOutline->updateTargetBlock(*m_camera, *m_world, 5.0f);  // Explicitly pass max distance
    }
    
    // Update item entities
    updateItemEntities(deltaTime);
    
    // Check for item collection
    checkItemCollection();
}

void Game::render() {
    // Clear the screen - we'll let the skybox provide the background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Calculate how the camera sees the world
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = m_camera->getProjectionMatrix(m_window->getAspectRatio());
    
    // Render skybox first (it should be rendered behind everything)
    if (m_skyboxRenderer) {
        m_skyboxRenderer->render(view, projection, static_cast<float>(glfwGetTime()));
    }
    
    // No lighting setup needed - we want flat, bright rendering!
    
    // Draw the beautiful world
    if (m_world && m_chunkRenderer) {
        m_world->render(m_chunkRenderer.get(), view, projection);
    }
    
    // Render clouds
    if (m_cloudRenderer && g_worldConfig.clouds.enabled) {
        m_cloudRenderer->render(view, projection, static_cast<float>(glfwGetTime()), m_camera->getPosition());
    }

    // Render crosshair
    if (m_crosshair) {
        int width, height;
        m_window->getFramebufferSize(width, height);
        m_crosshair->render(width, height);
    }

    // Render hotbar
    if (m_hotbar) {
        int width, height;
        m_window->getFramebufferSize(width, height);
        m_hotbar->render(width, height);
    }


    // Currently commented out to prevent display issues
    /*
    if (m_inventory) {
        int width, height;
        m_window->getFramebufferSize(width, height);
        m_inventory->render(width, height);
    }
    */

    // Render block outline
    if (m_blockOutline) {
        m_blockOutline->render(view, projection);
    }
    
    // Render item entities
    renderItemEntities();
}

void Game::cleanup() {
    m_world.reset();
    glfwTerminate();
}

void Game::processInput(float deltaTime) {
    GLFWwindow* window = m_window->getHandle();
    
    // Toggle flying mode with F key
    static bool fKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressed) {
        if (m_camera) {
            bool newFlying = !m_camera->isFlying();
            m_camera->setFlying(newFlying);
            // Flying mode toggled (remove debug output)
        }
        fKeyPressed = true;
    } else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        fKeyPressed = false;
    }
    
    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_camera->processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_camera->processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_camera->processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_camera->processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        m_camera->processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_camera->processKeyboard(DOWN, deltaTime);
        
        // Hotbar slot selection (1-0 keys)
    if (m_hotbar) {
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { m_hotbar->setSelectedSlot(0); /* m_inventory->setSelectedSlot(0); */ }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { m_hotbar->setSelectedSlot(1); /* m_inventory->setSelectedSlot(1); */ }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { m_hotbar->setSelectedSlot(2); /* m_inventory->setSelectedSlot(2); */ }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { m_hotbar->setSelectedSlot(3); /* m_inventory->setSelectedSlot(3); */ }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { m_hotbar->setSelectedSlot(4); /* m_inventory->setSelectedSlot(4); */ }
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) { m_hotbar->setSelectedSlot(5); /* m_inventory->setSelectedSlot(5); */ }
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) { m_hotbar->setSelectedSlot(6); /* m_inventory->setSelectedSlot(6); */ }
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) { m_hotbar->setSelectedSlot(7); /* m_inventory->setSelectedSlot(7); */ }
        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) { m_hotbar->setSelectedSlot(8); }
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) { m_hotbar->setSelectedSlot(9); }
    }
    

    /*
    if (m_inventory) {
        static bool eKeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !eKeyPressed) {
            m_inventory->toggleInventory();
            eKeyPressed = true;
        } else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
            eKeyPressed = false;
        }
    }
    */
        
    // Close window on ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void Game::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    
    if (game->m_firstMouse) {
        game->m_lastX = xpos;
        game->m_lastY = ypos;
        game->m_firstMouse = false;
    }
    
    float xoffset = xpos - game->m_lastX;
    float yoffset = game->m_lastY - ypos; // Reversed since y-coordinates go from bottom to top
    
    game->m_lastX = xpos;
    game->m_lastY = ypos;
    
    game->m_camera->processMouseMovement(xoffset, yoffset);
}

void Game::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    
    // Handle left mouse button for block breaking
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        game->handleBlockBreaking();
    }
    
    // Handle right mouse button for block placement
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        game->handleBlockPlacement();
    }
}

void Game::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    
    // Check if camera is in flying mode for speed control
    if (game->m_camera && game->m_camera->isFlying()) {
        // Use scroll for flying speed control
        game->m_camera->processMouseScroll(static_cast<float>(yoffset));
    } else if (game->m_hotbar) {
        // Use scroll for hotbar selection when not flying
        game->m_hotbar->handleScrollInput(yoffset);
    }
}

//Handle window resize for scalability
void Game::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Update OpenGL viewport to match new window size
    glViewport(0, 0, width, height);
    
    // Window resized (debug output removed)
}

// In Game::handleBlockBreaking()

void Game::handleBlockBreaking() {
    if (!m_camera || !m_world) {
        return;
    }

    // Use efficient shared raycast utility
    glm::vec3 rayStart = m_camera->getPosition();
    glm::vec3 rayDirection = m_camera->getFront();
    
    // Add a small offset to the ray start to improve accuracy
    rayStart += rayDirection * 0.1f;
    
    auto result = RaycastUtil::raycast(rayStart, rayDirection, *m_world, 5.0f);

    if (result.hit) {
        BlockType blockType = m_world->getBlockType(result.blockPos);

        if (blockType != BlockType::AIR) {
            m_world->setBlock(result.blockPos.x, result.blockPos.y, result.blockPos.z, BlockType::AIR);
            
            glm::vec3 itemPosition = glm::vec3(result.blockPos) + 0.5f;
            spawnItemEntity(itemPosition, blockType);
            
            std::cout << "Block broken at (" << result.blockPos.x << ", " << result.blockPos.y << ", " << result.blockPos.z << ")" << std::endl;
        }
    }
}


// In Game::handleBlockPlacement()
void Game::handleBlockPlacement() {
    // If you re-enable this function, you MUST update its raycast call as well.
    /*
    if (!m_camera || !m_world || !m_inventory) return;
    
    // ... get selected block ...
    
    glm::vec3 rayStart = m_camera->getPosition();
    glm::vec3 rayDirection = m_camera->getFront();
    rayStart += rayDirection * 0.1f;

    auto result = RaycastUtil::raycast(rayStart, rayDirection, *m_world, 5.0f);

    if (result.hit) {
        // ... rest of placement logic ...
    }
    */
}

void Game::updateItemEntities(float deltaTime) {
    // Update all item entities
    for (auto& itemEntity : m_itemEntities) {
        if (itemEntity && !itemEntity->isCollected()) {
            itemEntity->update(deltaTime);
        }
    }
    
    // Remove collected item entities
    m_itemEntities.erase(
        std::remove_if(m_itemEntities.begin(), m_itemEntities.end(),
            [](const std::unique_ptr<ItemEntity>& item) {
                return item->isCollected();
            }),
        m_itemEntities.end()
    );
}

void Game::renderItemEntities() {
    if (!m_camera) return;
    
    // Set up view and projection matrices for 3D rendering
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = m_camera->getProjectionMatrix(m_window->getAspectRatio());
    
    // Get basic shader for item rendering
    auto& assetManager = AssetManager::getInstance();
    auto shader = assetManager.loadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    
    if (shader) {
        shader->use();
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);
        
        // Render all item entities
        for (auto& itemEntity : m_itemEntities) {
            if (itemEntity && !itemEntity->isCollected()) {
                itemEntity->render();
            }
        }
    }
}

void Game::spawnItemEntity(const glm::vec3& position, BlockType blockType) {
    std::cout << "Spawning item entity at (" << position.x << ", " << position.y << ", " << position.z << ") for block type " << static_cast<int>(blockType) << std::endl;
    auto itemEntity = std::make_unique<ItemEntity>(position, blockType, m_world.get());
    m_itemEntities.push_back(std::move(itemEntity));
    std::cout << "Item entity spawned! Total items: " << m_itemEntities.size() << std::endl;
}

void Game::checkItemCollection() {
    if (!m_camera || !m_hotbar) return;
    
    glm::vec3 playerPosition = m_camera->getPosition();
    
    for (auto& itemEntity : m_itemEntities) {
        if (itemEntity && itemEntity->canBeCollected() && !itemEntity->isCollected()) {
            float distance = glm::distance(playerPosition, itemEntity->getPosition());
            
            if (distance <= itemEntity->getCollectionRadius()) {
                // Add item to hotbar inventory using new stacking system
                BlockType blockType = itemEntity->getBlockType();
                int remainingItems = m_hotbar->addItem(blockType, 1);
                
                if (remainingItems == 0) {
                    // Item was successfully added
                    itemEntity->setCollected();
                    std::cout << "Collected " << static_cast<int>(blockType) << " and added to hotbar!" << std::endl;
                } else {
                    // Item couldn't be added (hotbar full)
                    std::cout << "Hotbar is full! Cannot collect " << static_cast<int>(blockType) << std::endl;
                }
            }
        }
    }
}
