/**
 * Author:  silent-rookie      2024
*/

#include "flow-tos-tag.h"

namespace ns3{

TypeId FlowTosTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::FlowTosTag")
    .SetParent<Tag> ()
    .SetGroupName("BasicSim")
    .AddConstructor<FlowTosTag> ()
    ;
  return tid;
}


FlowTosTag::FlowTosTag(): m_tos(0)
{

}

void FlowTosTag::SetTos(uint8_t tos){
    m_tos = tos;
}
    
uint8_t FlowTosTag::GetTos() const{
    return m_tos;
}

TypeId FlowTosTag::GetInstanceTypeId (void) const{
    return GetTypeId();
}

uint32_t FlowTosTag::GetSerializedSize (void) const{
    return sizeof (uint8_t);
}

void FlowTosTag::Serialize (TagBuffer i) const{
    i.WriteU8(m_tos);
}

void FlowTosTag::Deserialize (TagBuffer i){
    m_tos = i.ReadU8();
}

void FlowTosTag::Print (std::ostream &os) const{
    os << "tos = " << m_tos;
}



}