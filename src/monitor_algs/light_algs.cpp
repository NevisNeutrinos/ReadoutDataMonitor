//
// Created by Jon Sensenig on 8/20/25.
//

#include "light_algs.h"
#include <set>

LightAlgs::LightAlgs() {}

bool LightAlgs::ProcessEvent(EventStruct &event, LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {

    // Get the number of light channels
    std::set<uint16_t> unique_channels(event.light_channel.begin(), event.light_channel.end());
    lbw_metrics.num_light_channels = unique_channels.size();
    LightChannelDistribution(event.light_adc, event.light_channel, lbw_metrics, metrics);

    return true;
}

void LightAlgs::LightChannelDistribution(const std::vector<std::vector<uint16_t>> &light_words,
                                        const std::vector<uint16_t> &light_channels,
                                        LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {

    for (const auto &channel : light_channels) {
        for (const auto &word : light_words.at(channel)) {
            metrics.light_histograms.at(channel).fill(word);
        }
    }
}


void LightAlgs::Clear() {
}
