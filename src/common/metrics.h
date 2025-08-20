//
// Created by Jon Sensenig on 8/20/25.
//

#ifndef METRICS_H
#define METRICS_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <cmath>


struct Metric_Struct {
    uint32_t num_fems;
    uint32_t num_charge_channels;
    uint32_t num_light_channels;
};


/**
 * @struct Histogram
 * @brief A histogram with a predefined range and number of bins.
 *
 * This struct is optimized for speed and memory by using a std::vector to store
 * bin counts. It requires a specified range [min, max] and a number of bins
 * upon creation. It also tracks values that fall outside this range.
 */
struct Histogram {
    // Configuration
    int32_t min_value;
    int32_t max_value;
    int32_t num_bins;
    double bin_width;

    // Data storage
    std::vector<int32_t> bins;
    int32_t below_range_count;
    int32_t above_range_count;

    /**
     * @brief Constructs a Histogram with a defined range and bin count.
     * @param min The minimum value of the range (inclusive).
     * @param max The maximum value of the range (exclusive).
     * @param bins_count The number of bins to create within the range.
     * @throws std::invalid_argument if max <= min or bins_count is zero or negative.
     */
    Histogram(int32_t min, int32_t max, int32_t bins_count)
        : min_value(min), max_value(max), num_bins(bins_count), below_range_count(0), above_range_count(0) {
        if (max <= min) {
            throw std::invalid_argument("max_value must be greater than min_value.");
        }
        if (bins_count <= 0) {
            throw std::invalid_argument("Number of bins must be positive.");
        }
        bins.resize(num_bins, 0);
        bin_width = static_cast<double>(max_value - min_value) / num_bins;
    }

    void fill(int32_t value) {
        if (value < min_value) {
            below_range_count++;
        } else if (value >= max_value) {
            above_range_count++;
        } else {
            // Calculate the bin index
            int bin_index = static_cast<int>(std::floor((value - min_value) / bin_width));
            // Ensure the index is within bounds [0, num_bins - 1]
            if (bin_index >= 0 && bin_index < num_bins) {
                bins[bin_index]++;
            }
        }
    }

    std::vector<int32_t> serialize() const {
        // Start with the configuration metadata
        std::vector<int32_t> serialized_data = {
            min_value,
            max_value,
            num_bins,
            below_range_count,
            above_range_count
        };
        // Append the bin counts
        serialized_data.insert(serialized_data.end(), bins.begin(), bins.end());
        return serialized_data;
    }

    static Histogram deserialize(const std::vector<int32_t>& data) {
        // The vector must contain at least the 5 metadata fields.
        if (data.size() < 5) {
            throw std::runtime_error("Deserialization failed: data is too short for metadata.");
        }

        int32_t min_val = data[0];
        int32_t max_val = data[1];
        int32_t bins_count = data[2];

        // Check if the total size matches the expected size from metadata.
        if (data.size() != 5 + static_cast<size_t>(bins_count)) {
             throw std::runtime_error("Deserialization failed: data size does not match metadata.");
        }

        // Create a new histogram with the deserialized configuration
        Histogram hist(min_val, max_val, bins_count);
        hist.below_range_count = data[3];
        hist.above_range_count = data[4];

        // Copy the bin counts
        for (int i = 0; i < bins_count; ++i) {
            hist.bins[i] = data[5 + i];
        }

        return hist;
    }

    /**
     * @brief Prints the contents of the histogram to the console.
     */
    void print() const {
        std::cout << "--- Histogram ---" << std::endl;
        std::cout << "Range: [" << min_value << ", " << max_value << "), Bins: " << num_bins << std::endl;
        std::cout << "Values below range (<" << min_value << "): " << below_range_count << std::endl;
        for (int i = 0; i < num_bins; ++i) {
            double bin_start = min_value + i * bin_width;
            double bin_end = bin_start + bin_width;
            std::cout << "Bin " << i << " [" << bin_start << ", " << bin_end << "): "
                      << std::string(bins[i], '*') << " (" << bins[i] << ")" << std::endl;
        }
        std::cout << "Values above range (>=" << max_value << "): " << above_range_count << std::endl;
        std::cout << "-----------------" << std::endl;
    }
};

#endif //METRICS_H
