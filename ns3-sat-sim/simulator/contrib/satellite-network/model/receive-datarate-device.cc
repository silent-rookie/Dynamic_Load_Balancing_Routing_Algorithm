/**
 * Author: rookie   2024
*/

#include "ns3/basic-simulation.h"
#include "receive-datarate-device.h"



namespace ns3{

NS_LOG_COMPONENT_DEFINE ("ReceiveDataRateDevice");

int64_t ReceiveDataRateDevice::m_receive_datarate_update_interval_ns = 0;

ReceiveDataRateDevice::ReceiveDataRateDevice(): receive_bytes(0) {

}

void ReceiveDataRateDevice::UpdateReceiveDataRate(){
    // calculte bps
    NS_ABORT_MSG_IF(m_receive_datarate_update_interval_ns == 0, "receive_datarate_update_interval_ns == 0");
    double seconds = (double)m_receive_datarate_update_interval_ns / 1000 / 1000 / 1000;
    uint64_t now_bps = receive_bytes * 8 / seconds;
    receive_rate = DataRate(now_bps);
    receive_bytes = 0;
}

// void ReceiveDataRateDevice::UpdateReceiveDateRate(int64_t current_time){
//     // calculte bps
//     uint64_t now_bps = receive_bytes * 8 / m_receive_datarate_update_interval_ns * 1000 * 1000 * 1000;
//     receive_rate = DataRate(now_bps);
//     receive_bytes = 0;

//     // plan the next update
//     int64_t next_update_ns = current_time + m_receive_datarate_update_interval_ns;
//     if (next_update_ns < m_simulate_end_time_ns) {
//         Simulator::Schedule(
//           NanoSeconds(m_receive_datarate_update_interval_ns), &ReceiveDataRateDevice::UpdateReceiveDateRate, this, next_update_ns);
//     }
// }

void ReceiveDataRateDevice::SetReceiveDatarateUpdateIntervalNS(int64_t receive_datarate_update_interval_ns) {
    m_receive_datarate_update_interval_ns = receive_datarate_update_interval_ns;
}

// void ReceiveDataRateDevice::SetSimulateEndTimeNS(int64_t simulate_end_time_ns) {
//     m_simulate_end_time_ns = simulate_end_time_ns;
// }

DataRate ReceiveDataRateDevice::GetReceiveDataRate() { 
    return receive_rate; 
}

}