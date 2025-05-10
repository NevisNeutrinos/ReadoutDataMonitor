//
// Created by Jon Sensenig on 5/5/25.
//

#include "src/common/data_monitor.h"
#include <iostream>
#include <thread>

int main() {
    std::cout << "Runnning!" << std::endl;

    asio::io_context io_context;
    std::cout << "Starting controller..." << std::endl;
    bool run = true;
    data_monitor::DataMonitor dm(io_context, "127.0.0.1", 1752, true, run);

    std::thread io_thread([&]() { io_context.run(); });
    std::thread monitor_thread([&]() { dm.ReceiveCommand(); });

    // dm.SetRunning(false);
    // std::cout << "Data Monitor stopped!\n";

    monitor_thread.join();
    io_context.stop();
    io_thread.join();

    for (int i = 0; i < 50; i++) {
        auto charge_num = dm.SelectChargeChannel();
        auto light_num = dm.SelectLightChannel();
        std::cout << "Charge/Light " << charge_num << "/" << light_num << std::endl;
    }

    return 0;
}