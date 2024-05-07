/**
 * Author:  silent-rookie      2024
*/

#ifndef ARBITER_LEO_GS_GEO_HELPER_H
#define ARBITER_LEO_GS_GEO_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/basic-simulation.h"
#include "ns3/topology-satellite-network.h"
#include "ns3/ipv4-arbiter-routing.h"
#include "ns3/abort.h"
#include "ns3/arbiter-leo.h"
#include "ns3/arbiter-gs.h"
#include "ns3/arbiter-geo.h"
#include "ns3/arbiter-helper.h"

namespace ns3 {

    class ArbiterLEOGSGEOHelper : public ArbiterHelper
    {
    public:
        static TypeId GetTypeId (void);
        ArbiterLEOGSGEOHelper(Ptr<BasicSimulation> basicSimulation, Ptr<TopologySatelliteNetwork> topology);

        virtual void Install() override;

        Ptr<ArbiterLEO> GetArbiterLEO(size_t index);
        Ptr<ArbiterGS> GetArbiterGS(size_t index);
        Ptr<ArbiterGEO> GetArbiterGEO(size_t index);

    protected:
        std::vector<std::vector<std::vector<std::tuple<int32_t, int32_t, int32_t>>>> InitialEmptyForwardingState();
        void UpdateState(int64_t t);
        void UpdateForwardingState(int64_t t);
        void UpdateIllsState(int64_t t);

        // Parameters
        int64_t m_dynamicStateUpdateIntervalNs;
        std::vector<Ptr<ArbiterLEO>> m_arbiters_leo;
        std::vector<Ptr<ArbiterGS>> m_arbiters_gs;
        std::vector<Ptr<ArbiterGEO>> m_arbiters_geo;
    };

} // namespace ns3



#endif