/**
 * Author:  silent-rookie      2024
*/

#include "arbiter-leo-gs.h"
#include "ns3/arbiter-leo-geo-helper.h"



namespace ns3{


NS_OBJECT_ENSURE_REGISTERED (ArbiterLEOGS);

std::list<Ptr<MobilityModel>> ArbiterLEOGS::trafic_jam_areas;      // list of trafic jam area position
double ArbiterLEOGS::trafic_judge_rate_in_jam = 0;                  // Determine if a detour is necessary in jam area
double ArbiterLEOGS::trafic_judge_rate_non_jam = 0;                 // Determine if a detour is necessary in not-jam area
double ArbiterLEOGS::tarfic_judge_rate_jam_to_normal = 0;           // Determine if a trafic jam area is transform to not-jam area
int64_t ArbiterLEOGS::trafic_jam_area_radius_m = 0;               // radius of a trafic jam area in meter
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
    m_next_GEO_node_id = next_GEO_node_id;
    m_next_hop_lists = next_hop_lists;
    m_arbiter_leogeo_helper = arbiter_leogeo_helper;
}

void ArbiterLEOGS::Initialize(Ptr<BasicSimulation> basicSimulation, int64_t num_sat, int64_t num_gs){
    trafic_judge_rate_in_jam = parse_positive_double(basicSimulation->GetConfigParamOrFail("trafic_judge_rate_in_jam"));
    trafic_judge_rate_non_jam = parse_positive_double(basicSimulation->GetConfigParamOrFail("trafic_judge_rate_non_jam"));
    tarfic_judge_rate_jam_to_normal = parse_positive_double(basicSimulation->GetConfigParamOrFail("trafic_judge_rate_jam_to_normal"));
    trafic_jam_area_radius_m = parse_positive_int64(basicSimulation->GetConfigParamOrFail("trafic_jam_area_radius_m"));
    isl_data_rate_megabit_per_s = parse_positive_double(basicSimulation->GetConfigParamOrFail("isl_data_rate_megabit_per_s"));
    gsl_data_rate_megabit_per_s = parse_positive_double(basicSimulation->GetConfigParamOrFail("gsl_data_rate_megabit_per_s"));
    num_satellites = num_sat;
    num_groundstations = num_gs;
}

std::tuple<int32_t, int32_t, int32_t> ArbiterLEOGS::TopologySatelliteNetworkDecide(
        int32_t source_node_id,
        int32_t target_node_id,
        Ptr<const Packet> pkt,
        Ipv4Header const &ipHeader,
        bool is_request_for_source_ip_so_no_next_header
) {
    NS_ABORT_MSG_IF(target_node_id == m_node_id, "target id == current id?");

    // upadte current node
    // ToDo: it is suitable that update itself before forward?
    CheckIfNeedDetourForNode(m_nodes.Get(m_node_id));

    if((uint32_t)m_node_id < num_satellites + num_groundstations){
        // Note! we assume that LEO and groud station have 3 candidate
        for(size_t i = 0; i < m_next_hop_lists[target_node_id].size(); ++i){
            int32_t target_index = std::get<0>(m_next_hop_lists[target_node_id][i]);
            Ptr<Node> target_node = m_nodes.Get(target_index);
            if(target_index == target_node_id || !CheckIfNeedDetourForNode(target_node)){
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
            m_arbiter_leogeo_helper->m_arbiters_geo[target_geo_id]->PushGEONextHop(pkt, m_node_id);

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

bool ArbiterLEOGS::CheckIfNeedDetourForNode(Ptr<Node> node){
    // interface for device in satellite:
    // 0: loop-back interface
    // 1 ~ 4: isl interface
    // 5: gsl interface
    // 6: ill interface
    // interface for device in ground station:
    // 0: loop-back interface
    // 1: gsl interface
    uint32_t num_interfaces = node->GetObject<Ipv4>()->GetNInterfaces();
    NS_ABORT_MSG_IF(num_interfaces != 7 || num_interfaces != 2, "num interfaces in devices must as 7 or 2");

    // check if the node is in trafic jam area
    bool is_in_trafic_jam_area = false;
    std::vector<std::list<Ptr<MobilityModel>>::iterator> in_trafic_jam_ptrs;
    Ptr<MobilityModel> aMobility = node->GetObject<MobilityModel>();
    std::list<Ptr<MobilityModel>>::iterator ptr = trafic_jam_areas.begin();
    while(ptr != trafic_jam_areas.end()){
        Ptr<MobilityModel> bMobility = *ptr;
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
    // i begin at 1 to skip the loop-back interface
    for(uint32_t i = 1; i < num_interfaces; ++i){
        uint64_t now_bps, target_bps;
        if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>() != 0){
            // GSL NetDevice
            Ptr<GSLNetDevice> gsl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>();
            now_bps = gsl->GetReceiveDataRate().GetBitRate();
            target_bps = DataRate(std::to_string(gsl_data_rate_megabit_per_s) + "Mbps").GetBitRate();
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

        if(is_need_detour || is_jam_to_normal || is_normal_to_jam) break;
    }

    // update the trafic jam areas list
    if(is_jam_to_normal){
        for(std::list<Ptr<MobilityModel>>::iterator ptr : in_trafic_jam_ptrs){
            trafic_jam_areas.erase(ptr);
        }
    }
    else if(is_normal_to_jam){
        trafic_jam_areas.push_back(node->GetObject<MobilityModel>());

        // display the progres of trafic jam list(in case the list is too long)
        size_t areas_size = trafic_jam_areas.size();
        if((areas_size % 5) == 0){
            std::cout << "The trafic jam list size: " + areas_size << std::endl;
        }
        NS_ABORT_MSG_IF(areas_size > 30, "The trafic jam list size is bigger than 30(to big)");
    }


    return is_need_detour;
}

bool ArbiterLEOGS::CheckIfInTraficJamArea(Ptr<Node> node){
    // check if the node is in trafic jam area
    Ptr<MobilityModel> aMobility = node->GetObject<MobilityModel>();
    std::list<Ptr<MobilityModel>>::iterator ptr = trafic_jam_areas.begin();
    while(ptr != trafic_jam_areas.end()){
        Ptr<MobilityModel> bMobility = *ptr;
        double distance = aMobility->GetDistanceFrom (bMobility);
        if(distance < trafic_jam_area_radius_m){
            return true;
        }
        ++ptr;
    }

    return false;
}

std::tuple<int32_t, int32_t, int32_t>
ArbiterLEOGS::FindNextHopForGEO(int32_t target_node_id){
    int32_t ptr = std::get<0>(m_next_hop_lists[target_node_id][0]);
    NS_ABORT_MSG_IF(target_node_id == ptr, "LEO forward packet to GEO instead of ground station");

    int32_t last_ptr = ptr;
    // recursive search the node which not in trafic jam area
    while(ptr != target_node_id && CheckIfNeedDetourForNode(m_nodes.Get(ptr))){
        // make sure the next node is in ill distance
        if(m_arbiter_leogeo_helper->m_arbiters_leo_gs[ptr]->m_next_GEO_node_id != m_next_GEO_node_id) break;

        last_ptr = ptr;
        ptr = std::get<0>(m_arbiter_leogeo_helper->m_arbiters_leo_gs[ptr]->m_next_hop_lists[target_node_id][0]);
    }

    // next hop of GEOsatellite is out of ill.
    // the situation is special, for now we just abort.
    NS_ABORT_MSG_IF(last_ptr == ptr, "next hop of GEOsatellite is out of ill");
    
    // interface for device in satellite:
    // 0: loop-back interface
    // 1 ~ 4: isl interface
    // 5: gsl interface
    // 6: ill interface
    // interface for device in GEOsatellite:
    // 0: loop-back interface
    // 1: ill interface
    return std::make_tuple(last_ptr, 1, 6);
}

std::string ArbiterLEOGS::StringReprOfForwardingState(){
    return "ArbiterLEOGS forwarding state";
}

}