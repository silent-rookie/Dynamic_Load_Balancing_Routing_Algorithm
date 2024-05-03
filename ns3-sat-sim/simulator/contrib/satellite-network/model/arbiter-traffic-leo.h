/**
 * Author:  silent-rookie      2024
*/



#ifndef ARBITER_TRAFFIC_LEO_H
#define ARBITER_TRAFFIC_LEO_H

#include "ns3/abort.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"
#include "ns3/arbiter-leo.h"
#include "ns3/arbiter-leo-gs-geo-helper.h"
#include "ns3/id-seq-ts-tos-header.h"

namespace ns3{

class ArbiterTrafficLEO: public ArbiterLEO{
public:
    static TypeId GetTypeId (void);

    ArbiterTrafficLEO(
            Ptr<Node> this_node,
            NodeContainer nodes,
            int32_t next_GEO_hop,
            std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>> next_hop_lists,
            ArbiterLEOGSGEOHelper* arbiter_leogeo_helper
    );

    std::tuple<int32_t, int32_t, int32_t> TopologySatelliteNetworkDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    ) override;

};


}







#endif