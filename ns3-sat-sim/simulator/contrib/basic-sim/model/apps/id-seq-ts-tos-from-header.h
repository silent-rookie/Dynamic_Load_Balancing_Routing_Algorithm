/**
 * Author: silent-rookie    2024
*/

#ifndef ID_SEQ_TS_TOS_FROM_HEADER_H
#define ID_SEQ_TS_TOS_FROM_HEADER_H

#include "ns3/id-seq-ts-tos-header.h"
#include "ns3/nstime.h"

namespace ns3{
    
class IdSeqTsTosFromHeader : public IdSeqTsTosHeader{
public:
    static TypeId GetTypeId (void);
    
    IdSeqTsTosFromHeader ();
    
    void SetFrom(int32_t from);
    int32_t GetFrom() const;

    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

private:
    int32_t m_from;
};



}




#endif