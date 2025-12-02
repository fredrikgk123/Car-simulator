#include <threepp/threepp.hpp>
#include "core/game.hpp"
#include "ui/imgui_context.hpp"
#include <iostream>
#include <stdexcept>
#include <memory>

using namespace threepp;

int main() {
    try {
        std::cout << "Starting Car Simulator..." << std::endl;

        Canvas canvas("Car Simulator");

        // Initialize ImGui with RAII for automatic cleanup
        std::cout << "Initializing ImGui..." << std::endl;
        auto imguiContext = std::make_unique<ImGuiContextWrapper>(canvas.windowPtr());

        if (!imguiContext->isInitialized()) {
            std::cerr << "Error: ImGui initialization failed" << std::endl;
            return 1;
        }

        std::cout << "Creating game instance..." << std::endl;
        auto game = std::make_unique<Game>(canvas);
        game->initialize();

        std::cout << "Entering main game loop..." << std::endl;

        canvas.animate([&game, &imguiContext] {
            try {
                if (game->shouldExit()) {
                    return;
                }

                float deltaTime = game->getClock().getDelta();

                game->update(deltaTime);

                imguiContext->newFrame();

                game->render();

                imguiContext->render();

            } catch (const std::exception& e) {
                std::cerr << "Error in game loop: " << e.what() << std::endl;
                game->requestExit();
            } catch (...) {
                std::cerr << "Unknown error in game loop" << std::endl;
                game->requestExit();
            }
        });

        // RAII ensures proper cleanup in reverse order
        std::cout << "Shutting down..." << std::endl;
        std::cout << "Car Simulator exited successfully." << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error during initialization: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Fatal error: Unknown exception during initialization" << std::endl;
        return 1;
    }
}
