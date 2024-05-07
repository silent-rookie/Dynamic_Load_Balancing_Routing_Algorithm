/**
 * Author: silent-rookie    2024
*/



#ifndef ARBITER_GEO_H
#define ARBITER_GEO_H

#include "ns3/arbiter-satnet.h"

namespace ns3{

class ArbiterLEOGSGEOHelper; 

class ArbiterGEO: public ArbiterSatnet
{
public:
    static TypeId GetTypeId (void);

    ArbiterGEO(
            Ptr<Node> this_node,
            NodeContainer nodes,
            Ptr<ArbiterLEOGSGEOHelper> arbiter_helper
    );

    static void InitializeArbiter(Ptr<BasicSimulation> basicSimulation, int64_t num_sat, int64_t num_gs, int64_t num_geo);

    // LEOGEO forward next-hop implementation
    std::tuple<int32_t, int32_t, int32_t> TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    );

    // find the next leo to for GEOsatellite
    std::tuple<int32_t, int32_t, int32_t> FindNextHopForGEO(int32_t from, int32_t target_node_id);

    /**
     * \brief push the next hop information
     * \param pkt the forward packet
     * \param from the packet forward from
     * \param to the packet send to
    */
    void PushGEONextHop(ns3::Ptr<const ns3::Packet> pkt, int32_t from);

    std::string StringReprOfForwardingState();

private:
    // update detour information each interval: receive_datarate_update_interval_ns
    void UpdateReceiveDatarate();

    Ptr<ArbiterLEOGSGEOHelper> m_arbiter_helper;
    std::unordered_map<uint64_t , int32_t> m_geo_pkt_next_hop_map;

    static int64_t receive_datarate_update_interval_ns;     // the interval that a netdevice receive datarate update
    static double ill_data_rate_megabit_per_s;
    static int64_t num_satellites;
    static int64_t num_groundstations;
    static int64_t num_GEOsatellites;
};

}



#endif