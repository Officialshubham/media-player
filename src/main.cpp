#include "PlayerGUI.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <exception>

// Global pointer for signal handling
PlayerGUI* g_player = nullptr;

// Signal handler for unexpected termination
void signal_handler(int signal) {
    std::cerr << "\nReceived signal " << signal << ". Cleaning up..." << std::endl;
    
    if (g_player) {
        // Perform cleanup
        delete g_player;
        g_player = nullptr;
    }
    
    // Exit with error code
    std::exit(EXIT_FAILURE);
}

// Setup signal handlers
void setup_signal_handlers() {
    std::signal(SIGINT, signal_handler);   // Ctrl+C
    std::signal(SIGTERM, signal_handler);  // Termination request
    std::signal(SIGSEGV, signal_handler);  // Segmentation fault
    std::signal(SIGABRT, signal_handler);  // Abort signal
}

int main(int argc, char* argv[]) {
    std::cout << "Starting GUI Media Player..." << std::endl;
    
    // Setup signal handlers for unexpected termination
    setup_signal_handlers();
    
    try {
        PlayerGUI player;
        g_player = &player;  // Store global reference
        
        player.run(argc, argv);
        
        g_player = nullptr;  // Clear global reference
        
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown exception occurred!" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << "Media Player exited successfully." << std::endl;
    return 0;
}