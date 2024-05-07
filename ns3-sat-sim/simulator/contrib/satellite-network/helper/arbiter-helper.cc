/**
 * Author: silent-rookie    2024
*/

#include "arbiter-helper.h"


namespace ns3{

NS_OBJECT_ENSURE_REGISTERED (ArbiterHelper);

TypeId ArbiterHelper::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::ArbiterHelper")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterHelper::ArbiterHelper(Ptr<BasicSimulation> basicSimulation, Ptr<TopologySatelliteNetwork> topology):
                                m_basicSimulation(basicSimulation), m_topology(topology) 
{

}


}