/**
 * Author:  silent-rookie      2024
*/



#ifndef ARBITER_LEO_GS_H
#define ARBITER_LEO_GS_H

#include "ns3/arbiter-satnet.h"
#include "ns3/abort.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"

namespace ns3 {

class ArbiterLEOGEOHelper;

class ArbiterLEOGS : public ArbiterSatnet
{
public:
    static TypeId GetTypeId (void);

    ArbiterLEOGS(
            Ptr<Node> this_node,
            NodeContainer nodes,
            int32_t next_GEO_hop,
            std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> next_hop_lists,
            ArbiterLEOGEOHelper* arbiter_leogeo_helper
    );

    static void Initialize(Ptr<BasicSimulation> basicSimulation, int64_t num_sat, int64_t num_gs);

    // LEOGEO forward next-hop implementation
    std::tuple<int32_t, int32_t, int32_t> TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    );

    // Update the forward state
    void SetLEOGSForwardState(int32_t target_node_id, std::vector<std::tuple<int32_t, int32_t, int32_t>> next_hop_list);
    void SetLEONextGEOID(int32_t next_GEO_node_id);

    // find the next leo to for GEOsatellite
    // called by GEOsatellite
    std::tuple<int32_t, int32_t, int32_t> FindNextHopForGEO(int32_t target_node_id);

    bool CheckIfNeedDetourForNode(Ptr<Node> node);
    bool CheckIfInTraficJamArea(Ptr<Node> node);

    std::string StringReprOfForwardingState();

private:
    int32_t m_next_GEO_node_id;
    ArbiterLEOGEOHelper* m_arbiter_leogeo_helper;
    std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> m_next_hop_lists;

    static std::list<Ptr<MobilityModel>> trafic_jam_areas;      // list of trafic jam area position
    static double trafic_judge_rate_in_jam;                      // Determine if a detour is necessary in jam area
    static double trafic_judge_rate_non_jam;                     // Determine if a detour is necessary in not-jam area
    static double tarfic_judge_rate_jam_to_normal;               // Determine if a trafic jam area is transform to not-jam area
    static int64_t trafic_jam_area_radius_m;                   // radius of a trafic jam area in meter
    static double isl_data_rate_megabit_per_s;
    static double gsl_data_rate_megabit_per_s;
    static int64_t num_satellites;
    static int64_t num_groundstations;
};

}

#endif //ARBITER_LEO_GEO_H