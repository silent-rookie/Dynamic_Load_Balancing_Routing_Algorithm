/**
 * Author: silent-rookie    2024
*/

#ifndef ID_SEQ_TS_TOS_HEADER_H
#define ID_SEQ_TS_TOS_HEADER_H

#include "ns3/id-seq-ts-header.h"
#include "ns3/nstime.h"

namespace ns3{
    
class IdSeqTsTosHeader : public IdSeqTsHeader{
public:
    static TypeId GetTypeId (void);
    
    IdSeqTsTosHeader ();
    
    void SetTos(uint8_t tos);
    uint8_t GetTos() const;

    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

private:
    uint8_t m_tos;
};

/**
 * ClassA: has the highest priority and the delay-sensitive interactive applications such as VoIP are involved;
 *         never detour
 * ClassB: consisting of relatively delay-robust applications such as real-time video streaming applications
 *         only detour in LEO layer
 * ClassC: represents best effort traffic, has robustness to long delays and delay changes
 *         detour to GEO satellites
*/
enum class TrafficClass{
    class_default = 0x0,

    class_A = 0x10,     // 0001 0000
    class_B = 0x08,     // 0000 1000
    class_C = 0x04      // 0000 0100
};

TrafficClass Tos2TrafficClass(uint8_t tos);
std::string TrafficClass2String(TrafficClass pclass);

extern std::vector<TrafficClass> TrafficClassVec;



}




#endif