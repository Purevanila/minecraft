#pragma once

#include "engine/graphics/Shader.h"
#include "engine/graphics/Texture.h"
#include "world/Block.h"
#include <glm/glm.hpp>
#include <memory>
#include <array>

// Structure to hold item data in a hotbar slot
struct HotbarSlot {
    BlockType blockType = BlockType::AIR;
    int count = 0;
    
    static constexpr int MAX_STACK_SIZE = 99;
    
    bool isEmpty() const { return blockType == BlockType::AIR || count <= 0; }
    bool isFull() const { return count >= MAX_STACK_SIZE; }
    bool canStack(BlockType otherType) const { 
        return blockType == otherType && !isFull(); 
    }
    void clear() { blockType = BlockType::AIR; count = 0; }
};

/**
 * Hotbar UI System
 * Renders a Minecraft-style hotbar with 10 inventory slots
 */
class Hotbar {
public:
    Hotbar();
    ~Hotbar();
    
    bool initialize();
    void render(int screenWidth, int screenHeight);
    void cleanup();
    
    // Hotbar interaction
    void setSelectedSlot(int slot) { 
        if (slot >= 0 && slot < HOTBAR_SLOTS) {
            m_selectedSlot = slot; 
        }
    }
    int getSelectedSlot() const { return m_selectedSlot; }
    void selectNextSlot() { m_selectedSlot = (m_selectedSlot + 1) % HOTBAR_SLOTS; }
    void selectPreviousSlot() { m_selectedSlot = (m_selectedSlot + HOTBAR_SLOTS - 1) % HOTBAR_SLOTS; }
    void selectSlotByNumber(int number) { 
        if (number >= 1 && number <= HOTBAR_SLOTS) {
            m_selectedSlot = number - 1; 
        }
    }
    
    // Input handling
    void handleScrollInput(double yOffset);
    void handleKeyInput(int key);
    
    // Inventory management
    void setSlotItem(int slot, BlockType blockType, int count = 1) { 
        if (slot >= 0 && slot < 10) {
            m_slots[slot].blockType = blockType;
            m_slots[slot].count = count;
        }
    }
    BlockType getSlotItem(int slot) const { 
        return (slot >= 0 && slot < 10) ? m_slots[slot].blockType : BlockType::AIR; 
    }
    int getSlotCount(int slot) const {
        return (slot >= 0 && slot < 10) ? m_slots[slot].count : 0;
    }
    BlockType getSelectedItem() const { return m_slots[m_selectedSlot].blockType; }
    int getSelectedCount() const { return m_slots[m_selectedSlot].count; }
    
    // Add item to hotbar with stacking (returns number of items that couldn't be added)
    int addItem(BlockType blockType, int count = 1);
    
    // Remove items from hotbar
    int removeItem(BlockType blockType, int count = 1);
    int removeFromSlot(int slot, int count = 1);
    void clearSlot(int slot);
    
    // Slot management
    void swapSlots(int slot1, int slot2);
    HotbarSlot getSlot(int slot) const;
    
    // Check if hotbar has space for more items
    bool hasSpace() const;
    bool hasSpaceFor(BlockType blockType, int count = 1) const;
    
private:
    void setupGeometry();
    void renderSlot(int slot, float x, float y, float size);
    void renderSelection(float x, float y, float size);
    void renderItemCount(int count, float x, float y, float size);
    void renderDigit(int digit, float x, float y, float scale);
    std::shared_ptr<Texture> getTextureForBlockType(BlockType blockType);
    
    unsigned int m_VAO, m_VBO, m_EBO;
    std::unique_ptr<Shader> m_shader;
    std::shared_ptr<Texture> m_hotbarTexture, m_selectionTexture;
    
    // Hotbar state
    static constexpr int HOTBAR_SLOTS = 10;
    static constexpr int MAX_STACK_SIZE = 99;
    static constexpr float ITEM_PADDING_RATIO = 0.8f;
    static constexpr float SELECTION_SCALE = 1.1f;
    static constexpr float HOTBAR_BOTTOM_MARGIN = 24.0f;
    std::array<HotbarSlot, HOTBAR_SLOTS> m_slots;
    int m_selectedSlot;
    
    // UI properties
    bool m_initialized;
    float m_slotSize;
    float m_hotbarWidth;
    float m_hotbarHeight;
};
