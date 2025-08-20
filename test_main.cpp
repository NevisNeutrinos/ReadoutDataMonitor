//
// Created by Jon Sensenig on 8/20/25.
//

#include "src/common/data_monitor.h"
#include <iostream>
#include <thread>

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Please include file name to monitor!" << std::endl;
        return 1;
    }
    const std::string monitor_file_name(argv[1]);
    std::cout << "Monitor file: " << monitor_file_name << std::endl;

    std::cout << "Runnning!" << std::endl;

    asio::io_context io_context;
    std::cout << "Starting controller..." << std::endl;
    bool run = true;
    data_monitor::DataMonitor dm(io_context, "127.0.0.1", 1234, 1235, true, run);

    std::thread io_thread([&]() { io_context.run(); });

    // Run monitor manually
    dm.SetMonitorFile(monitor_file_name);
    dm.OpenFile();
    dm.GetEvents(0,0);

    // Shutdown monitor
    dm.SetRunning(false);

    io_context.stop();
    io_thread.join();

    return 0;
}
