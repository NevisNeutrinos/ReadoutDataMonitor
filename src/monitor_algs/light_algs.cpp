//
// Created by Jon Sensenig on 8/20/25.
//

#include "light_algs.h"
#include <set>

LightAlgs::LightAlgs() {};

bool LightAlgs::ProcessEvent(EventStruct &event, Metric_Struct &metrics) {

    // Get the number of light channels
    std::set<uint16_t> unique_channels(event.light_channel.begin(), event.light_channel.end());
    metrics.num_light_channels = unique_channels.size();

    return true;
}

void LightAlgs::Clear() {
}
