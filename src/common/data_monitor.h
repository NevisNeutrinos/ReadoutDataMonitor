//
// Created by Jon Sensenig on 5/5/25.
//

#ifndef DATA_MONITOR_H
#define DATA_MONITOR_H

#include <random>

namespace data_monitor {

class DataMonitor {
public:

    DataMonitor(int charge_min, int charge_max, int light_min, int light_max);
    ~DataMonitor() = default;

    bool RunDataMonitor();

    // FIXME shoulf be private, public for testing
    void OpenFile();
    void DecodeEvent();
    size_t SelectEvent();
    size_t SelectChargeChannel();
    size_t SelectLightChannel();
    void ProcessEvents();

private:

    // Seed the random number generator
    // std::random_device rd;
    std::mt19937 random_generator_;

    // Create a uniform integer distribution object
    std::uniform_int_distribution<size_t> charge_channel_distrib_;
    std::uniform_int_distribution<size_t> light_channel_distrib_;

};

} // data_monitor

#endif //DATA_MONITOR_H
