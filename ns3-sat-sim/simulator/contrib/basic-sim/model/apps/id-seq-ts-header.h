/**
 * Author: silent-rookie    2024
*/

#ifndef ID_SEQ_TS_HEADER_H
#define ID_SEQ_TS_HEADER_H

#include "ns3/id-seq-header.h"
#include "ns3/nstime.h"

namespace ns3{
    
class IdSeqTsHeader : public IdSeqHeader{
public:
    static TypeId GetTypeId (void);
    
    IdSeqTsHeader ();
    
    Time GetTs (void) const;

    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

private:
    uint64_t m_ts;      // TimeStamp
};



}




#endif