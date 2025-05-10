//
// Created by Jon Sensenig on 5/6/25.
//

#ifndef METRICS_BASE_H
#define METRICS_BASE_H

#include "../readout_decoder/src/process_events.h"

class MetricsBase {

    MetricsBase();
    virtual ~MetricsBase() = default;

    /**
    *  Abstract method which shall implement the concrete metrics
    *  algorithms.
    *
    * @return  Returns true on success, false on failure.
    */
    virtual bool ProcessEvent(EventStruct &event) = 0;

    /**
    *  Abstract method which shall implement all required shutdown procedures to
    *  safely close the hardware and handle any errors which might occur.
    *
    * @return  Returns true on successful shutdown, false on failure.
    */
    virtual bool Reset() = 0;
};

#endif //METRICS_BASE_H
