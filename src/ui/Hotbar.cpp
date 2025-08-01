#include "ui/Hotbar.h"
#include "engine/AssetManager.h"
#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

Hotbar::Hotbar() 
    : m_VAO(0), m_VBO(0), m_EBO(0), m_selectedSlot(0), m_initialized(false), 
      m_slotSize(60.0f), m_hotbarWidth(600.0f), m_hotbarHeight(60.0f) {
    
    // Initialize all slots as empty
    for (auto& slot : m_slots) {
        slot.clear();
    }
}

Hotbar::~Hotbar() {
    cleanup();
}

bool Hotbar::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Load textures
    m_hotbarTexture = AssetManager::getInstance().loadTexture("assets/textures/hotbar.png");
    m_selectionTexture = AssetManager::getInstance().loadTexture("assets/textures/selecthotbar.png");
    
    if (!m_hotbarTexture || !m_selectionTexture) {
        std::cout << "Failed to load hotbar textures!" << std::endl;
        return false;
    }
    
    // Create shader for hotbar rendering
    m_shader = std::make_unique<Shader>();
    
    if (!m_shader->loadFromFiles("assets/shaders/hotbar.vert", "assets/shaders/hotbar.frag")) {
        std::cout << "Failed to create hotbar shader!" << std::endl;
        return false;
    }
    
    setupGeometry();
    m_initialized = true;
    
    return true;
}

void Hotbar::setupGeometry() {
    // Define quad vertices for slot rendering
    float vertices[] = {
        // Position    // Texture coords
        0.0f, 0.0f,    0.0f, 1.0f,  // Bottom-left
        1.0f, 0.0f,    1.0f, 1.0f,  // Bottom-right
        1.0f, 1.0f,    1.0f, 0.0f,  // Top-right
        0.0f, 1.0f,    0.0f, 0.0f   // Top-left
    };
    
    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };
    
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    
    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Hotbar::render(int screenWidth, int screenHeight) {
    if (!m_initialized || !m_shader) {
        return;
    }
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);  // UI elements should be on top
    
    // Use hotbar shader
    m_shader->use();
    
    // Set up orthographic projection for 2D rendering (bottom-left origin)
    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
    m_shader->setMat4("projection", projection);
    m_shader->setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));
    m_shader->setFloat("alpha", 1.0f);
    
    // Calculate hotbar position (centered at bottom of screen) - using bottom-left origin
    float hotbarX = (screenWidth - m_hotbarWidth) / 2.0f;
    float hotbarY = HOTBAR_BOTTOM_MARGIN;
    
    glBindVertexArray(m_VAO);
    
    // Render each hotbar slot
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        float slotX = hotbarX + i * m_slotSize;
        float slotY = hotbarY;
        
        renderSlot(i, slotX, slotY, m_slotSize);
    }
    
    // Render selection highlight
    float selectedX = hotbarX + m_selectedSlot * m_slotSize;
    float selectedY = hotbarY;
    renderSelection(selectedX, selectedY, m_slotSize);
    
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);  // Re-enable depth testing
    glDisable(GL_BLEND);
}

void Hotbar::renderSlot(int slot, float x, float y, float size) {
    // Bind hotbar texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_hotbarTexture->getID());
    m_shader->setInt("hotbarTexture", 0);
    m_shader->setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f)); // Default white
    m_shader->setFloat("alpha", 1.0f);
    
    // Set position and size
    m_shader->setVec2("position", glm::vec2(x, y));
    m_shader->setVec2("size", glm::vec2(size, size));
    
    // Draw the slot background
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // Render item icon in the slot if not empty
    if (!m_slots[slot].isEmpty()) {
        auto itemTexture = getTextureForBlockType(m_slots[slot].blockType);
        if (itemTexture) {
            // Bind item texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, itemTexture->getID());
            m_shader->setInt("hotbarTexture", 0);
            
            // Make stacked items slightly brighter to show they're stacked
            if (m_slots[slot].count > 1) {
                m_shader->setVec3("color", glm::vec3(1.2f, 1.2f, 1.0f)); // Slightly yellow-bright tint
            } else {
                m_shader->setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f)); // Normal color
            }
            
            // Make the item slightly smaller than the slot for padding
            float itemSize = size * ITEM_PADDING_RATIO;
            float itemOffset = (size - itemSize) * 0.5f;
            m_shader->setVec2("position", glm::vec2(x + itemOffset, y + itemOffset));
            m_shader->setVec2("size", glm::vec2(itemSize, itemSize));
            
            // Draw the item
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            
            // Render item count if more than 1
            if (m_slots[slot].count > 1) {
                renderItemCount(m_slots[slot].count, x, y, size);
            }
            
            // Reset color for next render
            m_shader->setVec3("color", glm::vec3(1.0f, 1.0f, 1.0f));
        }
    }
}

void Hotbar::renderSelection(float x, float y, float size) {
    // Bind selection texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_selectionTexture->getID());
    m_shader->setInt("hotbarTexture", 0);
    
    // Set position and size (slightly larger for highlight effect)
    float highlightSize = size * SELECTION_SCALE;
    float offset = (highlightSize - size) / 2.0f;
    m_shader->setVec2("position", glm::vec2(x - offset, y - offset));
    m_shader->setVec2("size", glm::vec2(highlightSize, highlightSize));
    
    // Draw the selection highlight
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Hotbar::renderItemCount(int count, float x, float y, float size) {
    if (count <= 1) return;
    
    // Position text in bottom-right corner of slot
    float textScale = 1.2f;  // Increased from 0.6f to make text larger
    float digitWidth = 8.0f * textScale;  // Increased for MUCH fatter digits
    float digitHeight = 8.0f * textScale;
    
    // Convert count to string and calculate total width
    std::string countStr = std::to_string(count);
    float totalWidth = countStr.length() * digitWidth;
    
    // Position in bottom-right corner with some padding (adjusted position)
    float textX = x + size - totalWidth - 10.0f;  // Changed from 14.0f to 10.0f (4 pixels right)
    float textY = y + 7.0f;  // Changed from 4.0f to 7.0f (3 pixels up)
    
    for (size_t i = 0; i < countStr.length(); ++i) {
        if (countStr[i] >= '0' && countStr[i] <= '9') {
            int digit = countStr[i] - '0';
            renderDigit(digit, textX + i * digitWidth, textY, textScale);
        }
    }
}

void Hotbar::renderDigit(int digit, float x, float y, float scale) {
    // MUCH FATTER/bolder bitmap font for digits 0-9
    // Each digit is represented as a 5x7 pixel grid with VERY thick strokes
    static const bool digitPatterns[10][35] = {
        // 0 - MUCH FATTER
        {1,1,1,1,1, 1,1,0,1,1, 1,1,0,1,1, 1,1,0,1,1, 1,1,0,1,1, 1,1,0,1,1, 1,1,1,1,1},
        // 1 - MUCH FATTER  
        {0,1,1,0,0, 1,1,1,0,0, 1,1,1,0,0, 0,1,1,0,0, 0,1,1,0,0, 0,1,1,0,0, 1,1,1,1,1},
        // 2 - MUCH FATTER
        {1,1,1,1,1, 1,1,0,1,1, 0,0,0,1,1, 1,1,1,1,1, 1,1,0,0,0, 1,1,0,0,0, 1,1,1,1,1},
        // 3 - MUCH FATTER
        {1,1,1,1,1, 0,0,0,1,1, 0,0,0,1,1, 1,1,1,1,1, 0,0,0,1,1, 0,0,0,1,1, 1,1,1,1,1},
        // 4 - MUCH FATTER
        {1,1,0,1,1, 1,1,0,1,1, 1,1,0,1,1, 1,1,1,1,1, 0,0,0,1,1, 0,0,0,1,1, 0,0,0,1,1},
        // 5 - MUCH FATTER
        {1,1,1,1,1, 1,1,0,0,0, 1,1,0,0,0, 1,1,1,1,1, 0,0,0,1,1, 0,0,0,1,1, 1,1,1,1,1},
        // 6 - MUCH FATTER
        {1,1,1,1,1, 1,1,0,0,0, 1,1,0,0,0, 1,1,1,1,1, 1,1,0,1,1, 1,1,0,1,1, 1,1,1,1,1},
        // 7 - MUCH FATTER
        {1,1,1,1,1, 0,0,0,1,1, 0,0,1,1,0, 0,1,1,0,0, 0,1,1,0,0, 0,1,1,0,0, 0,1,1,0,0},
        // 8 - MUCH FATTER
        {1,1,1,1,1, 1,1,0,1,1, 1,1,0,1,1, 1,1,1,1,1, 1,1,0,1,1, 1,1,0,1,1, 1,1,1,1,1},
        // 9 - MUCH FATTER
        {1,1,1,1,1, 1,1,0,1,1, 1,1,0,1,1, 1,1,1,1,1, 0,0,0,1,1, 0,0,0,1,1, 1,1,1,1,1}
    };
    
    if (digit < 0 || digit > 9) return;
    
    // Back to normal pixel size but with fatter digit patterns
    float pixelSize = 1.5f * scale;  // Reduced back from 2.0f
    const bool* pattern = digitPatterns[digit];
    
    // Render fully black digits (no shadow, no white text)
    m_shader->setVec3("color", glm::vec3(0.0f, 0.0f, 0.0f));
    
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
            if (pattern[row * 5 + col]) {
                float pixelX = x + col * pixelSize;
                float pixelY = y + (6 - row) * pixelSize;
                
                m_shader->setVec2("position", glm::vec2(pixelX, pixelY));
                m_shader->setVec2("size", glm::vec2(pixelSize, pixelSize));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
    }
}

void Hotbar::cleanup() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_EBO != 0) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
    // Textures are now managed by shared_ptr, so they'll be automatically cleaned up
    m_hotbarTexture.reset();
    m_selectionTexture.reset();
    m_shader.reset();
    m_initialized = false;
}

int Hotbar::addItem(BlockType blockType, int count) {
    // Skip AIR blocks
    if (blockType == BlockType::AIR || count <= 0) {
        return count;
    }
    
    int remainingCount = count;
    
    // First, try to stack with existing items
    for (int i = 0; i < HOTBAR_SLOTS; ++i) {
        if (remainingCount <= 0) break;
        
        auto& slot = m_slots[i];
        if (slot.canStack(blockType)) {
            int spaceInSlot = HotbarSlot::MAX_STACK_SIZE - slot.count;
            int toAdd = std::min(remainingCount, spaceInSlot);
            slot.count += toAdd;
            remainingCount -= toAdd;
        }
    }
    
    // If we still have items, find empty slots
    for (int i = 0; i < HOTBAR_SLOTS; ++i) {
        if (remainingCount <= 0) break;
        
        auto& slot = m_slots[i];
        if (slot.isEmpty()) {
            int toAdd = std::min(remainingCount, HotbarSlot::MAX_STACK_SIZE);
            slot.blockType = blockType;
            slot.count = toAdd;
            remainingCount -= toAdd;
        }
    }
    
    return remainingCount;
}

void Hotbar::handleScrollInput(double yOffset) {
    if (yOffset > 0) {
        selectPreviousSlot();
    } else if (yOffset < 0) {
        selectNextSlot();
    }
}

void Hotbar::handleKeyInput(int key) {
    // Handle number keys 1-9, 0 (for slot 10)
    if (key >= '1' && key <= '9') {
        selectSlotByNumber(key - '0');
    } else if (key == '0') {
        selectSlotByNumber(10);
    }
}

int Hotbar::removeItem(BlockType blockType, int count) {
    if (blockType == BlockType::AIR || count <= 0) {
        return 0;
    }
    
    int remainingToRemove = count;
    
    // Remove from slots with matching items
    for (auto& slot : m_slots) {
        if (remainingToRemove <= 0) break;
        
        if (slot.blockType == blockType && !slot.isEmpty()) {
            int toRemove = std::min(remainingToRemove, slot.count);
            slot.count -= toRemove;
            remainingToRemove -= toRemove;
            
            if (slot.count <= 0) {
                slot.clear();
            }
        }
    }
    
    return count - remainingToRemove; // Return how many were actually removed
}

int Hotbar::removeFromSlot(int slot, int count) {
    if (slot < 0 || slot >= HOTBAR_SLOTS || count <= 0) {
        return 0;
    }
    
    auto& slotRef = m_slots[slot];
    if (slotRef.isEmpty()) {
        return 0;
    }
    
    int toRemove = std::min(count, slotRef.count);
    slotRef.count -= toRemove;
    
    if (slotRef.count <= 0) {
        slotRef.clear();
    }
    
    return toRemove;
}

void Hotbar::clearSlot(int slot) {
    if (slot >= 0 && slot < HOTBAR_SLOTS) {
        m_slots[slot].clear();
    }
}

void Hotbar::swapSlots(int slot1, int slot2) {
    if (slot1 >= 0 && slot1 < HOTBAR_SLOTS && 
        slot2 >= 0 && slot2 < HOTBAR_SLOTS && 
        slot1 != slot2) {
        std::swap(m_slots[slot1], m_slots[slot2]);
    }
}

HotbarSlot Hotbar::getSlot(int slot) const {
    if (slot >= 0 && slot < HOTBAR_SLOTS) {
        return m_slots[slot];
    }
    return HotbarSlot{}; // Return empty slot for invalid indices
}

bool Hotbar::hasSpace() const {
    for (const auto& slot : m_slots) {
        if (slot.isEmpty() || !slot.isFull()) {
            return true;
        }
    }
    return false;
}

bool Hotbar::hasSpaceFor(BlockType blockType, int count) const {
    if (blockType == BlockType::AIR || count <= 0) {
        return true;
    }
    
    int remainingCount = count;
    
    // Check existing stacks
    for (const auto& slot : m_slots) {
        if (slot.canStack(blockType)) {
            int spaceInSlot = HotbarSlot::MAX_STACK_SIZE - slot.count;
            remainingCount -= spaceInSlot;
            if (remainingCount <= 0) {
                return true;
            }
        }
    }
    
    // Check empty slots
    for (const auto& slot : m_slots) {
        if (slot.isEmpty()) {
            remainingCount -= HotbarSlot::MAX_STACK_SIZE;
            if (remainingCount <= 0) {
                return true;
            }
        }
    }
    
    return remainingCount <= 0;
}

std::shared_ptr<Texture> Hotbar::getTextureForBlockType(BlockType blockType) {
    // Map block types to their corresponding textures
    switch (blockType) {
        case BlockType::GRASS:
            return AssetManager::getInstance().loadTexture("assets/textures/grass.png");
        case BlockType::DIRT:
            return AssetManager::getInstance().loadTexture("assets/textures/grass.png"); // Use grass for now
        case BlockType::STONE:
            return AssetManager::getInstance().loadTexture("assets/textures/stone.png");
        case BlockType::OAK_LOG:
            return AssetManager::getInstance().loadTexture("assets/textures/oak.png");
        case BlockType::LEAVES:
            return AssetManager::getInstance().loadTexture("assets/textures/oakleave.png");
        case BlockType::WATER:
            return AssetManager::getInstance().loadTexture("assets/textures/water.png");
        case BlockType::GRAVEL:
            return AssetManager::getInstance().loadTexture("assets/textures/gravel.png");
        default:
            // Default to stone texture for unknown blocks
            return AssetManager::getInstance().loadTexture("assets/textures/stone.png");
    }
}
