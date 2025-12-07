//
// Created by Jon Sensenig on 5/5/25.
//

#include "src/common/data_monitor.h"
#include <iostream>
#include <thread>

// Gets user input safely.
int GetUserInput() {
    int choice;
    std::cin >> choice;

    // Handle invalid input (non-numeric)
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a number.\n";
        return GetUserInput();  // Retry
    }

    switch (choice) {
        case 0x1: {
            return static_cast<int>(pgrams::communication::CommunicationCodes::COL_Query_LB_Data);
        } case -1: {
            return -1;
        } default: {
            std::cerr << "Invalid input. Please enter a number.\n";
            return GetUserInput();
        }
    }
}

// Prints the current state and available options.
void PrintState() {
    std::cout << "Select a command:\n";
    std::cout << "  [1] MinSummary\n";
    std::cout << "  [-1] Exit\n";
    std::cout << "Enter choice: ";
}

// Runs the command-line interface for the state machine.
void Run(data_monitor::DataMonitor& dm, int run, int file_number, int num_evts, int stride) {

    Command cmd(1,4);
    while (true) {
        PrintState();
        int input = GetUserInput();
        if (input == -1) {
            std::cout << "Exiting...\n";
            break;
        }
        cmd.command = static_cast<uint16_t>(input);
        cmd.arguments = {run, file_number, num_evts, stride};
        dm.HandleCommand(cmd);
    }
}

int main(int argc, char* argv[]) {

    if (argc < 5) {
        std::cerr << "Please include IP address and port!" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <RUN> <FILE_NUM> <NUM_EVT> <STRIDE>\n";
        return 1;
    }

    int run_number = std::stoi(argv[1]);
    int file_number = std::stoi(argv[2]);
    int num_evt = std::stoi(argv[3]);
    int stride = std::stoi(argv[4]);

    std::cout << "Runnning!" << std::endl;

    asio::io_context io_context;
    std::cout << "Starting controller..." << std::endl;
    bool run = true;
    data_monitor::DataMonitor dm(io_context, "127.0.0.1", 1753, 1752, false, run);

    std::thread io_thread1([&]() { io_context.run(); });
    std::thread io_thread2([&]() { io_context.run(); });
    std::thread monitor_thread([&]() { dm.ReceiveCommand(); });

    // Run the command line control
    Run(dm, run_number, file_number, num_evt, stride);

    dm.SetRunning(false);
    monitor_thread.join();
    io_context.stop();
    io_thread1.join();
    io_thread2.join();

    return 0;
}
