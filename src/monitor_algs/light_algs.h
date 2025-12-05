//
// Created by Jon Sensenig on 8/20/25.
//

#ifndef LIGHT_ALGS_H
#define LIGHT_ALGS_H

#include "monitor_algs_base.hpp"

class LightAlgs : public MonitorAlgBase {
public:
    LightAlgs() = default;
    ~LightAlgs() override = default;

    bool ProcessEvent(EventStruct &event) override;
    void Clear() override;
    bool UpdateMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) override;


    void MinimalSummary(EventStruct& event);
    void UpdateMinimalMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics);

    private:

    void BaselineRms(const std::vector<uint16_t> &channel_charge_words, uint16_t channel);

    std::array<double, NUM_LIGHT_CHANNELS> rms_{0};
    std::array<double, NUM_LIGHT_CHANNELS> baseline_{0};
    std::array<size_t, NUM_LIGHT_CHANNELS> light_rois_{0};
    size_t num_events_ = 0;

};

#endif //LIGHT_ALGS_H
