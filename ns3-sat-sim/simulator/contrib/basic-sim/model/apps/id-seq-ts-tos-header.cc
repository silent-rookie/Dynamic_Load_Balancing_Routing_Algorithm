/**
 * Author: silent-rookie    2024
*/

#include "id-seq-ts-tos-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("IdSeqTsTosHeader");

NS_OBJECT_ENSURE_REGISTERED (IdSeqTsTosHeader);

std::vector<TrafficClass> TrafficClassVec = {TrafficClass::class_A, TrafficClass::class_B, TrafficClass::class_C, TrafficClass::class_default};

TypeId IdSeqTsTosHeader::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::IdSeqTsTosHeader")
            .SetParent<IdSeqTsHeader> ()
            .SetGroupName("BasicSim")
            .AddConstructor<IdSeqTsTosHeader> ()
    ;
    return tid;
}
    
IdSeqTsTosHeader::IdSeqTsTosHeader (): 
                IdSeqTsHeader(), m_tos(0)
{
    NS_LOG_FUNCTION (this);
}

void IdSeqTsTosHeader::SetTos(uint8_t tos){
    NS_LOG_FUNCTION (this);
    m_tos = tos;
}
    
uint8_t IdSeqTsTosHeader::GetTos() const {
    return m_tos;
}

TypeId IdSeqTsTosHeader::GetInstanceTypeId (void) const{
    return GetTypeId ();
}

void IdSeqTsTosHeader::Print (std::ostream &os) const{
    NS_LOG_FUNCTION (this << &os);
    os << "(tos=" << m_tos <<") AND ";
    IdSeqTsHeader::Print(os);
}

uint32_t IdSeqTsTosHeader::GetSerializedSize (void) const{
    NS_LOG_FUNCTION (this);
    return IdSeqTsHeader::GetSerializedSize() + 1;
}

void IdSeqTsTosHeader::Serialize (Buffer::Iterator start) const{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    i.WriteU8(m_tos);
    IdSeqTsHeader::Serialize(i);
}

uint32_t IdSeqTsTosHeader::Deserialize (Buffer::Iterator start){
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    m_tos = i.ReadU8();
    IdSeqTsHeader::Deserialize(i);
    return GetSerializedSize ();
}


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