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
        std::cout << "DM End" << std::endl;
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
   
    void DataMonitor::Run() {
        is_running_ = true;
        ReceiveCommand();
    }

    void DataMonitor::ReceiveCommand() {
        while (is_running_.load()) {
            Command cmd = command_client_.ReadRecvBuffer();
            HandleCommand(cmd);
        }
    }

    void DataMonitor::setNumEvent(std::vector<uint32_t> &args) {
        uint32_t num_events_ = args.at(2);
        event_stride_ = args.at(3);
        process_num_events_ = num_events_ * event_stride_;
        // 5k events per file so return error if requesting more
        if (process_num_events_ > 5000) process_num_events_ = 5000;
    }

    void DataMonitor::setFileName(std::vector<uint32_t> &args) {
        // std::string base_path("/home/pgrams/data/nov2025_integration_data/readout_data/");
        std::string base_path("/home/pgrams/data/readout_data/");
        //std::string base_path("/home/sabertooth2/GramsReadout/build/ReadoutDataMonitor/");
        uint32_t run_number = args.at(0);
        uint32_t file_number = args.at(1);
        monitor_file_ = base_path + "pGRAMS_bin_" + std::to_string(run_number) + "_" + std::to_string(file_number) + ".dat";
        std::cout << "Requested file: " << monitor_file_ << std::endl;
    }

    void DataMonitor:: HandleCommand(Command& cmd) {
        std::cout << "Received command: 0x" << std::hex << cmd.command << std::dec << std::endl;
        switch (cmd.command) {
            case static_cast<int>(CommunicationCodes::COL_Query_LB_Data): {
                // Create file name
                if (cmd.arguments.size() < 5) break;
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
            case static_cast<int>(CommunicationCodes::COL_Query_Event_Data): {
                // Create file name
                if (cmd.arguments.size() < 5) break;
                setFileName(cmd.arguments);
                setNumEvent(cmd.arguments);
                choose_random_ = cmd.arguments.at(4) == 1;
                if (process_num_events_ > 1) break;
                // Set the functions to process events, create metrics and update them
                // since the class members have an implicit this pointer to the current instance of the
                // class we have to make it explicit with a lambda function
                metric_creator_ = [this](EventStruct& evt) { this->CreateEventMetrics(evt); };
                update_metrics_ = [this]() { this->UpdateEventMetrics(); };
                ProcessFile();
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

    void SetMetrics(uint32_t charge_metric, uint32_t light_metric) {
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

    void DataMonitor::ProcessFile() {
        // if (!OpenFile()) { return; }
        if (!process_events_->OpenFile(monitor_file_)) {
            std::cerr << "Failed to load file!" << std::endl;
        }
        GetEventMetrics();
    }

    void DataMonitor::GetEventMetrics() {
        if (debug_) std::cout << "entering processing" << std::endl;
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
    }

    // void DataMonitor::SendMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {
    //     // Send the LBW metrics
    //     auto tmp_vec = lbw_metrics.serialize();
    //     Command lbw_cmd(0x4001, tmp_vec.size());
    //     lbw_cmd.arguments = std::move(tmp_vec);
    //     status_client_.WriteSendBuffer(lbw_cmd);
    //
    //     std::cout << "Sent metrics.." << std::endl;
    // }

    void DataMonitor::SendMetric(std::vector<uint32_t> &metric_vec, uint32_t metric_id) {
        // Send the metrics
        Command lbw_cmd(metric_id, metric_vec.size());
        lbw_cmd.arguments = std::move(metric_vec);
        status_client_.WriteSendBuffer(lbw_cmd);
        std::cout << "Sent metrics.." << std::endl;
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

        auto tmp_vec = lbw_metrics_.serialize();
        SendMetric(tmp_vec, 0x4001);

        if (debug_) std::cout << "Updated light.." << std::endl;
        if (debug_) lbw_metrics_.print();
        // Clear the metrics for the next file
        charge_algs_.Clear();
        light_algs_.Clear();
    }

    void DataMonitor::CreateEventMetrics(EventStruct & event) {
        charge_algs_.GetChargeEvent(event);
        if (debug_) std::cout << "Processed charge event.." << std::endl;
        //num_light_rois_ = light_algs_.GetLightEvent(event);
        if (debug_) std::cout << "Processed light event.." << std::endl;
    }

    void DataMonitor::UpdateEventMetrics() {
        if (debug_) std::cout << "Updating Event Metrics.." << std::endl;
        if (choose_random_) {
            auto charge_uniform = std::uniform_int_distribution<size_t>(0, NUM_CHARGE_CHANNELS);
            size_t charge_channel = charge_uniform(random_generator_);
            if (debug_) std::cout << "Random charge ch: " << charge_channel << std::endl;
            auto tmp_vec = charge_algs_.UpdateChargeEvent(charge_event_metric_, charge_channel);
            SendMetric(tmp_vec, 0x4002);

            auto light_uniform = std::uniform_int_distribution<size_t>(0, num_light_rois_);
            size_t light_roi = light_uniform(random_generator_);
            if (debug_) std::cout << "Random light roi: " << light_roi << std::endl;
            if (light_algs_.isLightRoi()) {
                tmp_vec = light_algs_.UpdateLightEvent(light_event_metric_, light_roi);
                SendMetric(tmp_vec, 0x4003);
            }
        } else {
            for (size_t i = 0; i < NUM_CHARGE_CHANNELS; i++) {
                auto tmp_vec = charge_algs_.UpdateChargeEvent(charge_event_metric_, i);
                if (debug_) std::cout << "Updated charge event.." << std::endl;
                SendMetric(tmp_vec, 0x4002);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            for (size_t i = 0; i < num_light_rois_; i++) {
                auto tmp_vec = light_algs_.UpdateLightEvent(light_event_metric_, i);
                if (debug_) std::cout << "Updated light event.." << std::endl;
                SendMetric(tmp_vec, 0x4003);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        // Make sure to clear it
        charge_algs_.Clear();
        light_algs_.Clear();
    }

} // data_monitor
