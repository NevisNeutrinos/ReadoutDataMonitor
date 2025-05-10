//
// Created by Jon Sensenig on 5/5/25.
//

#include "data_monitor.h"
#include <iostream>
#include <random>

namespace data_monitor {

    DataMonitor::DataMonitor(asio::io_context& io_context, const std::string& ip_address,
                             short port, bool is_server, bool is_running) :
    tcp_connection_(io_context, ip_address, port, is_server),
    random_generator_(std::random_device()()),
    event_distrib_(event_min, event_max),
    charge_channel_distrib_(charge_min,charge_max),
    light_channel_distrib_(light_min, light_max)
    {
        std::cout << "DM" << std::endl;
        process_events_ = std::make_unique<ProcessEvents>(light_slot_);
    }

    void DataMonitor::SetRunning(const bool run) {
        is_running_.store(run);
        tcp_connection_.setStopCmdRead(!run);
    }

    void DataMonitor::ReceiveCommand() {
        while (is_running_.load()) {
            Command cmd = tcp_connection_.ReadRecvBuffer();
            HandleCommand(cmd);
        }
    }

    void DataMonitor::RunDecoder() {
        is_decoding_.store(true);
        decode_thread_ = std::thread(&DataMonitor::RunMetrics, this);
    }

    void DataMonitor::StopDecoder() {
        is_decoding_.store(false);
        if (decode_thread_.joinable()) {
            decode_thread_.join();
        }
    }

    void DataMonitor:: HandleCommand(Command& cmd) {
        std::cout << "Received command: 0x" << std::hex << cmd.command << std::dec << std::endl;
        switch (cmd.command) {
            case kRunDecoder: {
                RunDecoder();
                break;
            }
            case kStopDecoder: {
                StopDecoder();
                break;
            }
            case kDecodeEvent: {
                DecodeEvent(cmd.arguments);
                break;
            }
            default: {
                std::cerr << "Unknown command: 0x" << std::hex << cmd.command << std::dec << std::endl;
            }
        }
    }

    void DataMonitor::RunMetrics() {
        // 1. check for received command
        // 2. if so execute command, otherwise process event
        // 3. Send event results
        while (is_decoding_.load()) {
            OpenFile();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            const uint16_t charge_channel = SelectChargeChannel();
            const uint16_t light_channel = SelectLightChannel();
            SelectEvents();
            GetEvents(charge_channel, light_channel);
        }
    }

    void DataMonitor::OpenFile() {
        // Todo need to select files somehow
        const std::string file_name = "pGRAMS_bin_" + std::to_string(1) + "_" + std::to_string(0) + ".dat";
        if (!process_events_->OpenFile(file_name)) {
            std::cerr << "Failed to load file!" << std::endl;
        }
    }

    void DataMonitor::DecodeEvent(std::vector<int32_t>& args) {
        if (args.size() != 3) {
            return;
        }
        selected_events_.clear();
        selected_events_.push_back(args.at(0));
        const int32_t charge_channel = args.at(1);
        const int32_t light_channel = args.at(2);
        GetEvents(charge_channel, light_channel);
    }

    void DataMonitor::SelectEvents() {
        // Randomly but uniformly select events to process
        selected_events_.clear();
        for (size_t i = 0; i < events_per_file; i++) selected_events_.push_back(event_distrib_(random_generator_));
        // Events must be in ascending order so we can check them in order as we decode the file
        if (!selected_events_.empty()) std::sort(selected_events_.begin(), selected_events_.end());
    }

    uint16_t DataMonitor::SelectChargeChannel() {
        return charge_channel_distrib_(random_generator_);
    }

    size_t DataMonitor::SelectLightChannel() {
        return light_channel_distrib_(random_generator_);
    }

    void DataMonitor::GetEvents(uint16_t charge_channel, uint16_t light_channel) {
        // 1. Throw number to randomly select charge and light channels
        // 2. Loop through all events in file to find event(s)
        // 3. While looping check FEM header data
        // 4. Process channel data in the "subscribed" metric algs
        // 5. Send results
        Command cmd(0x26, 3);
        cmd.arguments = {selected_events_.at(0), charge_channel, light_channel}; // FIXME sending fake status
        tcp_connection_.WriteSendBuffer(cmd);
    }
} // data_monitor