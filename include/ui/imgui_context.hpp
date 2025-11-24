// AI Assistance: GitHub Copilot was used for RAII pattern implementation
// and modern C++20 resource management techniques.

#pragma once

#include <threepp/extras/imgui/ImguiContext.hpp>
#include <memory>
#include <functional>

/**
 * Internal implementation class that inherits from threepp's ImguiContext
 * and implements the required pure virtual function.
 */
class ImguiContextImpl : public ImguiContext {
public:
    explicit ImguiContextImpl(void* windowPtr, std::function<void()> renderFunc)
        : ImguiContext(windowPtr), renderCallback_(std::move(renderFunc)) {}

    // Virtual destructor for proper cleanup when deleted via base pointer
    ~ImguiContextImpl() override = default;

    // Delete copy and move to prevent issues with the base class
    ImguiContextImpl(const ImguiContextImpl&) = delete;
    ImguiContextImpl& operator=(const ImguiContextImpl&) = delete;
    ImguiContextImpl(ImguiContextImpl&&) = delete;
    ImguiContextImpl& operator=(ImguiContextImpl&&) = delete;

protected:
    void onRender() override {
        if (renderCallback_) {
            renderCallback_();
        }
    }

private:
    std::function<void()> renderCallback_;
};

/**
 * RAII wrapper for ImGui context using threepp's built-in support.
 * Ensures proper initialization and cleanup with exception safety.
 * Uses smart pointers exclusively - no raw pointer management.
 */
class ImGuiContextWrapper {
public:
    // RAII initialization - throws on failure
    explicit ImGuiContextWrapper(void* windowPtr);

    // Destructor handles cleanup automatically
    ~ImGuiContextWrapper();

    // Delete copy and move operations - resource should not be duplicated
    ImGuiContextWrapper(const ImGuiContextWrapper&) = delete;
    ImGuiContextWrapper& operator=(const ImGuiContextWrapper&) = delete;
    ImGuiContextWrapper(ImGuiContextWrapper&&) = delete;
    ImGuiContextWrapper& operator=(ImGuiContextWrapper&&) = delete;

    // Start a new frame
    void newFrame();

    // Render ImGui
    void render();

    // Check if initialized successfully
    [[nodiscard]] bool isInitialized() const noexcept { return initialized_; }

private:
    std::unique_ptr<ImguiContextImpl> instance_;
    bool initialized_;
};
