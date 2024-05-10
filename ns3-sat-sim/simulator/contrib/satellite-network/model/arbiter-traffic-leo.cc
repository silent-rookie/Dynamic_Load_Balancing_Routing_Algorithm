/**
 * Author:  silent-rookie      2024
*/

#include "arbiter-traffic-leo.h"
#include "ns3/from-tag.h"



namespace ns3{


NS_OBJECT_ENSURE_REGISTERED (ArbiterTrafficLEO);

TypeId ArbiterTrafficLEO::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::ArbiterTrafficLEO")
            .SetParent<ArbiterLEO> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterTrafficLEO::ArbiterTrafficLEO(
            Ptr<Node> this_node,
            NodeContainer nodes,
            int32_t next_GEO_hop,
            std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> next_hop_lists,
            Ptr<ArbiterLEOGSGEOHelper> arbiter_leogeo_helper
): ArbiterLEO(this_node, nodes, next_GEO_hop, next_hop_lists, arbiter_leogeo_helper)
{

}

std::tuple<int32_t, int32_t, int32_t> ArbiterTrafficLEO::TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip)
{
    NS_ABORT_MSG_UNLESS(m_node_id < num_satellites, "arbiter_leo in: " + std::to_string(m_node_id));
    NS_ABORT_MSG_IF(target_node_id == m_node_id, "target_id == current_id, id: " + std::to_string(m_node_id));

    FlowTosTag tag;
    bool found = pkt->PeekPacketTag(tag);
    if(!found){
        Icmpv4TimeExceeded icmp;
        pkt->PeekHeader(icmp);
        Ipv4Header header = icmp.GetHeader();
        if(header.GetTtl() == 0){
            // Icmp packet(see Ipv4L3Protocol::IpForward, line 1066, and Icmpv4L4Protocol::SendTimeExceededTtl)
            // just forward to shortest LEO
            return m_next_hop_lists[target_node_id][0];

            // we can do more check: if(ResolveNodeIdFromIp(header.GetSource().Get()) == (uint32_t)target_node_id).
            // but I think it is time-consuming, so I did not use it.
        }
    }
    NS_ABORT_MSG_IF(!found, "a packet has no FlowTosTag");
    TrafficClass pclass = Tos2TrafficClass(tag.GetTos());

    // the packet not implement traffic class
    // we just hand over to ArbiterLEO
    if(pclass == TrafficClass::class_default){
        return ArbiterLEO::TopologySatelliteNetworkDecide(source_node_id, target_node_id, pkt, ipHeader, is_socket_request_for_source_ip);
    }

    // the next node do not need detour
    int32_t next_first = std::get<0>(m_next_hop_lists[target_node_id][0]);
    if(next_first == target_node_id || !m_arbiter_helper->GetArbiterLEO(next_first)->CheckIfNeedDetour()){
        return m_next_hop_lists[target_node_id][0]; 
    }

    // the next node need detour
    if(pclass == TrafficClass::class_A){
        // never detour
        return m_next_hop_lists[target_node_id][0];
    }
    else if(pclass == TrafficClass::class_B){
        // detour to LEO satellite which has the shortest distance
        return m_next_hop_lists[target_node_id][1];
    }
    else if(pclass == TrafficClass::class_C){
        // detour to GEO satellite
        AddFromTagForGEO(pkt);
        return std::make_tuple(m_next_GEO_node_id, 6, 1);
    }
    else{
        NS_ABORT_MSG("TrafficClass error");
    }
}




}