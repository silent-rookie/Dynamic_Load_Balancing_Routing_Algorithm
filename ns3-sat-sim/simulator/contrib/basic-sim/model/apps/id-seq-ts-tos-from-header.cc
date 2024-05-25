/**
 * Author: silent-rookie    2024
*/

#include "id-seq-ts-tos-from-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("IdSeqTsTosFromHeader");

NS_OBJECT_ENSURE_REGISTERED (IdSeqTsTosFromHeader);



TypeId IdSeqTsTosFromHeader::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::IdSeqTsTosFromHeader")
            .SetParent<IdSeqTsTosHeader> ()
            .SetGroupName("BasicSim")
            .AddConstructor<IdSeqTsTosFromHeader> ()
    ;
    return tid;
}
    
IdSeqTsTosFromHeader::IdSeqTsTosFromHeader (): 
                IdSeqTsTosHeader(), m_from(-1)
{
    NS_LOG_FUNCTION (this);
}

void IdSeqTsTosFromHeader::SetFrom(int32_t from){
    NS_LOG_FUNCTION (this);
    m_from = from;
}
    
int32_t IdSeqTsTosFromHeader::GetFrom() const {
    return m_from;
}

TypeId IdSeqTsTosFromHeader::GetInstanceTypeId (void) const{
    return GetTypeId ();
}

void IdSeqTsTosFromHeader::Print (std::ostream &os) const{
    NS_LOG_FUNCTION (this << &os);
    os << "(from=" << m_from <<") AND ";
    IdSeqTsTosHeader::Print(os);
}

uint32_t IdSeqTsTosFromHeader::GetSerializedSize (void) const{
    NS_LOG_FUNCTION (this);
    return IdSeqTsTosHeader::GetSerializedSize() + 4;
}

void IdSeqTsTosFromHeader::Serialize (Buffer::Iterator start) const{
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    i.WriteU8(m_from);
    IdSeqTsTosHeader::Serialize(i);
}

uint32_t IdSeqTsTosFromHeader::Deserialize (Buffer::Iterator start){
    NS_LOG_FUNCTION (this << &start);
    Buffer::Iterator i = start;
    m_from = i.ReadU8();
    IdSeqTsTosHeader::Deserialize(i);
    return GetSerializedSize ();
}



}