/**
 * Author:  silent-rookie      2024
*/

#include "arbiter-leo-gs.h"
#include "ns3/arbiter-leo-geo-helper.h"



namespace ns3{


NS_OBJECT_ENSURE_REGISTERED (ArbiterLEOGS);

ArbiterLEOGS::TraficAreasList ArbiterLEOGS::trafic_jam_areas;      // list of trafic jam area position
double ArbiterLEOGS::trafic_judge_rate_in_jam = 0;                  // Determine if a detour is necessary in jam area
double ArbiterLEOGS::trafic_judge_rate_non_jam = 0;                 // Determine if a detour is necessary in not-jam area
double ArbiterLEOGS::tarfic_judge_rate_jam_to_normal = 0;           // Determine if a trafic jam area is transform to not-jam area
int64_t ArbiterLEOGS::trafic_jam_area_radius_m = 0;               // radius of a trafic jam area in meter
int64_t ArbiterLEOGS::trafic_jam_update_interval_ns = 0;
int64_t ArbiterLEOGS::receive_datarate_update_interval_ns = 0;
double ArbiterLEOGS::isl_data_rate_megabit_per_s = 0;
double ArbiterLEOGS::gsl_data_rate_megabit_per_s = 0;
int64_t ArbiterLEOGS::num_satellites = 0;
int64_t ArbiterLEOGS::num_groundstations = 0;

TypeId ArbiterLEOGS::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ArbiterLEOGS")
            .SetParent<ArbiterSatnet> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterLEOGS::ArbiterLEOGS(
        Ptr<Node> this_node,
        NodeContainer nodes,
        int32_t next_GEO_node_id,
        std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> next_hop_lists,
        ArbiterLEOGEOHelper* arbiter_leogeo_helper
) : ArbiterSatnet(this_node, nodes)
{
    is_detour = false;
    m_next_GEO_node_id = next_GEO_node_id;
    m_next_hop_lists = next_hop_lists;
    m_arbiter_leogeo_helper = arbiter_leogeo_helper;

    // Initialize must before
    NS_ABORT_MSG_IF(receive_datarate_update_interval_ns == 0, "Initialize must before");
    UpdateState();
}

void ArbiterLEOGS::Initialize(Ptr<BasicSimulation> basicSimulation, int64_t num_sat, int64_t num_gs){
    trafic_judge_rate_in_jam = parse_positive_double(basicSimulation->GetConfigParamOrFail("trafic_judge_rate_in_jam"));
    trafic_judge_rate_non_jam = parse_positive_double(basicSimulation->GetConfigParamOrFail("trafic_judge_rate_non_jam"));
    tarfic_judge_rate_jam_to_normal = parse_positive_double(basicSimulation->GetConfigParamOrFail("trafic_judge_rate_jam_to_normal"));
    trafic_jam_area_radius_m = parse_positive_int64(basicSimulation->GetConfigParamOrFail("trafic_jam_area_radius_m"));
    trafic_jam_update_interval_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("trafic_jam_update_interval_ns"));
    receive_datarate_update_interval_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("receive_datarate_update_interval_ns"));
    isl_data_rate_megabit_per_s = parse_positive_double(basicSimulation->GetConfigParamOrFail("isl_data_rate_megabit_per_s"));
    gsl_data_rate_megabit_per_s = parse_positive_double(basicSimulation->GetConfigParamOrFail("gsl_data_rate_megabit_per_s"));
    num_satellites = num_sat;
    num_groundstations = num_gs;

    // initialize receive_datarate_update_interval_ns in ReceiveDatarateDevice
    ReceiveDataRateDevice::SetReceiveDatarateUpdateIntervalNS(receive_datarate_update_interval_ns);

    std::cout << "\n  > Algorithm argument" << std::endl;
    std::cout << "    trafic_judge_rate_in_jam:             " + std::to_string(trafic_judge_rate_in_jam) << std::endl;
    std::cout << "    trafic_judge_rate_non_jam:            " + std::to_string(trafic_judge_rate_non_jam) << std::endl;
    std::cout << "    tarfic_judge_rate_jam_to_normal:      " + std::to_string(tarfic_judge_rate_jam_to_normal) << std::endl;
    std::cout << "    trafic_jam_area_radius_m:             " + std::to_string(trafic_jam_area_radius_m) << std::endl;
    std::cout << "    trafic_jam_update_interval_ns:        " + std::to_string(trafic_jam_update_interval_ns) << std::endl;
    std::cout << "    receive_datarate_update_interval_ns:  " + std::to_string(receive_datarate_update_interval_ns) << std::endl;
    std::cout << std::endl;
}

std::tuple<int32_t, int32_t, int32_t> ArbiterLEOGS::TopologySatelliteNetworkDecide(
        int32_t source_node_id,
        int32_t target_node_id,
        Ptr<const Packet> pkt,
        Ipv4Header const &ipHeader,
        bool is_request_for_source_ip_so_no_next_header
) {
    NS_ABORT_MSG_IF(target_node_id == m_node_id, "target_id == current_id, id: " + std::to_string(m_node_id));

    if((uint32_t)m_node_id < num_satellites + num_groundstations){
        // Note! we assume that LEO and groud station have 3 candidate
        for(size_t i = 0; i < m_next_hop_lists[target_node_id].size(); ++i){
            int32_t target_index = std::get<0>(m_next_hop_lists[target_node_id][i]);
            Ptr<Node> target_node = m_nodes.Get(target_index);
            if( target_index == target_node_id || 
                !m_arbiter_leogeo_helper->GetArbiterLEOGS(target_index)->CheckIfNeedDetour()){
                // find a neighbor ground station
                // or
                // find a neighbor leo satellite which can be forward
                return m_next_hop_lists[target_node_id][i];
            }
        }

        if((uint32_t)m_node_id < num_satellites){
            // arbiter for satellite
            // 3 neighbor leo satellites are in trafic jam areas,
            // we can only forward the packet to GEOsatellite

            // make the GEOsatellite know the pkt is forward from current node
            int32_t target_geo_id = m_next_GEO_node_id - num_satellites - num_groundstations;
            m_arbiter_leogeo_helper->GetArbiterGEO(target_geo_id)->PushGEONextHop(pkt, m_node_id);

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
        else{
            // arbiter for ground station
            // 3 neighbor leo satellites are in trafic jam areas,
            // we can only forward the packet to nearest leo satellite
            return m_next_hop_lists[target_node_id][0];
        }
    }
    else{
        NS_ABORT_MSG("invalid m_node_id: " + std::to_string(m_node_id));
    }
}

void ArbiterLEOGS::SetLEOGSForwardState(int32_t target_node_id, std::vector<std::tuple<int32_t, int32_t, int32_t>> next_hop_list){
    m_next_hop_lists[target_node_id] = next_hop_list;
}

void ArbiterLEOGS::SetLEONextGEOID(int32_t next_GEO_node_id){
    m_next_GEO_node_id = next_GEO_node_id;
}

std::vector<std::tuple<int32_t, int32_t, int32_t>> 
ArbiterLEOGS::GetLEOGSForwardState(int32_t target_node_id){
    return m_next_hop_lists[target_node_id];
}

int32_t ArbiterLEOGS::GetLEONextGEOID(){
    return m_next_GEO_node_id;
}

bool ArbiterLEOGS::CheckIfNeedDetour(){
    return is_detour;
}

bool ArbiterLEOGS::CheckIfInTraficJamArea(){
    // check if the node is in trafic jam area
    Ptr<Node> node = m_nodes.Get(m_node_id);
    Ptr<MobilityModel> aMobility = node->GetObject<MobilityModel>();
    TraficAreasList::iterator ptr = trafic_jam_areas.begin();
    while(ptr != trafic_jam_areas.end()){
        Ptr<MobilityModel> bMobility = ptr->first;
        double distance = aMobility->GetDistanceFrom (bMobility);
        if(distance < trafic_jam_area_radius_m){
            return true;
        }
        ++ptr;
    }

    return false;
}

std::string ArbiterLEOGS::StringReprOfForwardingState(){
    return "ArbiterLEOGS forwarding state";
}

void ArbiterLEOGS::ScheduleTraficJamArea(std::pair<Ptr<MobilityModel>, bool>& ptr){
    ptr.second = true;
}

void ArbiterLEOGS::UpdateState(){
    UpdateReceiveDatarate();
    UpdateDetour();

    // Plan next update
    Simulator::Schedule(NanoSeconds(receive_datarate_update_interval_ns), &ArbiterLEOGS::UpdateState, this);
}

void ArbiterLEOGS::UpdateDetour(){
    // interface for device in satellite:
    // 0: loop-back interface
    // 1 ~ 4: isl interface
    // 5: gsl interface
    // 6: ill interface
    // interface for device in ground station:
    // 0: loop-back interface
    // 1: gsl interface
    Ptr<Node> node = m_nodes.Get(m_node_id);
    uint32_t num_interfaces = node->GetObject<Ipv4>()->GetNInterfaces();
    NS_ABORT_MSG_IF(num_interfaces != 7 && num_interfaces != 2, "num interfaces in devices must as 7 or 2");

    // check if the node is in trafic jam area
    bool is_in_trafic_jam_area = false;
    std::vector<TraficAreasList::iterator> in_trafic_jam_ptrs;
    Ptr<MobilityModel> aMobility = node->GetObject<MobilityModel>();
    TraficAreasList::iterator ptr = trafic_jam_areas.begin();
    while(ptr != trafic_jam_areas.end()){
        Ptr<MobilityModel> bMobility = ptr->first;
        double distance = aMobility->GetDistanceFrom (bMobility);
        if(distance < trafic_jam_area_radius_m){
            is_in_trafic_jam_area = true;
            in_trafic_jam_ptrs.push_back(ptr);
        }
        ++ptr;
    }

    // calculate if it need detour 
    bool is_need_detour = false;
    bool is_jam_to_normal = false;
    bool is_normal_to_jam = false;
    // Only when all interfaces are uncongested can we determine that the area is non-jam area
    bool all_interface_is_jam_to_normal = true;

    // i begin at 1 to skip the loop-back interface
    // i end at 5 because the last interface of satellite is ill interface
    // (but is GSLNetDevice because we did not implement ILLNetDevice), so we skip it
    for(uint32_t i = 1; i < num_interfaces && i < 6; ++i){
        uint64_t now_bps, target_bps;
        now_bps = 0;
        target_bps = 0;
        if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>() != 0){
            // GSL NetDevice
            // NOTE: GSL do not calculate in trafic jam
            // Ptr<GSLNetDevice> gsl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>();
            // now_bps = gsl->GetReceiveDataRate().GetBitRate();
            // target_bps = DataRate(std::to_string(gsl_data_rate_megabit_per_s) + "Mbps").GetBitRate();
        }
        else if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>() != 0){
            // ISL NetDevice
            Ptr<PointToPointLaserNetDevice> isl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>();
            now_bps = isl->GetReceiveDataRate().GetBitRate();
            target_bps = DataRate(std::to_string(isl_data_rate_megabit_per_s) + "Mbps").GetBitRate();
        }
        else{
            NS_ABORT_MSG("Unidentified NetDevice");
        }

        if(is_in_trafic_jam_area){
            if(now_bps >= target_bps * trafic_judge_rate_in_jam){
                // the node is in jam area, and need detour
                is_need_detour = true;
            }
            else if(now_bps < target_bps * tarfic_judge_rate_jam_to_normal){
                // the node is in jam area, and the jam area is transform to normal area
                is_jam_to_normal = true;
            }
        }
        else{
            if(now_bps >= target_bps * trafic_judge_rate_non_jam){
                // the node is in not-jam area, and the area is transform to jam area
                is_normal_to_jam = true;
                is_need_detour = true;
            }
        }

        // check is something wrong
        bool check_condition = ((!is_need_detour) && (!is_jam_to_normal) && (!is_normal_to_jam)) ||     // nothing change
                                ((!is_need_detour) && (is_jam_to_normal) && (!is_normal_to_jam)) ||     // jam -> normal
                                ((is_need_detour) && (!is_jam_to_normal) && (is_normal_to_jam))  ||     // normal -> jam
                                ((is_need_detour) && (!is_jam_to_normal) && (!is_normal_to_jam));       // detour in jam area
        std::string fatal_str = "is_need_detour: " + std::to_string(is_need_detour) + ", " + 
                                "is_jam_to_normal: " + std::to_string(is_jam_to_normal) + ", " +
                                "is_normal_to_jam: " + std::to_string(is_normal_to_jam) + ", " +
                                "now_bps: " + std::to_string(now_bps);
        NS_ABORT_MSG_UNLESS(check_condition, fatal_str);

        all_interface_is_jam_to_normal &= is_jam_to_normal;
        is_jam_to_normal = false;

        if(is_need_detour) break;
    }

    // update the trafic jam areas list
    if(is_in_trafic_jam_area && all_interface_is_jam_to_normal){
        for(TraficAreasList::iterator ptr : in_trafic_jam_ptrs){
            // must after trafic_jam_update_interval_ns
            if(ptr->second){
                trafic_jam_areas.erase(ptr);

                // display the progres of trafic jam list(in case the list is too long)
                size_t areas_size = trafic_jam_areas.size();
                if((areas_size % 5) == 0){
                    std::cout << "The trafic jam list size(decrease): " << areas_size << std::endl;
                }
            }
        }
    }
    else if(is_normal_to_jam){
        trafic_jam_areas.push_back(std::make_pair(node->GetObject<MobilityModel>(), false));
        Simulator::Schedule(NanoSeconds(trafic_jam_update_interval_ns), &ArbiterLEOGS::ScheduleTraficJamArea, this, trafic_jam_areas.back());

        // display the progres of trafic jam list(in case the list is too long)
        size_t areas_size = trafic_jam_areas.size();
        if((areas_size % 5) == 0){
            std::cout << "The trafic jam list size(increase): " << areas_size << std::endl;
        }
        NS_ABORT_MSG_IF(areas_size > 30, "The trafic jam list size is bigger than 30(too big)");
    }

    // update if the node need detour which attach to this arbiter
    is_detour = is_need_detour;
}

void ArbiterLEOGS::UpdateReceiveDatarate(){
    // interface for device in satellite:
    // 0: loop-back interface
    // 1 ~ 4: isl interface
    // 5: gsl interface
    // 6: ill interface
    // interface for device in ground station:
    // 0: loop-back interface
    // 1: gsl interface
    Ptr<Node> node = m_nodes.Get(m_node_id);
    uint32_t num_interfaces = node->GetObject<Ipv4>()->GetNInterfaces();
    NS_ABORT_MSG_IF(num_interfaces != 7 && num_interfaces != 2, "num interfaces in devices must as 7 or 2");

    // i begin at 1 to skip the loop-back interface
    for(uint32_t i = 1; i < num_interfaces; ++i){
        if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>() != 0){
            // GSL NetDevice
            Ptr<GSLNetDevice> gsl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>();
            gsl->UpdateReceiveDataRate();
        }
        else if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>() != 0){
            // ISL NetDevice
            Ptr<PointToPointLaserNetDevice> isl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>();
            isl->UpdateReceiveDataRate();
        }
        else{
            NS_ABORT_MSG("Unidentified NetDevice");
        }
    }
}

}