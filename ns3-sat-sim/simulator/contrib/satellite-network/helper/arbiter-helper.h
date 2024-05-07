/**
 * Author: silent-rookie    2024
*/

#ifndef ARBITER_HELPER_H
#define ARBITER_HELPER_H

#include "ns3/basic-simulation.h"
#include "ns3/topology-satellite-network.h"



namespace ns3{

class ArbiterHelper : public Object
{
public:
    static TypeId GetTypeId (void);
    ArbiterHelper(Ptr<BasicSimulation> basicSimulation, Ptr<TopologySatelliteNetwork> topology);

    // Install arbiter for all nodes in topology
    virtual void Install() = 0;

protected:
    Ptr<BasicSimulation> m_basicSimulation;
    Ptr<TopologySatelliteNetwork> m_topology;

};


}







#endif