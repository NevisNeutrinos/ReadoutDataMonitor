//
// Created by Jon Sensenig on 5/5/25.
//

#include "data_monitor.h"
#include <iostream>
#include <random>

namespace data_monitor {

    DataMonitor::DataMonitor(asio::io_context& io_context, const std::string& ip_address, const uint16_t command_port,
                             const uint16_t status_port, bool is_server, bool is_running) :
    command_client_(io_context, ip_address, command_port, is_server, true, false),
    status_client_(io_context, ip_address, status_port, is_server, false, true),
    random_generator_(std::random_device()()),
    event_distrib_(event_min, event_max),
    charge_channel_distrib_(charge_min,charge_max),
    light_channel_distrib_(light_min, light_max),
    light_algs_(),
    charge_algs_(),
    debug_(true)
    {
        std::cout << "DM" << std::endl;
        process_events_ = std::make_unique<ProcessEvents>(light_slot_, false, std::vector<uint16_t>(), false);
        process_events_->UseEventStride(true);
    }

    DataMonitor::~DataMonitor() {
        // TODO make sure this doesn't leak memory
        // if this class is destructed with an open file
        process_events_.reset();
    }

    void DataMonitor::SetRunning(const bool run) {
        is_running_.store(run);
        command_client_.setStopCmdRead(!run);
        status_client_.setStopCmdRead(!run);
    }

    void DataMonitor::ReceiveCommand() {
        while (is_running_.load()) {
            Command cmd = command_client_.ReadRecvBuffer();
            HandleCommand(cmd);
        }
    }

    void DataMonitor::setNumEvent(std::vector<int32_t> &args) {
        int32_t num_events_ = args.at(2);
        event_stride_ = args.at(3);
        process_num_events_ = num_events_ * event_stride_;
        // 5k events per file so return error if requesting more
        if (process_num_events_ > 5000) process_num_events_ = 5000;
    }

    void DataMonitor::setFileName(std::vector<int32_t> &args) {
        std::string base_path("/home/pgrams/data/nov2025_integration_data/readout_data/");
        int32_t run_number = args.at(0);
        int32_t file_number = args.at(1);
        monitor_file_ = base_path + "pGRAMS_bin_" + std::to_string(run_number) + "_" + std::to_string(file_number) + ".dat";
        std::cout << "Requested file: " << monitor_file_ << std::endl;
    }

    void DataMonitor:: HandleCommand(Command& cmd) {
        std::cout << "Received command: 0x" << std::hex << cmd.command << std::dec << std::endl;
        switch (cmd.command) {
            case kMinimalQuery: {
                // Create file name
                if (cmd.arguments.size() < 4) break;
                setFileName(cmd.arguments);
                setNumEvent(cmd.arguments);
                // Set the functions to process events, create metrics and update them
                // since the class members have an implicit this pointer to the current instance of the
                // class we have to make it explicit with a lambda function
                metric_creator_ = [this](EventStruct& evt) { this->CreateMinimalMetrics(evt); };
                update_metrics_ = [this]() { this->UpdateMinimalMetrics(); };

                ProcessFile();
                break;
            }
            case kStopDecoder: {
                break;
            }
            case kDecodeEvent: {
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

    bool DataMonitor::OpenFile() {
        // Using file and run sent from ground
        // const std::string file_name = "pGRAMS_bin_" + std::to_string(1) + "_" + std::to_string(0) + ".dat";
        if (!process_events_->OpenFile(monitor_file_)) {
            std::cerr << "Failed to load file!" << std::endl;
        }
    }

    void DataMonitor::ProcessFile() {
        // if (!OpenFile()) { return; }
        OpenFile();
        GetEventMetrics();
    }

    void DataMonitor::GetEventMetrics() {
        // Set the decoder stride
        process_events_->SetEventStride(event_stride_);
        // Loop through desired events, either adjacent events or with some stride
        size_t event_count = 0;
        while (process_events_->GetEvent() && (event_count < process_num_events_) && (event_count < EVENT_LOOP_MAX)) {
            // the decoder must iterate through each event since we don't know a priori the event size
            if ((event_count % event_stride_) != 0) {
                event_count++;
                continue;
            }
            if (debug_) std::cout << "Processing event: " << event_count << std::endl;
            EventStruct evt_data = process_events_->GetEventStruct();
            // Calculate event metrics
            metric_creator_(evt_data);
            event_count++;
        }
        // The number of desired events have been processed and metrics created
        // so update the metrics and send them
        update_metrics_();
        SendMetrics(lbw_metrics_, metrics_);
    }

    void DataMonitor::SendMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {
        // Send the LBW metrics first
        auto tmp_vec = lbw_metrics.serialize();
        Command lbw_cmd(0x4001, tmp_vec.size());
        lbw_cmd.arguments = std::move(tmp_vec);
        status_client_.WriteSendBuffer(lbw_cmd);

        // Send the metrics
        tmp_vec = metrics.serialize();
        Command cmd(0x4002, tmp_vec.size());
        cmd.arguments = std::move(tmp_vec);
        status_client_.WriteSendBuffer(cmd);
    }

    void DataMonitor::CreateMinimalMetrics(EventStruct & event) {
        charge_algs_.MinimalSummary(event);
        if (debug_) std::cout << "Processed charge.." << std::endl;
        light_algs_.MinimalSummary(event);
        if (debug_) std::cout << "Processed light.." << std::endl;
    }

    void DataMonitor::UpdateMinimalMetrics() {
        charge_algs_.UpdateMinimalMetrics(lbw_metrics_, metrics_);
        if (debug_) std::cout << "Updated charge.." << std::endl;
        light_algs_.UpdateMinimalMetrics(lbw_metrics_, metrics_);
        if (debug_) std::cout << "Updated light.." << std::endl;
    }

    // void DataMonitor::SelectEvents() {
    //     // Randomly but uniformly select events to process
    //     selected_events_.clear();
    //     for (size_t i = 0; i < events_per_file; i++) selected_events_.push_back(event_distrib_(random_generator_));
    //     // Events must be in ascending order so we can check them in order as we decode the file
    //     if (!selected_events_.empty()) std::sort(selected_events_.begin(), selected_events_.end());
    // }
    //
    // uint16_t DataMonitor::SelectChargeChannel() {
    //     return charge_channel_distrib_(random_generator_);
    // }
    //
    // size_t DataMonitor::SelectLightChannel() {
    //     return light_channel_distrib_(random_generator_);
    // }

} // data_monitor