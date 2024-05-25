/**
 * Author:  silent-rookie      2024
*/

#ifndef FLOW_TOS_TAG_H
#define FLOW_TOS_TAG_H

#include "ns3/tag.h"

namespace ns3{

class FlowTosTag : public Tag
{
public:
    static TypeId GetTypeId (void);

    FlowTosTag();
    void SetTos(uint8_t tos);
    uint8_t GetTos() const;

    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (TagBuffer i) const;
    virtual void Deserialize (TagBuffer i);
    virtual void Print (std::ostream &os) const;

private:
    uint8_t m_tos;
};


}



#endif