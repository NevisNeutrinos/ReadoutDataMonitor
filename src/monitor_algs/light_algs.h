//
// Created by Jon Sensenig on 8/20/25.
//

#ifndef LIGHT_ALGS_H
#define LIGHT_ALGS_H

#include "monitor_algs_base.hpp"

class LightAlgs : public MonitorAlgBase {
public:
        LightAlgs();
        ~LightAlgs() override = default;

        bool ProcessEvent(EventStruct &event, Metric_Struct &metrics) override;
        void Clear() override;
        void LightChannelDistribution(const std::vector<std::vector<uint16_t>> &light_words,
                                      const std::vector<uint16_t> &light_channels, Metric_Struct &metrics);

    private:
};



#endif //LIGHT_ALGS_H
