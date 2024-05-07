/**
 * Author: silent-rookie    2024
*/

#include "arbiter-geo.h"
#include "ns3/arbiter-leo-gs-geo-helper.h"



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
    NS_ABORT_MSG_UNLESS(m_node_id >= num_satellites + num_groundstations, "arbiter_gs in: " + std::to_string(m_node_id));
    NS_ABORT_MSG_IF(target_node_id == m_node_id, "target_id == current_id, id: " + std::to_string(m_node_id));

    uint64_t hash_value = (uint64_t)PeekPointer<const ns3::Packet>(pkt);
    NS_ABORT_MSG_IF(m_geo_pkt_next_hop_map.find(hash_value) == m_geo_pkt_next_hop_map.end(), "the packet have never been push to map");

    int32_t from = m_geo_pkt_next_hop_map[hash_value];
    return FindNextHopForGEO(from, target_node_id);
}

std::tuple<int32_t, int32_t, int32_t> 
ArbiterGEO::FindNextHopForGEO(int32_t from, int32_t target_node_id){
    int32_t ptr = std::get<0>(m_arbiter_helper->GetArbiterLEO(from)->GetLEOForwardState(target_node_id)[0]);
    NS_ABORT_MSG_IF(target_node_id == ptr, "LEO forward packet to GEO instead of ground station");

    int32_t last_ptr = ptr;
    // recursive search the node which not in trafic jam area
    while(ptr != target_node_id && m_arbiter_helper->GetArbiterLEO(ptr)->CheckIfNeedDetour()){
        // make sure the next node is in ill distance
        if(m_arbiter_helper->GetArbiterLEO(ptr)->GetLEONextGEOID() != m_node_id){
            // next hop of GEOsatellite is out of ill.
            // this situation is special, for now we just abort.
            NS_ABORT_MSG("next hop of GEOsatellite is out of ill");
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