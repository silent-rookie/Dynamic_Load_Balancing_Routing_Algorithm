/**
 * Author: silent-rookie    2024
*/

#include "traffic-classify-tos.h"
#include "ns3/abort.h"

namespace ns3{


std::vector<TrafficClass> TrafficClassVec = {TrafficClass::class_A, TrafficClass::class_B, TrafficClass::class_C, TrafficClass::class_default};


TrafficClass Tos2TrafficClass(uint8_t tos){
    switch (tos)
    {
        case 0x10:
            return TrafficClass::class_A;
        case 0x08:
            return TrafficClass::class_B;
        case 0x04:
            return TrafficClass::class_C;
        case 0x0:
            return TrafficClass::class_default;
        default:
            NS_ABORT_MSG("invalid tos in IdSeqTsTosHeader: " + std::to_string(tos));
    }
}

std::string TrafficClass2String(TrafficClass pclass){
    if(pclass == TrafficClass::class_A){
        return "A";
    }
    else if(pclass == TrafficClass::class_B){
        return "B";
    }
    else if(pclass == TrafficClass::class_C){
        return "C";
    }
    else if(pclass == TrafficClass::class_default){
        return "Default";
    }
    else{
        NS_ABORT_MSG("invalid TrafficClass: " + std::to_string(static_cast<uint8_t>(pclass)));
    }
}



}