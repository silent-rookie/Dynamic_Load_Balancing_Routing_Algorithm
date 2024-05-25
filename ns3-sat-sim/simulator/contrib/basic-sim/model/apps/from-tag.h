/**
 * Author:  silent-rookie      2024
*/

#ifndef FROM_TAG_H
#define FROM_TAG_H

#include "ns3/tag.h"

namespace ns3{

class FromTag : public Tag
{
public:
    static TypeId GetTypeId (void);

    FromTag();
    void SetFrom(uint64_t from);
    uint64_t GetFrom() const;

    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (TagBuffer i) const;
    virtual void Deserialize (TagBuffer i);
    virtual void Print (std::ostream &os) const;

private:
    uint64_t m_from_node_id;
};


}



#endif