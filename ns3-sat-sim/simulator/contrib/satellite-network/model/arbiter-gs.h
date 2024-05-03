/**
 * Author:  silent-rookie      2024
*/



#ifndef ARBITER_GS_H
#define ARBITER_GS_H

#include "ns3/arbiter-satnet.h"
#include "ns3/abort.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"

namespace ns3 {

class ArbiterLEOGSGEOHelper;

class ArbiterGS : public ArbiterSatnet
{
public:
    static TypeId GetTypeId (void);

    ArbiterGS(
            Ptr<Node> this_node,
            NodeContainer nodes,
            std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> next_hop_lists,
            ArbiterLEOGSGEOHelper* arbiter_helper
    );

    static void Initialize(Ptr<BasicSimulation> basicSimulation, int64_t num_sat, int64_t num_gs, int64_t num_geo);

    // LEOGEO forward next-hop implementation
    std::tuple<int32_t, int32_t, int32_t> TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    );

    // Update the forward state
    void SetGSForwardState(int32_t target_node_id, std::vector<std::tuple<int32_t, int32_t, int32_t>> next_hop_list);

    std::vector<std::tuple<int32_t, int32_t, int32_t>> GetGSForwardState(int32_t target_node_id);

    std::string StringReprOfForwardingState();

private:
    // update detour information each interval: receive_datarate_update_interval_ns
    void UpdateReceiveDatarate();

protected:
    ArbiterLEOGSGEOHelper* m_arbiter_helper;
    std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> m_next_hop_lists;

    static int64_t receive_datarate_update_interval_ns;     // the interval that a netdevice receive datarate update
    static double gsl_data_rate_megabit_per_s;
    static int64_t num_satellites;
    static int64_t num_groundstations;
    static int64_t num_GEOsatellites;
};

}

#endif //ARBITER_LEO_GEO_H