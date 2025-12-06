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
    num_events_++;
}

void ChargeAlgs::BaselineRms(const std::vector<uint16_t> &channel_charge_words, uint16_t channel) {
    // Calculate the baseline and RMS for the channel, only use the first 5 samples
    double num_samples = 5;

    double baseline_sum = 0;
    double rms_sum = 0;
    for (size_t i = 0; i < num_samples; i++) {
        baseline_sum += channel_charge_words[i];
        rms_sum += (channel_charge_words[i] * channel_charge_words[i]);
    }
    baseline_sum /= num_samples;
    rms_sum /= num_samples;

    baseline_[channel] += baseline_sum;
    rms_[channel] += rms_sum;
}

void ChargeAlgs::HitsAboveThreshold(const std::vector<uint16_t> &channel_charge_words, uint16_t channel) {
    // Use baseline shifted threshold instead of baseline subtraction to avoid pesky 16b int overflows
    double rms_threshold = 3.0;
    double threshold = baseline_[channel] + rms_threshold * abs(rms_[channel] - baseline_[channel]);

    for (auto adc_word : channel_charge_words) {
        if (adc_word > threshold) charge_hits_[channel]++;
    }
}

void ChargeAlgs::UpdateMinimalMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {

    if (num_events_ < 1) { num_events_ = 1; } // Avoid divide by 0
    /*
     * Finish the aggregated Baseline & RMS calculation and update the metrics
     * Average hits per event and the charge hits to the metrics
     */
    std::cout << "num_events_ = " << num_events_ << std::endl;
    std::array<int32_t, NUM_CHARGE_CHANNELS> baseline_int;
    std::array<int32_t, NUM_CHARGE_CHANNELS> rms_int;
    std::array<int32_t, NUM_CHARGE_CHANNELS> avg_hits_int;
    for (size_t i = 0; i < NUM_CHARGE_CHANNELS; i++) {
        // if (i < 10) std::cout << i << ":" << baseline_[i] / num_events_ << "|" << rms_[i] / num_events_ << "|" << charge_hits_[i] << std::endl;
        baseline_int[i] = static_cast<int32_t>(baseline_[i] / num_events_);
        // TODO could perform the sqrt on ground for safety and efficiency
        // check to make sure rms is non-negative, should never be but better to avoid NaN
        rms_int[i] = static_cast<int32_t>((rms_[i] < 0) ? INT32_MAX : std::sqrt(rms_[i] / num_events_));
        avg_hits_int[i] = static_cast<int32_t>(charge_hits_[i] / num_events_);
        // if (i < 10) std::cout << "  ->" << baseline_int[i] << "|" << rms_int[i] << "|" << avg_hits_int[i] << std::endl;
    }

    // Update the metrics
    lbw_metrics.setChargeBaselines(baseline_int);
    lbw_metrics.setChargeRms(rms_int);
    lbw_metrics.setAvgNumHits(avg_hits_int);

}

void ChargeAlgs::Clear() {
    // Clear the metrics between queries
    for (size_t i = 0; i < NUM_CHARGE_CHANNELS; i++) {
        baseline_[i] = 0;
        rms_[i] = 0;
        charge_hits_[i] = 0;
    }
    num_events_ = 0;
}

bool ChargeAlgs::UpdateMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {
    return true;
}
