/**
 * Author:  silent-rookie      2024
*/

#ifndef ARBITER_LEO_GEO_HELPER_H
#define ARBITER_LEO_GEO_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/basic-simulation.h"
#include "ns3/topology-satellite-network.h"
#include "ns3/ipv4-arbiter-routing.h"
#include "ns3/abort.h"
#include "ns3/arbiter-leo-gs.h"
#include "ns3/arbiter-geo.h"

namespace ns3 {

    class ArbiterLEOGEOHelper
    {
    public:
        ArbiterLEOGEOHelper(Ptr<BasicSimulation> basicSimulation, Ptr<TopologySatelliteNetwork> topology);

        Ptr<ArbiterLEOGS> GetArbiterLEOGS(size_t index);
        Ptr<ArbiterGEO> GetArbiterGEO(size_t index);

    private:

        std::vector<std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>>> InitialEmptyForwardingState();
        void UpdateState(int64_t t);
        void UpdateForwardingState(int64_t t);
        void UpdateIllsState(int64_t t);

        // Parameters
        Ptr<BasicSimulation> m_basicSimulation;
        Ptr<TopologySatelliteNetwork> m_topology;
        int64_t m_dynamicStateUpdateIntervalNs;
        std::vector<Ptr<ArbiterLEOGS>> m_arbiters_leo_gs;
        std::vector<Ptr<ArbiterGEO>> m_arbiters_geo;
    };

} // namespace ns3



#endif