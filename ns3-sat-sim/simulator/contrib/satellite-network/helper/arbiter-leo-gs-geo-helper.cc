/**
 * Author:  silent-rookie      2024
*/

#include "arbiter-leo-gs-geo-helper.h"

namespace ns3{

NS_OBJECT_ENSURE_REGISTERED (ArbiterLEOGSGEOHelper);

TypeId ArbiterLEOGSGEOHelper::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::ArbiterLEOGSGEOHelper")
            .SetParent<ArbiterHelper> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterLEOGSGEOHelper::ArbiterLEOGSGEOHelper(Ptr<BasicSimulation> basicSimulation, 
                                        Ptr<TopologySatelliteNetwork> topology)
: ArbiterHelper(basicSimulation, topology)
{

}

void ArbiterLEOGSGEOHelper::Install(){
    std::cout << "INITIALIZE LEOGSGEO ARBITER" << std::endl;

    NodeContainer m_nodes = m_topology->GetNodes();

    // Read in initial forwarding state
    std::vector<std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>>> initial_forwarding_state = InitialEmptyForwardingState();

    // Initialize
    ArbiterLEO::InitializeArbiter(m_basicSimulation, m_topology->GetNumSatellites(), m_topology->GetNumGroundStations(), m_topology->GetNumGEOSatellites());
    ArbiterGS::InitializeArbiter(m_basicSimulation, m_topology->GetNumSatellites(), m_topology->GetNumGroundStations(), m_topology->GetNumGEOSatellites());
    ArbiterGEO::InitializeArbiter(m_basicSimulation, m_topology->GetNumSatellites(), m_topology->GetNumGroundStations(), m_topology->GetNumGEOSatellites());
    // Set the routing arbiters
    std::cout << "  > Setting the routing arbiter on LEO node" << std::endl;
    for (size_t i = 0; i < m_topology->GetNumSatellites(); i++) {
        Ptr<ArbiterLEO> arbiter = CreateObject<ArbiterLEO>(m_nodes.Get(i), m_nodes, -2, initial_forwarding_state[i], this);
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
    m_basicSimulation->RegisterTimestamp("Initialize LEOGSGEO dynamic state");

    std::cout << std::endl;
}

Ptr<ArbiterLEO> ArbiterLEOGSGEOHelper::GetArbiterLEO(size_t index){
    if(index < 0 || index >= m_arbiters_leo.size()) return 0;
    return m_arbiters_leo[index];
}

Ptr<ArbiterGS> ArbiterLEOGSGEOHelper::GetArbiterGS(size_t index){
    if(index < 0 || index >= m_arbiters_gs.size()) return 0;
    return m_arbiters_gs[index];
}

Ptr<ArbiterGEO> ArbiterLEOGSGEOHelper::GetArbiterGEO(size_t index){
    if(index < 0 || index >= m_arbiters_geo.size()) return 0;
    return m_arbiters_geo[index];
}

Ptr<BasicSimulation> ArbiterLEOGSGEOHelper::GetBasicSimulation(){
    return m_basicSimulation;
}

std::vector<std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>>> 
ArbiterLEOGSGEOHelper::InitialEmptyForwardingState(){
    size_t num_leo_gs = m_topology->GetNumSatellites() + m_topology->GetNumGroundStations();
    std::vector<std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>>> initial_forwarding_state;
    for (size_t i = 0; i < num_leo_gs; i++) {
        std::vector<std::vector <std::tuple<int32_t, int32_t, int32_t>>> next_hop_list;
        for (size_t j = 0; j < num_leo_gs; j++) {
            std::vector <std::tuple<int32_t, int32_t, int32_t>> current_list;
            for(size_t k = 0; k < 3; ++k){          // we assume that LEO and groud station have 3 candidate
                current_list.push_back(std::make_tuple(-2, -2, -2));
            }
            next_hop_list.push_back(current_list); // -2 indicates an invalid entry
        }
        initial_forwarding_state.push_back(next_hop_list);
    }
    return initial_forwarding_state;
}

void ArbiterLEOGSGEOHelper::UpdateState(int64_t t){
    UpdateForwardingState(t);
    UpdateIllsState(t);

    // Given that this code will only be used with satellite networks, this is okay-ish,
    // but it does create a very tight coupling between the two -- technically this class
    // can be used for other purposes as well
    if (!parse_boolean(m_basicSimulation->GetConfigParamOrDefault("satellite_network_force_static", "false"))) {

        // Plan the next update
        int64_t next_update_ns = t + m_dynamicStateUpdateIntervalNs;
        if (next_update_ns < m_basicSimulation->GetSimulationEndTimeNs()) {
            Simulator::Schedule(NanoSeconds(m_dynamicStateUpdateIntervalNs), &ArbiterLEOGSGEOHelper::UpdateState, this, next_update_ns);
        }

    }
}

void ArbiterLEOGSGEOHelper::UpdateForwardingState(int64_t t) {
    /**
     * update LEO GS forwarding state
    */
    NodeContainer m_nodes = m_topology->GetNodes();

    // Filename
    std::ostringstream res;
    res << m_basicSimulation->GetRunDir() << "/";
    res << m_basicSimulation->GetConfigParamOrFail("satellite_network_routes_dir") << "/fstate/fstate_" << t << ".txt";
    std::string filename = res.str();

    // Check that the file exists
    if (!file_exists(filename)) {
        throw std::runtime_error(format_string("File %s does not exist.", filename.c_str()));
    }

    // Open file
    std::string line;
    std::ifstream fstate_file(filename);
    if (fstate_file) {
        // Go over each line
        while (getline(fstate_file, line)) {

            // Split on ,
            std::vector<std::string> comma_split = split_string(line, ",", 11);

            // Retrieve identifiers
            int64_t current_node_id = parse_positive_int64(comma_split[0]);
            int64_t target_node_id = parse_positive_int64(comma_split[1]);

            std::vector<std::tuple<int32_t, int32_t, int32_t>> next_hop_list;
            for(size_t i = 2; i < 11; i += 3){
                int64_t next_hop_node_id = parse_int64(comma_split[i]);
                int64_t my_if_id = parse_int64(comma_split[i + 1]);
                int64_t next_if_id = parse_int64(comma_split[i + 2]);

                int64_t num_leo_gs = m_topology->GetNumSatellites() + m_topology->GetNumGroundStations();
                // Check the node identifiers
                NS_ABORT_MSG_IF(current_node_id < 0 || current_node_id >= num_leo_gs, "Invalid current node id.");
                NS_ABORT_MSG_IF(target_node_id < 0 || target_node_id >= num_leo_gs, "Invalid target node id.");
                NS_ABORT_MSG_IF(next_hop_node_id < -1 || next_hop_node_id >= num_leo_gs, "Invalid next hop node id.");

                // Drops are only valid if all three values are -1
                NS_ABORT_MSG_IF(
                        !(next_hop_node_id == -1 && my_if_id == -1 && next_if_id == -1)
                        &&
                        !(next_hop_node_id != -1 && my_if_id != -1 && next_if_id != -1),
                        "All three must be -1 for it to signify a drop."
                );

                // Check the interfaces exist
                /**
                 *  // interface for device in satellite:
                    // 0: loop-back interface
                    // 1 ~ 4: isl interface
                    // 5: gsl interface
                    // 6: ill interface
                    // interface for device in ground station:
                    // 0: loop-back interface
                    // 1: gsl interface
                */
                if(current_node_id < m_topology->GetNumSatellites()){
                    NS_ABORT_MSG_UNLESS(my_if_id == -1 || 
                                        (my_if_id >= 0 && my_if_id + 2 < m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNInterfaces()), 
                                        "Invalid current interface");
                }
                else{
                    NS_ABORT_MSG_UNLESS(my_if_id == -1 || 
                                        (my_if_id >= 0 && my_if_id + 1 < m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNInterfaces()), 
                                        "Invalid current interface");
                }

                if(next_hop_node_id < m_topology->GetNumSatellites()){
                    NS_ABORT_MSG_UNLESS(next_if_id == -1 || 
                                        (next_if_id >= 0 && next_if_id + 2 < m_nodes.Get(next_hop_node_id)->GetObject<Ipv4>()->GetNInterfaces()), 
                                        "Invalid next hop interface");
                }
                else{
                    NS_ABORT_MSG_UNLESS(next_if_id == -1 || 
                                        (next_if_id >= 0 && next_if_id + 1 < m_nodes.Get(next_hop_node_id)->GetObject<Ipv4>()->GetNInterfaces()), 
                                        "Invalid next hop interface");
                }

                // Node id and interface id checks are only necessary for non-drops
                if (next_hop_node_id != -1 && my_if_id != -1 && next_if_id != -1) {

                    // It must be either GSL or ISL
                    bool source_is_gsl = m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + my_if_id)->GetObject<GSLNetDevice>() != 0;
                    bool source_is_isl = m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + my_if_id)->GetObject<PointToPointLaserNetDevice>() != 0;
                    NS_ABORT_MSG_IF((!source_is_gsl) && (!source_is_isl), "Only GSL and ISL network devices are supported");

                    // If current is a GSL interface, the destination must also be a GSL interface
                    NS_ABORT_MSG_IF(
                        source_is_gsl &&
                        m_nodes.Get(next_hop_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + next_if_id)->GetObject<GSLNetDevice>() == 0,
                        "Destination interface must be attached to a GSL network device"
                    );

                    // If current is a p2p laser interface, the destination must match exactly its counter-part
                    NS_ABORT_MSG_IF(
                        source_is_isl &&
                        m_nodes.Get(next_hop_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + next_if_id)->GetObject<PointToPointLaserNetDevice>() == 0,
                        "Destination interface must be an ISL network device"
                    );

                    if (source_is_isl) {
                        Ptr<NetDevice> device0 = m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + my_if_id)->GetObject<PointToPointLaserNetDevice>()->GetChannel()->GetDevice(0);
                        Ptr<NetDevice> device1 = m_nodes.Get(current_node_id)->GetObject<Ipv4>()->GetNetDevice(1 + my_if_id)->GetObject<PointToPointLaserNetDevice>()->GetChannel()->GetDevice(1);
                        Ptr<NetDevice> other_device = device0->GetNode()->GetId() == current_node_id ? device1 : device0;
                        NS_ABORT_MSG_IF(other_device->GetNode()->GetId() != next_hop_node_id, "Next hop node id across does not match");
                        NS_ABORT_MSG_IF(other_device->GetIfIndex() != 1 + next_if_id, "Next hop interface id across does not match");
                    }

                }
            
                // Add to temp next_hop_list
                // Add 1 for skip the loop-back interface
                next_hop_list.push_back(std::make_tuple(next_hop_node_id, my_if_id + 1, next_if_id + 1));
            }

            // Add to forwarding state
            if(current_node_id < m_topology->GetNumSatellites()){
                m_arbiters_leo.at(current_node_id)->SetLEOForwardState(target_node_id, next_hop_list);
            }
            else{
                int64_t index = current_node_id - m_topology->GetNumSatellites();
                m_arbiters_gs.at(index)->SetGSForwardState(target_node_id, next_hop_list);
            }
        }

        // Close file
        fstate_file.close();

    } else {
        throw std::runtime_error(format_string("File %s could not be read.", filename.c_str()));
    }
}

void ArbiterLEOGSGEOHelper::UpdateIllsState(int64_t t){
    /**
     * update GEO forwarding state
    */
    NodeContainer m_nodes = m_topology->GetNodes();

    // Filename
    std::ostringstream res;
    res << m_basicSimulation->GetRunDir() << "/";
    res << m_basicSimulation->GetConfigParamOrFail("satellite_network_routes_dir") << "/ills/ills_" << t << ".txt";
    std::string filename = res.str();

    // Check that the file exists
    if (!file_exists(filename)) {
        throw std::runtime_error(format_string("File %s does not exist.", filename.c_str()));
    }

    // Open file
    std::string line;
    std::ifstream fstate_file(filename);
    if(fstate_file){
        // Go over each line
        while (getline(fstate_file, line)) {
            // Split on ,
            std::vector<std::string> comma_split = split_string(line, " ", 2);

            // Retrieve identifiers
            int64_t sat = parse_positive_int64(comma_split[0]);
            int64_t geo = parse_positive_int64(comma_split[1]);
            
            // Check the node identifiers
            NS_ABORT_MSG_IF(sat < 0 || sat >= m_topology->GetNumSatellites(), "invalid satellite node id of ill");
            NS_ABORT_MSG_IF(geo < 0 || geo >= m_topology->GetNumGEOSatellites(), "invalid GEOsatellite node id of ill");
            
            // Add to ills state
            m_arbiters_leo.at(sat)->SetLEONextGEOID(geo + m_topology->GetNumSatellites() + m_topology->GetNumGroundStations());
        }

        // Close file
        fstate_file.close();
    }
    else{
        throw std::runtime_error(format_string("File %s could not be read.", filename.c_str()));
    }
}

}