/**
 * Author: silent-rookie    2024
*/

#include "arbiter-traffic-classify-helper.h"



namespace ns3{

ArbiterTrafficClassifyHelper::ArbiterTrafficClassifyHelper(Ptr<BasicSimulation> basicSimulation, 
                                                        Ptr<TopologySatelliteNetwork> topology)
: ArbiterLEOGSGEOHelper(basicSimulation, topology)
{

}


void ArbiterTrafficClassifyHelper::InstallArbiter(Ptr<BasicSimulation> basicSimulation, 
                                                Ptr<TopologySatelliteNetwork> topology)
{
    NodeContainer m_nodes = topology->GetNodes();

    // Read in initial forwarding state
    std::cout << "  > Create initial single forwarding state" << std::endl;
    std::vector<std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>>> initial_forwarding_state = InitialEmptyForwardingState();
    basicSimulation->RegisterTimestamp("Create initial LEOGEO forwarding state");

    // Initialize
    ArbiterTrafficLEO::Initialize(basicSimulation, topology->GetNumSatellites(), topology->GetNumGroundStations(), topology->GetNumGEOSatellites());
    ArbiterGS::Initialize(basicSimulation, topology->GetNumSatellites(), topology->GetNumGroundStations(), topology->GetNumGEOSatellites());
    ArbiterGEO::Initialize(basicSimulation, topology->GetNumSatellites(), topology->GetNumGroundStations(), topology->GetNumGEOSatellites());
    // Set the routing arbiters
    std::cout << "  > Setting the routing arbiter on LEO node" << std::endl;
    for (size_t i = 0; i < topology->GetNumSatellites(); i++) {
        Ptr<ArbiterTrafficLEO> arbiter = CreateObject<ArbiterTrafficLEO>(m_nodes.Get(i), m_nodes, -2, initial_forwarding_state[i], this);
        m_arbiters_leo.push_back(arbiter);
        m_nodes.Get(i)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);
    }
    std::cout << "  > Setting the routing arbiter on GS node" << std::endl;
    for (size_t i = 0; i < topology->GetNumGroundStations(); i++) {
        size_t gs_id = i + topology->GetNumSatellites();
        Ptr<ArbiterGS> arbiter = CreateObject<ArbiterGS>(m_nodes.Get(gs_id), m_nodes, initial_forwarding_state[gs_id], this);
        m_arbiters_gs.push_back(arbiter);
        m_nodes.Get(gs_id)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);
    }
    std::cout << "  > Setting the routing arbiter on GEO node" << std::endl;
    for (size_t i = 0; i < topology->GetNumGEOSatellites(); ++i){
        size_t geo_id = i + topology->GetNumSatellites() + topology->GetNumGroundStations();
        Ptr<ArbiterGEO> arbiter = CreateObject<ArbiterGEO>(m_nodes.Get(geo_id), m_nodes, this);
        m_arbiters_geo.push_back(arbiter);
        m_nodes.Get(geo_id)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiter);
    }
    basicSimulation->RegisterTimestamp("Setup routing arbiter on each node");
}



}