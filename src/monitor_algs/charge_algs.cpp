//
// Created by Jon Sensenig on 8/20/25.
//

#include "charge_algs.h"

ChargeAlgs::ChargeAlgs() {}

bool ChargeAlgs::ProcessEvent(EventStruct &event, LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {

    lbw_metrics.num_fems = event.slot_number.size();
    lbw_metrics.num_charge_channels = event.charge_adc.size();

    ChargeChannelDistribution(event.charge_adc, event.charge_channel, lbw_metrics, metrics);
    return true;
}

void ChargeAlgs::ChargeChannelDistribution(const std::vector<std::vector<uint16_t>> &charge_words,
                                           const std::vector<uint16_t> &charge_channels,
                                           LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {

    for (const auto &channel : charge_channels) {
        lbw_metrics.charge_channel_num_samples.at(channel) = charge_words.at(channel).size();
        for (const auto &word : charge_words.at(channel)) {
            metrics.charge_histograms.at(channel).fill(word);
        }
    }
}

void ChargeAlgs::Clear() {
}