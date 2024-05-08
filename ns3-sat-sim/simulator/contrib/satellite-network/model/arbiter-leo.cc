/**
 * Author:  silent-rookie      2024
*/

#include "arbiter-leo.h"
#include "ns3/arbiter-leo-gs-geo-helper.h"



namespace ns3{


NS_OBJECT_ENSURE_REGISTERED (ArbiterLEO);

ArbiterLEO::TraficAreasList ArbiterLEO::trafic_jam_areas;      // list of trafic jam area position
ArbiterLEO::TraficAreasTime ArbiterLEO::trafic_areas_time;      // unordered map to record the time LEO satellite enter trafic jam area
double ArbiterLEO::trafic_judge_rate_in_jam = 0;                  // Determine if a detour is necessary in jam area
double ArbiterLEO::trafic_judge_rate_non_jam = 0;                 // Determine if a detour is necessary in not-jam area
double ArbiterLEO::trafic_judge_rate_jam_to_normal = 0;           // Determine if a trafic jam area is transform to not-jam area
int64_t ArbiterLEO::trafic_jam_area_radius_m = 0;               // radius of a trafic jam area in meter
int64_t ArbiterLEO::trafic_jam_update_interval_ns = 0;
int64_t ArbiterLEO::receive_datarate_update_interval_ns = 0;
double ArbiterLEO::isl_data_rate_megabit_per_s = 0;
double ArbiterLEO::gsl_data_rate_megabit_per_s = 0;
double ArbiterLEO::ill_data_rate_megabit_per_s = 0;
int64_t ArbiterLEO::num_satellites = 0;
int64_t ArbiterLEO::num_groundstations = 0;
int64_t ArbiterLEO::num_GEOsatellites = 0;

TypeId ArbiterLEO::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ArbiterLEO")
            .SetParent<ArbiterSatnet> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterLEO::ArbiterLEO(
        Ptr<Node> this_node,
        NodeContainer nodes,
        int32_t next_GEO_node_id,
        std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> next_hop_lists,
        Ptr<ArbiterLEOGSGEOHelper> arbiter_helper
) : ArbiterSatnet(this_node, nodes)
{
    is_detour = false;
    m_next_GEO_node_id = next_GEO_node_id;
    m_next_hop_lists = next_hop_lists;
    m_arbiter_helper = arbiter_helper;

    // Initialize must before
    NS_ABORT_MSG_IF(receive_datarate_update_interval_ns == 0, "Initialize must before");
    UpdateState();
}

void ArbiterLEO::InitializeArbiter(Ptr<BasicSimulation> basicSimulation, int64_t num_sat, int64_t num_gs, int64_t num_geo){
    trafic_judge_rate_in_jam = parse_positive_double(basicSimulation->GetConfigParamOrFail("trafic_judge_rate_in_jam"));
    trafic_judge_rate_non_jam = parse_positive_double(basicSimulation->GetConfigParamOrFail("trafic_judge_rate_non_jam"));
    trafic_judge_rate_jam_to_normal = parse_positive_double(basicSimulation->GetConfigParamOrFail("trafic_judge_rate_jam_to_normal"));
    trafic_jam_area_radius_m = parse_positive_int64(basicSimulation->GetConfigParamOrFail("trafic_jam_area_radius_m"));
    trafic_jam_update_interval_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("trafic_jam_update_interval_ns"));
    receive_datarate_update_interval_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("receive_datarate_update_interval_ns"));
    isl_data_rate_megabit_per_s = parse_positive_double(basicSimulation->GetConfigParamOrFail("isl_data_rate_megabit_per_s"));
    gsl_data_rate_megabit_per_s = parse_positive_double(basicSimulation->GetConfigParamOrFail("gsl_data_rate_megabit_per_s"));
    ill_data_rate_megabit_per_s = parse_positive_double(basicSimulation->GetConfigParamOrFail("ill_data_rate_megabit_per_s"));
    num_satellites = num_sat;
    num_groundstations = num_gs;
    num_GEOsatellites = num_geo;

    // initialize receive_datarate_update_interval_ns in ReceiveDatarateDevice
    ReceiveDataRateDevice::SetReceiveDatarateUpdateIntervalNS(receive_datarate_update_interval_ns);

    std::cout << "\n  > Algorithm argument" << std::endl;
    std::cout << "    trafic_judge_rate_in_jam:             " + std::to_string(trafic_judge_rate_in_jam) << std::endl;
    std::cout << "    trafic_judge_rate_non_jam:            " + std::to_string(trafic_judge_rate_non_jam) << std::endl;
    std::cout << "    tarfic_judge_rate_jam_to_normal:      " + std::to_string(trafic_judge_rate_jam_to_normal) << std::endl;
    std::cout << "    trafic_jam_area_radius_m:             " + std::to_string(trafic_jam_area_radius_m) << std::endl;
    std::cout << "    trafic_jam_update_interval_ns:        " + std::to_string(trafic_jam_update_interval_ns) << std::endl;
    std::cout << "    receive_datarate_update_interval_ns:  " + std::to_string(receive_datarate_update_interval_ns) << std::endl;
    std::cout << std::endl;
}

std::tuple<int32_t, int32_t, int32_t> ArbiterLEO::TopologySatelliteNetworkDecide(
        int32_t source_node_id,
        int32_t target_node_id,
        Ptr<const Packet> pkt,
        Ipv4Header const &ipHeader,
        bool is_request_for_source_ip_so_no_next_header
) {
    NS_ABORT_MSG_UNLESS(m_node_id < num_satellites, "arbiter_leo in: " + std::to_string(m_node_id));
    NS_ABORT_MSG_IF(target_node_id == m_node_id, "target_id == current_id, id: " + std::to_string(m_node_id));

    // Note! we assume that LEO have 3 candidate
    for(size_t i = 0; i < m_next_hop_lists[target_node_id].size(); ++i){
        int32_t target_index = std::get<0>(m_next_hop_lists[target_node_id][i]);
        if( target_index == target_node_id || 
            !m_arbiter_helper->GetArbiterLEO(target_index)->CheckIfNeedDetour()){
            // find a neighbor ground station
            // or
            // find a neighbor leo satellite which can be forward
            return m_next_hop_lists[target_node_id][i];
        }
    }

    // 3 neighbor leo satellites are in detour,
    // we can only forward the packet to GEOsatellite

    // make the GEOsatellite know the pkt is forward from current node
    int32_t target_geo_index = m_next_GEO_node_id - num_satellites - num_groundstations;
    m_arbiter_helper->GetArbiterGEO(target_geo_index)->PushGEONextHop(pkt, m_node_id);

    // interface for device in satellite:
    // 0: loop-back interface
    // 1 ~ 4: isl interface
    // 5: gsl interface
    // 6: ill interface
    // interface for device in GEOsatellite:
    // 0: loop-back interface
    // 1: ill interface
    return std::make_tuple(m_next_GEO_node_id, 6, 1);
}

void ArbiterLEO::SetLEOForwardState(int32_t target_node_id, std::vector<std::tuple<int32_t, int32_t, int32_t>> next_hop_list){
    m_next_hop_lists[target_node_id] = next_hop_list;
}

void ArbiterLEO::SetLEONextGEOID(int32_t next_GEO_node_id){
    m_next_GEO_node_id = next_GEO_node_id;
}

std::vector<std::tuple<int32_t, int32_t, int32_t>> 
ArbiterLEO::GetLEOForwardState(int32_t target_node_id){
    return m_next_hop_lists[target_node_id];
}

int32_t ArbiterLEO::GetLEONextGEOID(){
    return m_next_GEO_node_id;
}

bool ArbiterLEO::CheckIfNeedDetour(){
    return is_detour;
}

bool ArbiterLEO::CheckIfInTraficJamArea(){
    // check if the node is in trafic jam area
    TraficAreasList::iterator ptr = trafic_jam_areas.begin();
    while(ptr != trafic_jam_areas.end()){
        if(CheckIfInTheTraficJamArea(*ptr))
            return true;
        ++ptr;
    }

    return false;
}

bool ArbiterLEO::CheckIfInTheTraficJamArea(std::shared_ptr<Vector> target){
    Ptr<Node> node = m_nodes.Get(m_node_id);
    Vector current_position = node->GetObject<MobilityModel>()->GetPosition();
    double distance = CalculateDistance(current_position, *target);
    if(distance < trafic_jam_area_radius_m)
        return true;
    else
        return false;
}

std::string ArbiterLEO::StringReprOfForwardingState(){
    return "ArbiterLEO forwarding state";
}

void ArbiterLEO::UpdateState(){
    UpdateReceiveDatarate();
    UpdateDetour();

    // Plan next update
    Simulator::Schedule(NanoSeconds(receive_datarate_update_interval_ns), &ArbiterLEO::UpdateState, this);
}

void ArbiterLEO::UpdateDetour(){
    // interface for device in satellite:
    // 0: loop-back interface
    // 1 ~ 4: isl interface
    // 5: gsl interface
    // 6: ill interface
    Ptr<Node> node = m_nodes.Get(m_node_id);
    uint32_t num_interfaces = node->GetObject<Ipv4>()->GetNInterfaces();
    NS_ABORT_MSG_IF(num_interfaces != 7, "num interfaces in LEO must as 7: " + std::to_string(num_interfaces));

    // check if the node is in trafic jam area
    bool is_in_trafic_jam_area = CheckIfInTraficJamArea();

    // calculate total_now_bps and total_target_bps for detour state update
    // i begin at 1 to skip the loop-back interface
    uint64_t total_now_bps = 0;
    uint64_t total_target_bps = 0;
    for(uint32_t i = 1; i < num_interfaces; ++i){
        if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>() != 0){
            // GSL(ILL) NetDevice
            // NOTE: GSL(ILL) do not calculate in trafic jam
        }
        else if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>() != 0){
            // ISL NetDevice
            Ptr<PointToPointLaserNetDevice> isl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>();
            total_now_bps += isl->GetReceiveDataRate().GetBitRate();
            total_target_bps += DataRate(std::to_string(isl_data_rate_megabit_per_s) + "Mbps").GetBitRate();
        }
        else{
            NS_ABORT_MSG("Unidentified NetDevice");
        }
    }
    // if(total_now_bps >= 500)
    //     std::cout << ">>>  LEO " << m_node_id << ": " << total_now_bps << " " << total_target_bps << std::endl;

    bool is_need_detour;
    // update detour state
    if(total_now_bps < total_target_bps * trafic_judge_rate_jam_to_normal){
        // do not need detour anymore
        is_need_detour = false;
    }
    else if(!is_in_trafic_jam_area && total_now_bps >= total_target_bps * trafic_judge_rate_non_jam){
        // non-jam area -> jam area
        is_need_detour = true;

        Vector current_position = node->GetObject<MobilityModel>()->GetPosition();
        std::shared_ptr<Vector> ptr = std::make_shared<Vector>(current_position);
        trafic_jam_areas.push_back(ptr);
        trafic_areas_time[ptr] = std::unordered_map<int32_t, Time>();

        // display the progres of trafic jam list(in case the list is too long)
        size_t areas_size = trafic_jam_areas.size();
        std::cout << "The trafic jam list size(increase): " << areas_size << std::endl;
        NS_ABORT_MSG_IF(areas_size > 30, "The trafic jam list size is bigger than 30(too big)");
    }
    else if(is_in_trafic_jam_area && total_now_bps >= total_target_bps * trafic_judge_rate_in_jam){
        // detour in jam area
        is_need_detour = true;
    }
    else{
        // detour state do not change
        is_need_detour = is_detour;
    }


    /**
     * Note!!! 
     * Because the logic of delete a jam area is very complex, I think it is necessary to restate the logic here: 
     *  when a LEO satellite fly trafic_jam_update_interval_ns time over a jam area and has never experienced traffic detour, 
     *  we consider that the area now is not jam, so we delete the jam area from trafic_jam_areas.
    */
    TraficAreasList::iterator ptr = trafic_jam_areas.begin();
    while(ptr != trafic_jam_areas.end()){
        bool is_has_been_record = trafic_areas_time.at(*ptr).find(m_node_id) != trafic_areas_time.at(*ptr).end();
        bool is_delete_area = false;

        if(CheckIfInTheTraficJamArea(*ptr)){
            if(is_has_been_record){
                if(is_need_detour){
                    // The current LEO satellite is in the jam area, and the start time has been recorded before, and it need detour.
                    // we need to update the time.
                    trafic_areas_time.at(*ptr).at(m_node_id) = Simulator::Now();
                }
                else{
                    // The current LEO satellite is in the jam area, we determine whether the time of the current satellite 
                    // from start time to now is greater than trafic_jam_update_interval_ns, if yes, delete the jam area.
                    if(Simulator::Now() - trafic_areas_time.at(*ptr).at(m_node_id) >= NanoSeconds(trafic_jam_update_interval_ns)){
                        // jam area -> normal erea. So complicated! :-)
                        ptr = trafic_jam_areas.erase(ptr);
                        is_delete_area = true;

                        // display the progres of trafic jam list(in case the list is too long)
                        size_t areas_size = trafic_jam_areas.size();
                        std::cout << "The trafic jam list size(decrease): " << areas_size << std::endl;
                    }
                }
            }
            else{
                // The current LEO satellite just entered the area.
                // record the start time.
                trafic_areas_time.at(*ptr)[m_node_id] = Simulator::Now();
            }
        }
        else{
            if(is_has_been_record){
                // The current LEO satellite once entered the jam area, and now it has left the area.
                // just remove from trafic_areas_time.
                trafic_areas_time.at(*ptr).erase(m_node_id);
            }
            else{
                // The current LEO satellite has nothing to do with the area.
                // do nothing.
            }
        }

        if(!is_delete_area)
            ++ptr;
    }
    

    // update if the node need detour which attach to this arbiter
    is_detour = is_need_detour;
}

void ArbiterLEO::UpdateReceiveDatarate(){
    // interface for device in satellite:
    // 0: loop-back interface
    // 1 ~ 4: isl interface
    // 5: gsl interface
    // 6: ill interface
    Ptr<Node> node = m_nodes.Get(m_node_id);
    uint32_t num_interfaces = node->GetObject<Ipv4>()->GetNInterfaces();
    NS_ABORT_MSG_IF(num_interfaces != 7, "num interfaces in LEO must as 7: " + std::to_string(num_interfaces));

    // i begin at 1 to skip the loop-back interface
    for(uint32_t i = 1; i < num_interfaces; ++i){
        if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>() != 0){
            // GSL NetDevice(ILL NetDevice)
            Ptr<GSLNetDevice> gsl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>();
            gsl->UpdateReceiveDataRate();
        }
        else if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>() != 0){
            // ISL NetDevice
            Ptr<PointToPointLaserNetDevice> isl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>();
            isl->UpdateReceiveDataRate();
        }
        else{
            NS_ABORT_MSG("Unidentified NetDevice in LEO");
        }
    }
}

}