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
    return choice;
}

// Prints the current state and available options.
void PrintState() {
    std::cout << "Select a command:\n";
    std::cout << "  [1] MinSummary\n";
    std::cout << "  [2] Get 1 Event\n";
    std::cout << "  [-1] Exit\n";
    std::cout << "Enter choice: ";
}

// Runs the command-line interface for the state machine.
void Run(data_monitor::DataMonitor& dm, uint32_t run, uint32_t file_number, uint32_t num_evts, uint32_t stride, uint32_t random_flag) {

    Command cmd(1,5);
    while (true) {
        PrintState();
        int input = GetUserInput();
        if (input == -1) {
            std::cout << "Exiting...\n";
            break;
        }

        if (input == 1) {
            cmd.command = static_cast<int>(pgrams::communication::CommunicationCodes::COL_Query_LB_Data);
            cmd.arguments = {run, file_number, num_evts, stride};
        } else if (input == 2) { // treat num_evts as event_number
            cmd.command = static_cast<int>(pgrams::communication::CommunicationCodes::COL_Query_Event_Data);
            cmd.arguments = {run, file_number, num_evts, random_flag};
        } else {
            std::cerr << "Invalid input. Please enter a number.\n";
            continue;
        }

        dm.HandleCommand(cmd);
    }
}

int main(int argc, char* argv[]) {

    if (argc < 6) {
        std::cerr << "Please include IP address and port!" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <RUN> <FILE_NUM> <NUM_EVT> <STRIDE> <RANDOM_FLAG>\n";
        return 1;
    }

    uint32_t run_number = std::stoi(argv[1]);
    uint32_t file_number = std::stoi(argv[2]);
    uint32_t num_evt = std::stoi(argv[3]);
    uint32_t stride = std::stoi(argv[4]);
    uint32_t random_flag = std::stoi(argv[5]);

    std::cout << "Runnning!" << std::endl;

    asio::io_context io_context;
    std::cout << "Starting controller..." << std::endl;
    bool run = true;
    //data_monitor::DataMonitor dm(io_context, "127.0.0.1", 1753, 1752, false, run);
    data_monitor::DataMonitor dm(io_context, "10.44.45.96", 50017, 50016, false, run);

    std::thread io_thread1([&]() { io_context.run(); });
    std::thread io_thread2([&]() { io_context.run(); });
    std::thread monitor_thread([&]() { dm.ReceiveCommand(); });

    // Run the command line control
    Run(dm, run_number, file_number, num_evt, stride, random_flag);

    dm.SetRunning(false);
    monitor_thread.join();
    io_context.stop();
    io_thread1.join();
    io_thread2.join();

    return 0;
}
