//
// Created by Jon Sensenig on 8/20/25.
//

#include "light_algs.h"
#include <cmath>


bool LightAlgs::ProcessEvent(EventStruct &event) {

    // Get the number of light channels

    return true;
}

void LightAlgs::MinimalSummary(EventStruct &event) {
    // The unbiased light readout corresponds to ID 0x4. We want to use this to get an unbiased snapshot
    // of channel baseline & RMS loosely correlated with the trigger since it initiates the unbiased readout
    uint8_t beam_gate_id = 0x4;

    std::cout << "Size ID/Ch/ROI: " << event.light_trigger_id.size() << "/"
            << event.light_channel.size() << "/" << event.light_adc.size() << std::endl;

    for (size_t i = 0; i < event.light_channel.size(); i++) { // loop over each RI in the event
        if (event.light_channel[i] > NUM_LIGHT_CHANNELS-1) continue;
        if (event.light_trigger_id.at(i) != beam_gate_id) { // should be a Cosmic ie Disc 1 ROI
            light_rois_[event.light_channel.at(i)]++;
            continue; // skip non-beam gate readout ROIs
        }
        BaselineRms(event.light_adc.at(i), event.light_channel.at(i));
    }
}

void LightAlgs::BaselineRms(const std::vector<uint16_t> &channel_charge_words, uint16_t channel) {
    // Assuming we are receiving the unbiased light readout ROI
    // Calculate the baseline and RMS for the channel, only use the first 8 samples unless
    // there are <8 but shouldn't happen
    size_t num_samples = channel_charge_words.size() > 7 ? 8 : channel_charge_words.size();
    if (num_samples < 1) return;

    for (size_t i = 0; i < num_samples; i++) {
        baseline_[channel] += channel_charge_words[i];
        rms_[channel] += channel_charge_words[i] * channel_charge_words[i];
    }
    baseline_[channel] /= num_samples;
    rms_[channel] /= num_samples;
}


void LightAlgs::UpdateMinimalMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {
    if (num_events_ < 1) { num_events_ = 1; } // Avoid divide by 0
    /*
     * Finish the aggregated Baseline & RMS calculation and update the metrics
     */

    std::vector<int> baseline_int(baseline_.size());
    std::vector<int> rms_int(rms_.size());
    for (size_t i = 0; i < NUM_LIGHT_CHANNELS; i++) {
        baseline_int[i] = static_cast<int>(baseline_[i] / num_events_);
        // TODO could perform the sqrt on ground for safety and efficiency
        // check to make sure rms is non-negative, should never be but better to avoid NaN
        rms_int[i] = static_cast<int>((rms_[i] < 0) ? INT32_MAX : std::sqrt(rms_[i] / num_events_));
    }

    // TODO add to metrics

    /*
     * Average hits per event and the charge hits to the metrics
     */
    for (auto &rois : light_rois_) { rois /= num_events_; }
}


bool LightAlgs::UpdateMetrics(LowBwTpcMonitor &lbw_metrics, TpcMonitor &metrics) {
    return true;
}


void LightAlgs::Clear() {
    for (size_t i = 0; i < NUM_LIGHT_CHANNELS; i++) {
        rms_[i] = 0;
        baseline_[i] = 0;
        light_rois_[i] = 0;
    }
}
