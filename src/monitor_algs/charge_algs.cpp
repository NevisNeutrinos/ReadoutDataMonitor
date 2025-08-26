//
// Created by Jon Sensenig on 8/20/25.
//

#include "charge_algs.h"


bool ChargeAlgs::ProcessEvent(EventStruct &event, LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {

    lbw_metrics.setNumFems(event.slot_number.size());
    lbw_metrics.setNumChargeChannels(event.charge_adc.size());

    ChargeChannelDistribution(event.charge_adc, event.charge_channel, lbw_metrics, metrics);
    return true;
}

void ChargeAlgs::ChargeChannelDistribution(const std::vector<std::vector<uint16_t>> &charge_words,
                                           const std::vector<uint16_t> &charge_channels,
                                           LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {

    for (const auto &channel : charge_channels) {
        lbw_metrics.setChargeChannelSamples(channel, charge_words.at(channel).size());
        for (const auto &word : charge_words.at(channel)) {
            metrics.fillChargeChannelHistogram(channel, word);
        }
    }
}

void ChargeAlgs::Clear() {
}