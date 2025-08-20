//
// Created by Jon Sensenig on 8/20/25.
//

#include "charge_algs.h"

ChargeAlgs::ChargeAlgs() {};

bool ChargeAlgs::ProcessEvent(EventStruct &event, Metric_Struct &metrics) {

    metrics.num_fems = event.slot_number.size();
    metrics.num_charge_channels = event.charge_adc.size();

    ChargeChannelDistribution(event.charge_adc);
    return true;
}

void ChargeAlgs::Clear() {
}

void ChargeAlgs::ChargeChannelDistribution(const std::vector<std::vector<uint16_t>> &charge_words) {

    for (const auto &channel : charge_words) {
        for (const auto &word : channel) {
            charge_histogram_.fill(word);
        }
    }

}