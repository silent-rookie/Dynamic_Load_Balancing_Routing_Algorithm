/**
 * Author:  silent-rookie      2024
*/

#ifndef ARBITER_TRAFFIC_CLASSIFY_HELPER_H
#define ARBITER_TRAFFIC_CLASSIFY_HELPER_H

#include "ns3/arbiter-leo-gs-geo-helper.h"
#include "ns3/arbiter-traffic-leo.h"

namespace ns3 {

    class ArbiterTrafficClassifyHelper : public ArbiterLEOGSGEOHelper
    {
    public:
        static TypeId GetTypeId (void);
        ArbiterTrafficClassifyHelper(Ptr<BasicSimulation> basicSimulation, Ptr<TopologySatelliteNetwork> topology);

        void Install() override;

    };

} // namespace ns3



#endif