// AI Assistance: GitHub Copilot was used for RAII wrapper pattern
// and exception-safe resource management implementation.

#include "ui/imgui_context.hpp"
#include <iostream>
#include <stdexcept>

ImGuiContextWrapper::ImGuiContextWrapper(void* windowPtr)
    : instance_(nullptr), initialized_(false) {

    if (!windowPtr) {
        throw std::invalid_argument("ImGuiContextWrapper: windowPtr cannot be null");
    }

    try {
        // Create threepp's ImguiContext with an empty lambda
        // The actual UI content will be drawn by ImGuiLayer between newFrame() and render()
        instance_ = std::make_unique<ImguiContextImpl>(windowPtr, []() {});
        initialized_ = true;
    } catch (const std::exception& e) {
        std::cerr << "ImGuiContextWrapper initialization failed: " << e.what() << std::endl;
        throw std::runtime_error(std::string("Failed to initialize ImGui: ") + e.what());
    } catch (...) {
        std::cerr << "ImGuiContextWrapper initialization failed with unknown error" << std::endl;
        throw std::runtime_error("Failed to initialize ImGui: unknown error");
    }
}

ImGuiContextWrapper::~ImGuiContextWrapper() {
    // Smart pointer automatically cleans up resources
    // No manual cleanup needed - RAII handles everything
    instance_.reset();
    initialized_ = false;
}

void ImGuiContextWrapper::newFrame() {
    // threepp's ImguiContext::render() calls these internally, but we need them
    // separately for our rendering flow
    if (!initialized_ || !instance_) {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiContextWrapper::render() {
    if (!initialized_ || !instance_) {
        return;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
