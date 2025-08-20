//
// Created by Jon Sensenig on 8/20/25.
//

#include "charge_algs.h"

ChargeAlgs::ChargeAlgs() {};

bool ChargeAlgs::ProcessEvent(EventStruct &event, Metric_Struct &metrics) {

    metrics.num_fems = event.slot_number.size();
    metrics.num_charge_channels = event.charge_adc.size();

    ChargeChannelDistribution(event.charge_adc, event.charge_channel, metrics);
    return true;
}

void ChargeAlgs::ChargeChannelDistribution(const std::vector<std::vector<uint16_t>> &charge_words,
                                           const std::vector<uint16_t> &charge_channels, Metric_Struct &metrics) {

    for (const auto &channel : charge_channels) {
        metrics.charge_channel_num_samples.at(channel) = charge_words.at(channel).size();
        for (const auto &word : charge_words.at(channel)) {
            metrics.charge_histograms.at(channel).fill(word);
        }
    }
}

void ChargeAlgs::Clear() {
}