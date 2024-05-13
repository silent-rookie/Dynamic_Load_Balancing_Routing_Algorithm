/**
 * Author:  silent-rookie      2024
*/

#include "arbiter-gs.h"
#include "ns3/arbiter-leo-gs-geo-helper.h"



namespace ns3{


NS_OBJECT_ENSURE_REGISTERED (ArbiterGS);

int64_t ArbiterGS::receive_datarate_update_interval_ns = 0;
double ArbiterGS::gsl_data_rate_megabit_per_s = 0;
int64_t ArbiterGS::num_satellites = 0;
int64_t ArbiterGS::num_groundstations = 0;
int64_t ArbiterGS::num_GEOsatellites = 0;

TypeId ArbiterGS::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ArbiterGS")
            .SetParent<ArbiterSatnet> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterGS::ArbiterGS(
        Ptr<Node> this_node,
        NodeContainer nodes,
        std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> next_hop_lists,
        Ptr<ArbiterLEOGSGEOHelper> arbiter_helper
) : ArbiterSatnet(this_node, nodes)
{
    m_next_hop_lists = next_hop_lists;
    m_arbiter_helper = arbiter_helper;

    // Initialize must before
    NS_ABORT_MSG_IF(receive_datarate_update_interval_ns == 0, "Initialize must before");
    UpdateReceiveDatarate();
}

void ArbiterGS::InitializeArbiter(Ptr<BasicSimulation> basicSimulation, int64_t num_sat, int64_t num_gs, int64_t num_geo){
    receive_datarate_update_interval_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("receive_datarate_update_interval_ns"));
    gsl_data_rate_megabit_per_s = parse_positive_double(basicSimulation->GetConfigParamOrFail("gsl_data_rate_megabit_per_s"));
    num_satellites = num_sat;
    num_groundstations = num_gs;
    num_GEOsatellites = num_geo;

    // initialize receive_datarate_update_interval_ns in ReceiveDatarateDevice
    ReceiveDataRateDevice::SetReceiveDatarateUpdateIntervalNS(receive_datarate_update_interval_ns);
}

std::tuple<int32_t, int32_t, int32_t> ArbiterGS::TopologySatelliteNetworkDecide(
        int32_t source_node_id,
        int32_t target_node_id,
        Ptr<const Packet> pkt,
        Ipv4Header const &ipHeader,
        bool is_request_for_source_ip_so_no_next_header
) {
    NS_ABORT_MSG_UNLESS(m_node_id >= num_satellites && m_node_id < num_satellites + num_groundstations, 
                                                        "arbiter_gs in: " + std::to_string(m_node_id));
    NS_ABORT_MSG_IF(target_node_id == m_node_id, "target_id == current_id, id: " + std::to_string(m_node_id));

    // Note! we assume that groud station have 3 candidate
    for(size_t i = 0; i < m_next_hop_lists[target_node_id].size(); ++i){
        int32_t next_node_index = std::get<0>(m_next_hop_lists[target_node_id][i]);
        int32_t next_interface_index = std::get<2>(m_next_hop_lists[target_node_id][i]);
        if(next_node_index == -1) break;   // the  num of LEO this GS can see is less than 3
        if(!m_arbiter_helper->GetArbiterLEO(next_node_index)->CheckIfNeedDetour(next_interface_index)){
            // find a neighbor leo satellite which can be forward
            return m_next_hop_lists[target_node_id][i];
        }
    }

    // 3 neighbor leo satellites are in detour,
    // we can only forward the packet to the nearest LEO satellite
    return m_next_hop_lists[target_node_id][0];
}

void ArbiterGS::SetGSForwardState(int32_t target_node_id, std::vector<std::tuple<int32_t, int32_t, int32_t>> next_hop_list){
    m_next_hop_lists[target_node_id] = next_hop_list;
}

std::vector<std::tuple<int32_t, int32_t, int32_t>> 
ArbiterGS::GetGSForwardState(int32_t target_node_id){
    return m_next_hop_lists[target_node_id];
}

std::string ArbiterGS::StringReprOfForwardingState(){
    return "ArbiterGS forwarding state";
}

void ArbiterGS::UpdateReceiveDatarate(){
    // interface for device in ground station:
    // 0: loop-back interface
    // 1: gsl interface
    Ptr<Node> node = m_nodes.Get(m_node_id);
    uint32_t num_interfaces = node->GetObject<Ipv4>()->GetNInterfaces();
    NS_ABORT_MSG_IF(num_interfaces != 2, "num interfaces in gs must as 2: " + std::to_string(num_interfaces));

    // i begin at 1 to skip the loop-back interface
    for(uint32_t i = 1; i < num_interfaces; ++i){
        if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>() != 0){
            // GSL NetDevice
            Ptr<GSLNetDevice> gsl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>();
            gsl->UpdateReceiveDataRate();
        }
        else{
            NS_ABORT_MSG("Unidentified NetDevice in GS");
        }
    }

    // Plan next update
    Simulator::Schedule(NanoSeconds(receive_datarate_update_interval_ns), &ArbiterGS::UpdateReceiveDatarate, this);
}

}