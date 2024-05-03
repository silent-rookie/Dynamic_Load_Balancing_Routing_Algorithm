/**
 * Author:  silent-rookie      2024
*/



#ifndef ARBITER_LEO_H
#define ARBITER_LEO_H

#include "ns3/arbiter-satnet.h"
#include "ns3/abort.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"

namespace ns3 {

class ArbiterLEOGSGEOHelper;

class ArbiterLEO : public ArbiterSatnet
{
public:
    static TypeId GetTypeId (void);

    ArbiterLEO(
            Ptr<Node> this_node,
            NodeContainer nodes,
            int32_t next_GEO_hop,
            std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> next_hop_lists,
            ArbiterLEOGSGEOHelper* arbiter_helper
    );

    static void Initialize(Ptr<BasicSimulation> basicSimulation, int64_t num_sat, int64_t num_gs, int64_t num_geo);

    // LEOGEO forward next-hop implementation
    std::tuple<int32_t, int32_t, int32_t> TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    );

    // Update the forward state
    void SetLEOForwardState(int32_t target_node_id, std::vector<std::tuple<int32_t, int32_t, int32_t>> next_hop_list);
    void SetLEONextGEOID(int32_t next_GEO_node_id);

    std::vector<std::tuple<int32_t, int32_t, int32_t>> GetLEOForwardState(int32_t target_node_id);
    int32_t GetLEONextGEOID();

    bool CheckIfInTraficJamArea();
    bool CheckIfNeedDetour();

    std::string StringReprOfForwardingState();

private:
    // list of trafic jam area position
    // first: the trafic jam area position
    // second: true mean that the area be able to change to non-jam area
    typedef std::list<std::pair<Ptr<MobilityModel>, bool>> TraficAreasList;

    // update detour information each interval: receive_datarate_update_interval_ns
    void UpdateDetour();
    void UpdateReceiveDatarate();
    void UpdateState();

    // Schedule that after trafic_jam_update_interval_ns, 
    // a jam area can be transformed to non-jam area
    void ScheduleTraficJamArea(std::pair<Ptr<MobilityModel>, bool>& ptr);

    // if the node which attach to this arbiter is detour
    bool is_detour;

protected:
    int32_t m_next_GEO_node_id;
    ArbiterLEOGSGEOHelper* m_arbiter_helper;
    std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> m_next_hop_lists;

    static TraficAreasList trafic_jam_areas;                // list of trafic jam area position
    static double trafic_judge_rate_in_jam;                 // Determine if a detour is necessary in jam area
    static double trafic_judge_rate_non_jam;                // Determine if a detour is necessary in not-jam area
    static double trafic_judge_rate_jam_to_normal;          // Determine if a trafic jam area is transform to not-jam area
    static int64_t trafic_jam_area_radius_m;                // radius of a trafic jam area in meter
    static int64_t trafic_jam_update_interval_ns;           // after that interval time, a jam area be able to change to non-jam area
    static int64_t receive_datarate_update_interval_ns;     // the interval that a netdevice receive datarate update
    static double isl_data_rate_megabit_per_s;
    static double gsl_data_rate_megabit_per_s;
    static double ill_data_rate_megabit_per_s;
    static int64_t num_satellites;
    static int64_t num_groundstations;
    static int64_t num_GEOsatellites;
};

}

#endif //ARBITER_LEO_GEO_H