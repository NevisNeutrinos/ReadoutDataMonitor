//
// Created by Jon Sensenig on 8/20/25.
//

#include "charge_algs.h"

#include <cmath>
#include <numeric>
#include <algorithm>


bool ChargeAlgs::ProcessEvent(EventStruct &event) {
    return true;
}

void ChargeAlgs::MinimalSummary(EventStruct &event) {

    for (const auto &channel : event.charge_channel) {
        BaselineRms(event.charge_adc.at(channel), channel);
        HitsAboveThreshold(event.charge_adc.at(channel), channel); // needs to always follow baseline & RMS
    }
}

void ChargeAlgs::BaselineRms(const std::vector<uint16_t> &channel_charge_words, uint16_t channel) {
    // Calculate the baseline and RMS for the channel, only use the first 5 samples
    size_t num_samples = 5;

    for (size_t i = 0; i < num_samples; i++) {
        baseline_[channel] += channel_charge_words[i];
        rms_[channel] += channel_charge_words[i] * channel_charge_words[i];
    }
    baseline_[channel] /= num_samples;
    rms_[channel] /= num_samples;
}

void ChargeAlgs::HitsAboveThreshold(const std::vector<uint16_t> &channel_charge_words, uint16_t channel) {
    // Use baseline shifted threshold instead of baseline subtraction to avoid pesky 16b int overflows
    double rms_threshold = 2.0;
    double threshold = baseline_[channel] + rms_threshold * rms_[channel];

    for (auto adc_word : channel_charge_words) {
        if (adc_word > threshold) charge_hits_[channel]++;
    }
}

void ChargeAlgs::UpdateMinimalMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {

    if (num_events_ < 1) { num_events_ = 1; } // Avoid divide by 0
    /*
     * Finish the aggregated Baseline & RMS calculation and update the metrics
     */

    std::vector<int> baseline_int(baseline_.size());
    std::vector<int> rms_int(rms_.size());
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        baseline_int[i] = static_cast<int>(baseline_[i] / num_events_);
        // TODO could perform the sqrt on ground for safety and efficiency
        // check to make sure rms is non-negative, should never be but better to avoid NaN
        rms_int[i] = static_cast<int>((rms_[i] < 0) ? INT32_MAX : std::sqrt(rms_[i] / num_events_));
    }

    // TODO add to metrics

    /*
     * Average hits per event and the charge hits to the metrics
     */
    for (auto &hits : charge_hits_) { hits /= num_events_; }

}

void ChargeAlgs::Clear() {
    // Clear the metrics between queries
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        baseline_[i] = 0;
        rms_[i] = 0;
        charge_hits_[i] = 0;
    }
    num_events_ = 0;
}

bool ChargeAlgs::UpdateMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {
    return true;
}
