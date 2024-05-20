/**
 * Author:  silent-rookie      2024
*/

#include "arbiter-leo.h"
#include "ns3/arbiter-leo-gs-geo-helper.h"
#include "ns3/from-tag.h"



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
    is_in_jam_area = false;
    m_next_GEO_node_id = next_GEO_node_id;
    m_next_hop_lists = next_hop_lists;
    m_arbiter_helper = arbiter_helper;

    // interface for device in LEO satellite:
    // 0: loop-back interface
    // 1 ~ 4: isl interface
    // 5: gsl interface
    // 6: ill interface
    Ptr<Node> node = m_nodes.Get(m_node_id);
    uint32_t num_interfaces = node->GetObject<Ipv4>()->GetNInterfaces();
    NS_ABORT_MSG_IF(num_interfaces != 7, "num interfaces in LEO must as 7: " + std::to_string(num_interfaces));

    // initialize each interface detour information.
    interfaces_need_detour.push_back(false);    // loop-back interface
    for(uint32_t i = 1; i < num_interfaces; ++i){
        if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>() != 0){
            // ILL NetDevice(in our implement, ILL NetDevice is GSL NetDevice) do not attend detour calculation
        }
        else if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>() != 0){
            // ISL NetDevice
            interfaces_need_detour.push_back(false);
        }
        else{
            NS_ABORT_MSG("Unidentified NetDevice in LEO");
        }
    }
    interfaces_need_detour.push_back(false);    // GSL interface

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

std::tuple<int32_t, int32_t, int32_t> 
ArbiterLEO::TopologySatelliteNetworkDecide(
        int32_t source_node_id,
        int32_t target_node_id,
        Ptr<const Packet> pkt,
        Ipv4Header const &ipHeader,
        bool is_request_for_source_ip_so_no_next_header
) 
{
    NS_ABORT_MSG_UNLESS(m_node_id < num_satellites, "arbiter_leo in: " + std::to_string(m_node_id));
    NS_ABORT_MSG_IF(target_node_id == m_node_id, "target_id == current_id, id: " + std::to_string(m_node_id));

    // Note! we assume that LEO have 3 candidate
    for(size_t i = 0; i < m_next_hop_lists[target_node_id].size(); ++i){
        int32_t next_node_id = std::get<0>(m_next_hop_lists[target_node_id][i]);
        int32_t next_interface_index = std::get<2>(m_next_hop_lists[target_node_id][i]);
        if( next_node_id == target_node_id || 
            !m_arbiter_helper->GetArbiterLEO(next_node_id)->CheckIfNeedDetour(next_interface_index)){
            // find a neighbor ground station
            // or
            // find a neighbor leo satellite which can be forward
            return m_next_hop_lists[target_node_id][i];
        }
    }

    // 3 neighbor leo satellites are in detour,
    // we can only forward the packet to GEOsatellite
    return ForwardToGEO(target_node_id, pkt);
}

void ArbiterLEO::SetLEOForwardState(int32_t target_node_id, std::vector<std::tuple<int32_t, int32_t, int32_t>> next_hop_list){
    m_next_hop_lists[target_node_id] = next_hop_list;
}

void ArbiterLEO::SetLEONextGEOID(int32_t next_GEO_node_id){
    m_next_GEO_node_id = next_GEO_node_id;
}

std::tuple<int32_t, int32_t, int32_t> 
ArbiterLEO::ForwardToGEO(int32_t target_node_id, ns3::Ptr<const ns3::Packet> pkt)
{
    // forward to GEO
    AddFromTag(pkt);

    // make sure the GEO of next hop LEO is same to current LEO.
    int32_t next_node_id = std::get<0>(m_next_hop_lists[target_node_id][0]);
    if(m_arbiter_helper->GetArbiterLEO(next_node_id)->GetLEONextGEOID() != m_next_GEO_node_id){
        // In this project, each of our GEO satellites covers 33% of the LEO satellites, 
        // but actually GEO satellites can cover about 40% of the surface area, 
        // so we assume that the LEO satellites at the coverage boundary of the 
        // two GEO satellites can be covered by the two GEO satellites.

        // So we forward to the GEO which connect to next_node
        // BUT YOU NEED TO KNOW THAT:
        // It is possible that current LEO satellite is not covered by the two GEO satellites, and we do not deal with that
        return std::make_tuple(m_arbiter_helper->GetArbiterLEO(next_node_id)->GetLEONextGEOID(), 6, 1);
    }

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

void ArbiterLEO::AddFromTag(Ptr<const ns3::Packet> pkt){
    FromTag tag;
    tag.SetFrom(m_node_id);
    pkt->AddPacketTag(tag);
}

std::vector<std::tuple<int32_t, int32_t, int32_t>> 
ArbiterLEO::GetLEOForwardState(int32_t target_node_id){
    return m_next_hop_lists[target_node_id];
}

int32_t ArbiterLEO::GetLEONextGEOID(){
    return m_next_GEO_node_id;
}

bool ArbiterLEO::CheckIfInTraficJamArea(){
    return is_in_jam_area;
}

bool ArbiterLEO::CheckIfNeedDetour(int32_t interface){
    // only detour in ISL NetDevice and GSL NetDevice
    NS_ABORT_MSG_UNLESS(interface >= 1 && interface <= 5, 
                "interface: " + std::to_string(interface) + " in LEO: " + std::to_string(m_node_id));
    return interfaces_need_detour[interface];
}

bool ArbiterLEO::CalculateIfInTraficJamArea(){
    // check if the node is in trafic jam area
    TraficAreasList::iterator ptr = trafic_jam_areas.begin();
    while(ptr != trafic_jam_areas.end()){
        if(CalculateIfInTheTraficJamArea(*ptr))
            return true;
        ++ptr;
    }

    return false;
}

bool ArbiterLEO::CalculateIfInTheTraficJamArea(std::shared_ptr<Vector> target){
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
    is_in_jam_area = CalculateIfInTraficJamArea();

    // update detour update
    int num_interface_detour = 0;
    // i begin at 1 to skip the loop-back interface
    for(uint32_t i = 1; i < interfaces_need_detour.size(); ++i){
        if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>() != 0){
            // GSL(ILL) NetDevice
            // NOTE: GSL(ILL) do not attend detour calculation, but we
            // also update detour state because ground station need GSL detour state.
            Ptr<GSLNetDevice> gsl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<GSLNetDevice>();
            uint64_t now_bps = gsl->GetReceiveDataRate().GetBitRate();
            uint64_t max_bps = DataRate(std::to_string(gsl_data_rate_megabit_per_s) + "Mbps").GetBitRate();
            interfaces_need_detour[i] = (now_bps >= max_bps * trafic_judge_rate_non_jam);
        }
        else if(node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>() != 0){
            // ISL NetDevice
            Ptr<PointToPointLaserNetDevice> isl = node->GetObject<Ipv4>()->GetNetDevice(i)->GetObject<PointToPointLaserNetDevice>();
            uint64_t now_bps = isl->GetReceiveDataRate().GetBitRate();
            uint64_t max_bps = DataRate(std::to_string(isl_data_rate_megabit_per_s) + "Mbps").GetBitRate();

            // update detour state
            if(now_bps < max_bps * trafic_judge_rate_jam_to_normal){
                // do not need detour anymore
                interfaces_need_detour[i] = false;
            }
            else if(!is_in_jam_area && now_bps >= max_bps * trafic_judge_rate_non_jam){
                // detour in non-jam area
                interfaces_need_detour[i] = true;
                ++num_interface_detour;
            }
            else if(is_in_jam_area && now_bps >= max_bps * trafic_judge_rate_in_jam){
                // detour in jam area
                interfaces_need_detour[i] = true;
                ++num_interface_detour;
            }
            else{
                // detour state do not change
            }
        }
        else{
            NS_ABORT_MSG("Unidentified NetDevice");
        }
    }

    // We set a non-jam area change to jam area only
    // at least 2 ISL interface are need detour.
    // This is for stability reason. 
    if(!is_in_jam_area && num_interface_detour >= 2){
        is_in_jam_area = true;

        Vector current_position = node->GetObject<MobilityModel>()->GetPosition();
        std::shared_ptr<Vector> ptr = std::make_shared<Vector>(current_position);
        trafic_jam_areas.push_back(ptr);
        trafic_areas_time[ptr] = std::unordered_map<int32_t, Time>();

        // display the progres of trafic jam list
        size_t areas_size = trafic_jam_areas.size();
        std::cout << "The trafic jam list size(increase): " << areas_size << std::endl;
        // NS_ABORT_MSG_IF(areas_size > 30, "The trafic jam list size is bigger than 30(too big)");
    }

    // if simulate time less than trafic_jam_update_interval_ns, a jam area can not change to non-jam area,
    // so we just return to speed up the program
    if(m_arbiter_helper->GetBasicSimulation()->GetSimulationEndTimeNs() <= trafic_jam_update_interval_ns)
        return;

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

        if(CalculateIfInTheTraficJamArea(*ptr)){
            if(is_has_been_record){
                if(num_interface_detour > 0){
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
                        is_in_jam_area = false;
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

}

void ArbiterLEO::UpdateReceiveDatarate(){
    // interface for device in LEO satellite:
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

/***

#include <iostream>
#include <cmath>

// 定义点的结构体
struct Point {
    double x, y, z;
    Point(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
};

// 定义轨道参数结构体
struct OrbitParams {
    double semi_major_axis; // 半长轴
    double inclination;     // 倾角
    // 其他轨道参数...
};

// 计算两点之间的距离
double distance(const Point& p1, const Point& p2) {
    return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) +
                     (p1.y - p2.y) * (p1.y - p2.y) +
                     (p1.z - p2.z) * (p1.z - p2.z));
}

// 计算轨道平面与球心的最短距离
double min_distance_to_sphere(const OrbitParams& orbit_params, const Point& sphere_center) {
    // 计算轨道平面的法向量
    double inclination_rad = orbit_params.inclination * M_PI / 180.0;
    double normal_x = 0.0;
    double normal_y = std::cos(inclination_rad);
    double normal_z = std::sin(inclination_rad);

    // 计算距离球心最近的轨道平面上的一点
    // 该点到球心的向量与轨道平面的法向量垂直
    double t = (sphere_center.x * normal_x + sphere_center.y * normal_y + sphere_center.z * normal_z) /
               (normal_x * normal_x + normal_y * normal_y + normal_z * normal_z);
    double x = sphere_center.x - t * normal_x;
    double y = sphere_center.y - t * normal_y;
    double z = sphere_center.z - t * normal_z;

    // 计算轨道平面上的一点与球心的距离
    Point point_on_orbit_plane(x, y, z);
    return distance(point_on_orbit_plane, sphere_center);
}

// 判断轨道是否与给定球相交
bool is_orbit_intersect_sphere(const OrbitParams& orbit_params, const Point& sphere_center, double sphere_radius) {
    // 计算轨道平面与球心的最短距离
    double min_distance = min_distance_to_sphere(orbit_params, sphere_center);

    // 判断最短距离是否小于等于球的半径
    if (min_distance <= sphere_radius) {
        return true;
    } else {
        return false;
    }
}

int main() {
    // 示例数据
    OrbitParams orbit_params = {1000.0, 30.0}; // 假设半长轴为1000，倾角为30度
    Point sphere_center(500.0, 500.0, 500.0); // 球心坐标
    double sphere_radius = 200.0; // 球的半径

    // 判断轨道是否与给定球相交
    bool result = is_orbit_intersect_sphere(orbit_params, sphere_center, sphere_radius);
    if (result) {
        std::cout << "轨道与给定球相交，卫星可能进入球内。" << std::endl;
    } else {
        std::cout << "轨道与给定球不相交，卫星不会进入球内。" << std::endl;
    }

    return 0;
}

*/

}