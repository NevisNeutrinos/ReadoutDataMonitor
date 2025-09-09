//
// Created by Jon Sensenig on 8/20/25.
//

#include "charge_algs.h"

#include <cmath>
#include <numeric>


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
        auto [mean, stddev] = MeanStdDev(charge_words.at(channel));
        for (const auto &word : charge_words.at(channel)) {
            metrics.fillChargeChannelHistogram(channel, word);
        }
    }
}

std::pair<int, int> ChargeAlgs::MeanStdDev(const std::vector<uint16_t> &channel_charge_words) {
    // Calculate the mean for the channel
    double mean = std::accumulate(channel_charge_words.begin(), channel_charge_words.end(), 0);
    mean /= channel_charge_words.size();

    // Reserve the vector for the squared difference.
    std::vector<double> square_difference;
    square_difference.reserve(channel_charge_words.size());

    /** Calculate standard deviation
     * 1. Calculate the difference between each element and the mean: (x - mu)
     * 2. Square the result: (x - mu) * (x - mu)
     * 3. Sum result SUM((x - mu) * (x - mu))
     */

    // 1. & 2. Calculate the squared difference between each element and the mean: (x - mu) * (x - mu)
    std::transform(channel_charge_words.begin(), channel_charge_words.end(),
        square_difference.begin(), // write results into this vector
        [mean](int element) { return (element - mean) * (element - mean); }
    );

    // 3. Sum of squared differences
    double variance = std::accumulate(square_difference.begin(), square_difference.end(), 0.0);
    variance /= channel_charge_words.size();

    // Finally since we are returning a signed int, we cast to it. We also know the range
    // of inputs is [0,4095] so the mean and standard deviation will always be 0 <= X <= 4095
    int mean_int = static_cast<int>(mean);
    int stddev_int = static_cast<int>(std::sqrt(variance));

    return std::pair<int, int>(mean_int, stddev_int);
}

void ChargeAlgs::Clear() {
}