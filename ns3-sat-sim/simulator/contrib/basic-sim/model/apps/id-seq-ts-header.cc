/**
 * Author: silent-rookie    2024
*/

#include "id-seq-ts-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("IdSeqTsHeader");

NS_OBJECT_ENSURE_REGISTERED (IdSeqTsHeader);



TypeId IdSeqTsHeader::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::IdSeqTsHeader")
            .SetParent<IdSeqHeader> ()
            .SetGroupName("BasicSim")
            .AddConstructor<IdSeqTsHeader> ()
    ;
    return tid;
}
    
IdSeqTsHeader::IdSeqTsHeader (): IdSeqHeader(), 
                                m_ts(Simulator::Now ().GetTimeStep ())
{
    NS_LOG_FUNCTION (this);
}
    
Time IdSeqTsHeader::GetTs (void) const{
    NS_LOG_FUNCTION (this);
    return TimeStep (m_ts);
}

TypeId IdSeqTsHeader::GetInstanceTypeId (void) const{
    return GetTypeId ();
}

void IdSeqTsHeader::Print (std::ostream &os) const{
    NS_LOG_FUNCTION (this << &os);
    os << "(time=" << TimeStep (m_ts).GetSeconds() << ") AND ";
    IdSeqHeader::Print (os);
}

uint32_t IdSeqTsHeader::GetSerializedSize (void) const{
    NS_LOG_FUNCTION (this);
    return IdSeqHeader::GetSerializedSize() + 8;
}

void IdSeqTsHeader::Serialize (Buffer::Iterator start) const{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    i.WriteHtonU64 (m_ts);
    IdSeqHeader::Serialize (i);
}

uint32_t IdSeqTsHeader::Deserialize (Buffer::Iterator start){
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    m_ts = i.ReadNtohU64 ();
    IdSeqHeader::Deserialize (i);
    return GetSerializedSize ();
}



}