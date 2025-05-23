//
// Created by Jon Sensenig on 5/5/25.
//

#ifndef DATA_MONITOR_H
#define DATA_MONITOR_H

#include "tcp_connection.h"
#include "process_events.h"
#include <random>
#include <atomic>
#include <thread>

namespace data_monitor {

class DataMonitor {
public:

    DataMonitor(asio::io_context& io_context, const std::string& ip_address,
                short port, bool is_server, bool is_running);
    ~DataMonitor() = default;

    void SetRunning(bool run);

    // FIXME shoulf be private, public for testing
    void OpenFile();
    void DecodeEvent(std::vector<int32_t>& args);
    void SelectEvents();
    uint16_t SelectChargeChannel();
    size_t SelectLightChannel();
    void GetEvents(uint16_t charge_channel, uint16_t light_channel);
    void ReceiveCommand();

private:

    void HandleCommand(Command& cmd);
    void RunMetrics();
    void RunDecoder();
    void StopDecoder();

    TCPConnection tcp_connection_;
    std::unique_ptr<ProcessEvents> process_events_;

    // Seed the random number generator
    // std::random_device rd;
    std::mt19937 random_generator_;

    // Create a uniform integer distribution object
    constexpr static uint16_t light_slot_ = 16;
    const size_t events_per_file = 5;
    std::vector<size_t> selected_events_;
    constexpr static int event_min = 0, event_max = 5000;
    constexpr static int charge_min = 0, charge_max = 188;
    constexpr static int light_min = 0, light_max = 32;
    std::uniform_int_distribution<size_t> event_distrib_;
    std::uniform_int_distribution<uint16_t> charge_channel_distrib_;
    std::uniform_int_distribution<uint16_t> light_channel_distrib_;

    std::atomic_bool is_running_;
    std::atomic_bool is_decoding_;

    std::thread decode_thread_;

    enum ControlCmds : uint16_t {
        kRunDecoder = 1,
        kStopDecoder = 2,
        kDecodeEvent = 3
    };

};

} // data_monitor

#endif //DATA_MONITOR_H
