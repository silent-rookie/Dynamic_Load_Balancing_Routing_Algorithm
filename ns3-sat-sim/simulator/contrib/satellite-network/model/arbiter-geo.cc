/**
 * Author: silent-rookie    2024
*/

#include "arbiter-geo.h"
#include "ns3/arbiter-leo-geo-helper.h"



namespace ns3{



NS_OBJECT_ENSURE_REGISTERED (ArbiterGEO);

TypeId ArbiterGEO::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ArbiterGEO")
            .SetParent<ArbiterSatnet> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}


ArbiterGEO::ArbiterGEO(
            Ptr<Node> this_node,
            NodeContainer nodes,
            ArbiterLEOGEOHelper* arbiter_leo_geo_helper
    ): ArbiterSatnet(this_node, nodes)
{
    m_arbiter_leo_geo_helper = arbiter_leo_geo_helper;
}

std::tuple<int32_t, int32_t, int32_t> 
ArbiterGEO::TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
){
    uint64_t hash_value = (uint64_t)PeekPointer<const ns3::Packet>(pkt);
    NS_ABORT_MSG_IF(m_geo_pkt_next_hop_map.find(hash_value) == m_geo_pkt_next_hop_map.end(), 
                                                    "the packet have never been push to map");

    int32_t from = m_geo_pkt_next_hop_map[hash_value];
    return m_arbiter_leo_geo_helper->m_arbiters_leo_gs[from]->FindNextHopForGEO(target_node_id);
}

void
ArbiterGEO::PushGEONextHop(ns3::Ptr<const ns3::Packet> pkt, int32_t from)
{
    // peek the ptr and transform to uint64
    uint64_t hash_value = (uint64_t)PeekPointer<const ns3::Packet>(pkt);
    bool condition = m_geo_pkt_next_hop_map.find(hash_value) != m_geo_pkt_next_hop_map.end();
    NS_ABORT_MSG_IF(condition, "find 2 packet have same ptr address");

    m_geo_pkt_next_hop_map.emplace(hash_value, from);
}

std::string ArbiterGEO::StringReprOfForwardingState(){
    // do nothing
    return "ArbiterGEO forwarding state";
}



}