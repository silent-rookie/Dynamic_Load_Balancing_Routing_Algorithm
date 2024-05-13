/**
 * Author: silent-rookie    2024
*/

#include "arbiter-geo.h"
#include "ns3/arbiter-leo-gs-geo-helper.h"
#include "ns3/from-tag.h"



namespace ns3{



NS_OBJECT_ENSURE_REGISTERED (ArbiterGEO);

int64_t ArbiterGEO::receive_datarate_update_interval_ns = 0;
double ArbiterGEO::ill_data_rate_megabit_per_s = 0;
int64_t ArbiterGEO::num_satellites = 0;
int64_t ArbiterGEO::num_groundstations = 0;
int64_t ArbiterGEO::num_GEOsatellites = 0;

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
            Ptr<ArbiterLEOGSGEOHelper> arbiter_helper
    ): ArbiterSatnet(this_node, nodes)
{
    m_arbiter_helper = arbiter_helper;

    // Initialize must before
    NS_ABORT_MSG_IF(receive_datarate_update_interval_ns == 0, "Initialize must before");
    UpdateReceiveDatarate();
}

void ArbiterGEO::InitializeArbiter(Ptr<BasicSimulation> basicSimulation, int64_t num_sat, int64_t num_gs, int64_t num_geo){
    receive_datarate_update_interval_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("receive_datarate_update_interval_ns"));
    ill_data_rate_megabit_per_s = parse_positive_double(basicSimulation->GetConfigParamOrFail("ill_data_rate_megabit_per_s"));
    num_satellites = num_sat;
    num_groundstations = num_gs;
    num_GEOsatellites = num_geo;

    // initialize receive_datarate_update_interval_ns in ReceiveDatarateDevice
    ReceiveDataRateDevice::SetReceiveDatarateUpdateIntervalNS(receive_datarate_update_interval_ns);
}

std::tuple<int32_t, int32_t, int32_t> 
ArbiterGEO::TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
){
    NS_ABORT_MSG_UNLESS(m_node_id >= num_satellites + num_groundstations, "arbiter_geo in: " + std::to_string(m_node_id));
    NS_ABORT_MSG_IF(target_node_id == m_node_id, "target_id == current_id, id: " + std::to_string(m_node_id));

    FromTag tag;
    bool found = pkt->PeekPacketTag(tag);
    if(!found){
        Icmpv4TimeExceeded icmp;
        pkt->PeekHeader(icmp);
        Ipv4Header header = icmp.GetHeader();
        if(header.GetTtl() == 0){
            // Icmp packet(see Ipv4L3Protocol::IpForward, line 1066, and Icmpv4L4Protocol::SendTimeExceededTtl).
            // we do not know the from LEO, so we can do nothing.
            return std::make_tuple(-1, -1, -1);

            // we can do more check: if(ResolveNodeIdFromIp(header.GetSource().Get()) == (uint32_t)target_node_id).
            // but I think it is time-consuming, so I did not use it.
        }
    }
    NS_ABORT_MSG_IF(!found, "a packet be forward to GEO can not find From LEO");

    return FindNextHopForGEO(tag.GetFrom(), target_node_id);
}

std::tuple<int32_t, int32_t, int32_t> 
ArbiterGEO::FindNextHopForGEO(uint64_t from, int32_t target_node_id){
    int32_t ptr = std::get<0>(m_arbiter_helper->GetArbiterLEO(from)->GetLEOForwardState(target_node_id)[0]);
    if(target_node_id == ptr){
        // may be the from LEO satellite move and the target_node_id GS can see it.
        return std::make_tuple(from, 1, 6);
    }

    int32_t last_ptr = ptr;
    // recursive search the node which not in trafic jam area
    while(ptr != target_node_id && !m_arbiter_helper->GetArbiterLEO(ptr)->CheckIfInTraficJamArea()){
        // make sure the next node is in ill distance
        int32_t next_GEO_id = m_arbiter_helper->GetArbiterLEO(ptr)->GetLEONextGEOID();
        if(next_GEO_id != m_node_id){
            if(last_ptr == ptr){
                // the first next hop of GEOsatellite is out of ill. this situation 
                // is special, because we already made a check in ArbiterLEO::ForwardToGEO.
                // The only reason I can think of is because we update ill state in that time.
                // For now we just forward to the first next hop, 
                // BUT YOU NEED TO KNOW THAT IT IS ILLEGAL.

                // NS_ABORT_MSG("next hop of GEOsatellite is out of ill");
                return std::make_tuple(ptr, 1, 6);
            }
            else{
                // the next hop of GEOsatellite is out of ill.
                // we forward to the last LEO.
                return std::make_tuple(last_ptr, 1, 6);
            }
        }

        last_ptr = ptr;
        ptr = std::get<0>(m_arbiter_helper->GetArbiterLEO(ptr)->GetLEOForwardState(target_node_id)[0]);
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
        // all leo on the road are in jam area.
        // we can only detour to the last leo
        return std::make_tuple(last_ptr, 1, 6);
    }
    else{
        return std::make_tuple(ptr, 1, 6);
    }
}

std::string ArbiterGEO::StringReprOfForwardingState(){
    // do nothing
    return "ArbiterGEO forwarding state";
}

void ArbiterGEO::UpdateReceiveDatarate(){
    // interface for device in GEO satellite:
    // 0: loop-back interface
    // 1: ill interface
    Ptr<Node> node = m_nodes.Get(m_node_id);
    uint32_t num_interfaces = node->GetObject<Ipv4>()->GetNInterfaces();
    NS_ABORT_MSG_IF(num_interfaces != 2, "num interfaces in GEO must as 2: " + std::to_string(num_interfaces));

    // i begin at 1 to skip the loop-back interface
    for(uint32_t i = 1; i < num_interfaces; ++i){
        if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>() != 0){
            // ILL NetDevice(GSL NetDevice)
            Ptr<GSLNetDevice> gsl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>();
            gsl->UpdateReceiveDataRate();
        }
        else{
            NS_ABORT_MSG("Unidentified NetDevice in GS");
        }
    }

    // Plan next update
    Simulator::Schedule(NanoSeconds(receive_datarate_update_interval_ns), &ArbiterGEO::UpdateReceiveDatarate, this);
}


}