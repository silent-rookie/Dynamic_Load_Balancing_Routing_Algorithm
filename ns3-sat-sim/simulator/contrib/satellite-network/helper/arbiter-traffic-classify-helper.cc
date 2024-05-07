/**
 * Author: silent-rookie    2024
*/

#include "arbiter-traffic-classify-helper.h"



namespace ns3{

NS_OBJECT_ENSURE_REGISTERED (ArbiterTrafficClassifyHelper);

TypeId ArbiterTrafficClassifyHelper::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::ArbiterTrafficClassifyHelper")
            .SetParent<ArbiterLEOGSGEOHelper> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterTrafficClassifyHelper::ArbiterTrafficClassifyHelper(Ptr<BasicSimulation> basicSimulation, 
                                                        Ptr<TopologySatelliteNetwork> topology)
: ArbiterLEOGSGEOHelper(basicSimulation, topology)
{

}

void ArbiterTrafficClassifyHelper::Install(){
    std::cout << "INITIALIZE TRAFFIC CLASSIFY LEOGSGEO ARBITER" << std::endl;

    NodeContainer m_nodes = m_topology->GetNodes();

    // Read in initial forwarding state
    std::vector<std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>>> initial_forwarding_state = InitialEmptyForwardingState();

    // Initialize
    ArbiterTrafficLEO::InitializeArbiter(m_basicSimulation, m_topology->GetNumSatellites(), m_topology->GetNumGroundStations(), m_topology->GetNumGEOSatellites());
    ArbiterGS::InitializeArbiter(m_basicSimulation, m_topology->GetNumSatellites(), m_topology->GetNumGroundStations(), m_topology->GetNumGEOSatellites());
    ArbiterGEO::InitializeArbiter(m_basicSimulation, m_topology->GetNumSatellites(), m_topology->GetNumGroundStations(), m_topology->GetNumGEOSatellites());
    // Set the routing arbiters
    std::cout << "  > Setting the routing arbiter on LEO node" << std::endl;
    for (size_t i = 0; i < m_topology->GetNumSatellites(); i++) {
        Ptr<ArbiterTrafficLEO> arbiter = CreateObject<ArbiterTrafficLEO>(m_nodes.Get(i), m_nodes, -2, initial_forwarding_state[i], this);
        m_arbiters_leo.push_back(arbiter);
        m_nodes.Get(i)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);
    }
    std::cout << "  > Setting the routing arbiter on GS node" << std::endl;
    for (size_t i = 0; i < m_topology->GetNumGroundStations(); i++) {
        size_t gs_id = i + m_topology->GetNumSatellites();
        Ptr<ArbiterGS> arbiter = CreateObject<ArbiterGS>(m_nodes.Get(gs_id), m_nodes, initial_forwarding_state[gs_id], this);
        m_arbiters_gs.push_back(arbiter);
        m_nodes.Get(gs_id)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);
    }
    std::cout << "  > Setting the routing arbiter on GEO node" << std::endl;
    for (size_t i = 0; i < m_topology->GetNumGEOSatellites(); ++i){
        size_t geo_id = i + m_topology->GetNumSatellites() + m_topology->GetNumGroundStations();
        Ptr<ArbiterGEO> arbiter = CreateObject<ArbiterGEO>(m_nodes.Get(geo_id), m_nodes, this);
        m_arbiters_geo.push_back(arbiter);
        m_nodes.Get(geo_id)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);
    }
    m_basicSimulation->RegisterTimestamp("Setup routing arbiter on each node");

    // Load first forwarding state
    m_dynamicStateUpdateIntervalNs = parse_positive_int64(m_basicSimulation->GetConfigParamOrFail("dynamic_state_update_interval_ns"));
    std::cout << "  > Dynamic update interval: " << m_dynamicStateUpdateIntervalNs << "ns" << std::endl;
    std::cout << "  > Perform first update state load for t=0" << std::endl;
    UpdateState(0);
    m_basicSimulation->RegisterTimestamp("Initialize Traffic Classify LEOGSGEO dynamic state");

    std::cout << std::endl;
}

}