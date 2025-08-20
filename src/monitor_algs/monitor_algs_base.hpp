//
// Created by Jon Sensenig on 8/20/25.
//

#ifndef MONITOR_ALGS_BASE_HPP
#define MONITOR_ALGS_BASE_HPP

#include "process_events.h"
#include "metrics.h"

class MonitorAlgBase {
public:

    MonitorAlgBase() = default;
    virtual ~MonitorAlgBase() = default;

    /**
    *  Abstract method which will be the common interface to the monitoring algorithm.
    *
    *   @param [in] event:  The EvenStruct holding the decoded event data
    *
    * @return  Returns true on successful configuration, false on failure.
    */
    virtual bool ProcessEvent(EventStruct &event, Metric_Struct &metrics) = 0;

    /**
    *  Abstract method to clear a persistant state, e.g. counters, vectors.
    *
    * @return
    */
    virtual void Clear() = 0;

private:

};

#endif //MONITOR_ALGS_BASE_HPP
