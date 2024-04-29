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
    return FindNextHopForGEO(from, target_node_id);
}

std::tuple<int32_t, int32_t, int32_t> 
ArbiterGEO::FindNextHopForGEO(int32_t from, int32_t target_node_id){
    int32_t ptr = std::get<0>(m_arbiter_leo_geo_helper->m_arbiters_leo_gs[from]->GetLEOGSForwardState(target_node_id)[0]);
    NS_ABORT_MSG_IF(target_node_id == ptr, "LEO forward packet to GEO instead of ground station");

    int32_t last_ptr = ptr;
    // recursive search the node which not in trafic jam area
    while(ptr != target_node_id && m_arbiter_leo_geo_helper->m_arbiters_leo_gs[ptr]->CheckIfNeedDetour()){
        // make sure the next node is in ill distance
        if(m_arbiter_leo_geo_helper->m_arbiters_leo_gs[ptr]->GetLEONextGEOID() != m_node_id){
            // next hop of GEOsatellite is out of ill.
            // this situation is special, for now we just abort.
            NS_ABORT_MSG("next hop of GEOsatellite is out of ill");
        }

        last_ptr = ptr;
        ptr = std::get<0>(m_arbiter_leo_geo_helper->m_arbiters_leo_gs[ptr]->GetLEOGSForwardState(target_node_id)[0]);
    }
    
    // interface for device in satellite:
    // 0: loop-back interface
    // 1 ~ 4: isl interface
    // 5: gsl interface
    // 6: ill interface
    // interface for device in GEOsatellite:
    // 0: loop-back interface
    // 1: ill interface

    if(ptr == target_node_id){
        // all leo on the road are detour
        // we can only detour to the last leo
        return std::make_tuple(last_ptr, 1, 6);
    }
    else{
        return std::make_tuple(ptr, 1, 6);
    }
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