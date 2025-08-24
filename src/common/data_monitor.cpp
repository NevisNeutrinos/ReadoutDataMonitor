//
// Created by Jon Sensenig on 5/5/25.
//

#include "data_monitor.h"
#include <iostream>
#include <random>

namespace data_monitor {

    DataMonitor::DataMonitor(asio::io_context& io_context, const std::string& ip_address, const uint16_t command_port,
                             const uint16_t status_port, bool is_server, bool is_running) :
    tcp_connection_(io_context, ip_address, command_port, is_server, false),
    random_generator_(std::random_device()()),
    event_distrib_(event_min, event_max),
    charge_channel_distrib_(charge_min,charge_max),
    light_channel_distrib_(light_min, light_max),
    light_algs_(),
    charge_algs_()
    {
        std::cout << "DM" << std::endl;
        process_events_ = std::make_unique<ProcessEvents>(light_slot_, false, std::vector<uint16_t>());
        process_events_->UseEventStride(true);
        process_events_->SetEventStride(event_stride_);
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
        // Set the metric to use

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
                // charge_metric_ = cmd.arguments.at(0);
                // light_metric_ = cmd.arguments.at(1);
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

    void SetMetrics(int32_t charge_metric, int32_t light_metric) {
        switch (charge_metric) {
            case 0x1: {
                // TpcMonitor charge_monitor_;
                break;
            }
            default: {
                std::cerr << "Unknown charge metric: 0x" << std::hex << charge_metric << std::dec << std::endl;
            }
        }
        switch (light_metric) {
            case 0x1: {
                break;
            }
            default: {
                std::cerr << "Unknown light metric: 0x" << std::hex << charge_metric << std::dec << std::endl;
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
        // const std::string file_name = "pGRAMS_bin_" + std::to_string(1) + "_" + std::to_string(0) + ".dat";
        if (!process_events_->OpenFile(monitor_file_)) {
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
        /*
           x 1. Throw number to randomly select charge and light channels
            2. Loop through all events in file to find event(s)
            3. Process channel data in the "subscribed" metric algs
            4. Send results
        */

        // 1. Decided not to do this
        // 2.
        size_t event_count = 0;
        while (process_events_->GetEvent() && (event_count < EVENT_LOOP_MAX)) {
            if ((event_count % event_stride_) != 0) {
                event_count++;
                continue;
            }
            lbw_metrics_.clear();
            metrics_.clear();
            EventStruct evt_data = process_events_->GetEventStruct();
            // Calculate event metrics
            charge_algs_.ProcessEvent(evt_data, lbw_metrics_, metrics_);
            light_algs_.ProcessEvent(evt_data, lbw_metrics_, metrics_);
            lbw_metrics_.print();
            metrics_.print();

            // Send metrics
            SendMetrics(lbw_metrics_, metrics_);

            event_count++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // std::cout << event_count << ": \n"
        //   << "FEM Evt#: " << evt_data.event_number.at(0) << "/"  << evt_data.event_number.at(1) << "\n"
        //   << "Charge/Light #Ch: " << evt_data.charge_adc.size() << "/" << evt_data.light_channel.size() << std::endl;

        // Command cmd(0x26, 3);
        // cmd.arguments = {selected_events_.at(0), charge_channel, light_channel}; // FIXME sending fake status
        // tcp_connection_.WriteSendBuffer(cmd);
    }

    void DataMonitor::SendMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {
        // Send the LBW metrics first
        auto tmp_vec = lbw_metrics.serialize();
        Command lbw_cmd(0x4001, tmp_vec.size());
        lbw_cmd.arguments = std::move(tmp_vec);
        tcp_connection_.WriteSendBuffer(lbw_cmd);

        // Send the metrics
        tmp_vec = metrics.serialize();
        Command cmd(0x4002, tmp_vec.size());
        cmd.arguments = std::move(tmp_vec);
        tcp_connection_.WriteSendBuffer(cmd);
    }

} // data_monitor