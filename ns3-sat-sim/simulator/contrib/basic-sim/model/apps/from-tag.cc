/**
 * Author:  silent-rookie      2024
*/

#include "from-tag.h"

namespace ns3{

TypeId FromTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FromTag")
    .SetParent<Tag> ()
    .SetGroupName("BasicSim")
    .AddConstructor<FromTag> ()
    ;
  return tid;
}


FromTag::FromTag(): m_from_node_id(-1)
{

}

void FromTag::SetFrom(uint64_t from){
    m_from_node_id = from;
}
    
uint64_t FromTag::GetFrom() const{
    return m_from_node_id;
}

TypeId FromTag::GetInstanceTypeId (void) const{
    return GetTypeId();
}

uint32_t FromTag::GetSerializedSize (void) const{
    return sizeof (uint64_t);
}

void FromTag::Serialize (TagBuffer i) const{
    i.WriteU64(m_from_node_id);
}

void FromTag::Deserialize (TagBuffer i){
    m_from_node_id = i.ReadU64();
}

void FromTag::Print (std::ostream &os) const{
    os << "From_Node_ID = " << m_from_node_id;
}



}