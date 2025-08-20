//
// Created by Jon Sensenig on 8/20/25.
//

#ifndef CHARGE_ALGS_H
#define CHARGE_ALGS_H

#include "monitor_algs_base.hpp"
#include "metrics.h"

class ChargeAlgs : public MonitorAlgBase {
public:
    ChargeAlgs();
    ~ChargeAlgs() override = default;

    bool ProcessEvent(EventStruct &event, Metric_Struct &metrics) override;
    void Clear() override;
    void ChargeChannelDistribution(const std::vector<std::vector<uint16_t>> &charge_words,
                                   const std::vector<uint16_t> &charge_channels, Metric_Struct &metrics);


private:

    // Static variables
    constexpr static size_t NUM_CHANNELS = 192;
    constexpr static size_t NUM_SAMPLES = 763;

    Histogram charge_histogram_{1024, 4096, 16};

};

#endif //CHARGE_ALGS_H
