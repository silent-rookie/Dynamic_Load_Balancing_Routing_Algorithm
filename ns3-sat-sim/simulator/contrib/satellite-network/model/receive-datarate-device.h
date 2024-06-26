/**
 * Author: rookie   2024
*/

#ifndef RECEIVE_DATARATE_DEVICE_H
#define RECEIVE_DATARATE_DEVICE_H

#include "ns3/data-rate.h"


namespace ns3{

class ReceiveDataRateDevice{
public:
    ReceiveDataRateDevice();

    /**
     * \brief update reveive datarate
     * we update each device receive datarate in each m_receive_datarate_update_interval_ns
     * it is update in ArbiterLEOGS::UpdateDetour
    */
    void UpdateReceiveDataRate();
    // void UpdateReceiveDateRate(int64_t current_time);

    static void SetReceiveDatarateUpdateIntervalNS(int64_t receive_datarate_update_interval_ns);
    // void SetSimulateEndTimeNS(int64_t simulate_end_time_ns);

    DataRate GetReceiveDataRate();
protected:
    uint32_t receive_bytes;
    DataRate receive_rate;
    static int64_t m_receive_datarate_update_interval_ns;
    // int64_t m_simulate_end_time_ns;
};

}

#endif