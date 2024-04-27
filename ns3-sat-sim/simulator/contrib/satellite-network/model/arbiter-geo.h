/**
 * Author: silent-rookie    2024
*/



#ifndef ARBITER_GEO_H
#define ARBITER_GEO_H

#include "ns3/arbiter-satnet.h"

namespace ns3{

class ArbiterLEOGEOHelper; 

class ArbiterGEO: public ArbiterSatnet
{
public:
    static TypeId GetTypeId (void);

    ArbiterGEO(
            Ptr<Node> this_node,
            NodeContainer nodes,
            ArbiterLEOGEOHelper* arbiter_leo_geo_helper
    );

    // LEOGEO forward next-hop implementation
    std::tuple<int32_t, int32_t, int32_t> TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    );

    /**
     * \brief push the next hop information
     * \param pkt the forward packet
     * \param from the packet forward from
     * \param to the packet send to
    */
    void PushGEONextHop(ns3::Ptr<const ns3::Packet> pkt, int32_t from);

    std::string StringReprOfForwardingState();

private:
    ArbiterLEOGEOHelper* m_arbiter_leo_geo_helper;
    std::unordered_map<uint64_t , int32_t> m_geo_pkt_next_hop_map;
};

}



#endif