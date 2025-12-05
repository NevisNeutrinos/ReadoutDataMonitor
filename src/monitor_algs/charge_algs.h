//
// Created by Jon Sensenig on 8/20/25.
//

#ifndef CHARGE_ALGS_H
#define CHARGE_ALGS_H

#include "monitor_algs_base.hpp"

class ChargeAlgs : public MonitorAlgBase {
public:
    ChargeAlgs() = default;
    ~ChargeAlgs() override = default;

    bool ProcessEvent(EventStruct &event) override;
    bool UpdateMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) override;
    void Clear() override;

    void MinimalSummary(EventStruct &event);
    void UpdateMinimalMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics);
    void BaselineRms(const std::vector<uint16_t> &channel_charge_words, uint16_t channel);
    void HitsAboveThreshold(const std::vector<uint16_t> &channel_charge_words, uint16_t channel);


private:

    // Static variables
    constexpr static size_t NUM_CHANNELS = 192;
    constexpr static size_t NUM_SAMPLES = 763;

    Histogram charge_histogram_{1024, 4096, 16};

    std::array<double, NUM_CHANNELS> rms_{0};
    std::array<double, NUM_CHANNELS> baseline_{0};
    std::array<size_t, NUM_CHANNELS> charge_hits_{0};
    size_t num_events_ = 0;

};

#endif //CHARGE_ALGS_H
