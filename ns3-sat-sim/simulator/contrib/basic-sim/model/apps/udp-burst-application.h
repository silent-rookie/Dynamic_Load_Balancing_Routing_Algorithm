/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef UDP_BURST_APPLICATION_H
#define UDP_BURST_APPLICATION_H

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include "ns3/seq-ts-header.h"
#include "ns3/udp-burst-info.h"
#include "ns3/string.h"
#include "ns3/exp-util.h"
#include "ns3/random-variable-stream.h"
#include "ns3/basic-simulation.h"
#include "ns3/id-seq-ts-tos-header.h"
#include "ns3/flow-tos-tag.h"

namespace ns3 {

    class Socket;
    class Packet;

    class UdpBurstApplication : public Application
    {
    public:
        static TypeId GetTypeId (void);
        UdpBurstApplication ();
        virtual ~UdpBurstApplication ();
        uint32_t GetMaxUdpPayloadSizeByte();
        void RegisterOutgoingBurst(UdpBurstInfo burstInfo, InetSocketAddress targetAddress, bool enable_precise_logging);
        void RegisterIncomingBurst(UdpBurstInfo burstInfo, bool enable_precise_logging);
        void StartNextBurst();
        void BurstSendOut(size_t internal_burst_idx, Time on_end_time);
        void TransmitFullPacket(size_t internal_burst_idx);
        uint64_t GetSendCounterOf(int64_t udp_burst_id);
        uint64_t GetReceivedCounterOf(int64_t udp_burst_id);
        std::map<TrafficClass, uint64_t> GetSendCounterTrafficOf(int64_t udp_burst_id);
        std::map<TrafficClass, uint64_t> GetReceivedCounterTrafficOf(int64_t udp_burst_id);
        std::map<TrafficClass, int64_t> GetReceivedDelayNS(int64_t udp_burst_id);

        static void Initialize(Ptr<BasicSimulation> basicSimulation);

    protected:
        virtual void DoDispose (void);

    private:
        virtual void StartApplication (void);
        virtual void StopApplication (void);
        void HandleRead (Ptr<Socket> socket);

        void ScheduleWithPareto(size_t internal_burst_idx);
        TrafficClass GenerateRandomTrafficClass();
        

        uint16_t m_port;      //!< Port on which we listen for incoming packets.
        uint32_t m_max_udp_payload_size_byte;  //!< Maximum size of UDP payload before getting fragmented
        Ptr<Socket> m_socket; //!< IPv4 Socket
        std::string m_baseLogsDir; //!< Where the UDP burst logs will be written to:
                                   //!<   logs_dir/udp_burst_[id]_{incoming, outgoing}.csv
        EventId m_startNextBurstEvent; //!< Event to start next burst

        // Outgoing bursts
        std::vector<std::tuple<UdpBurstInfo, InetSocketAddress>> m_outgoing_bursts; //!< Weakly ascending on start time list of bursts
        std::vector<std::map<TrafficClass, uint64_t>> m_outgoing_bursts_packets_sent_counter; //!< Amount of UDP packets sent out already for each burst
        std::vector<EventId> m_outgoing_bursts_event_id; //!< Event ID of the outgoing burst send loop
        std::vector<bool> m_outgoing_bursts_enable_precise_logging; //!< True iff enable precise logging for each burst
        size_t m_next_internal_burst_idx; //!< Next burst index to send out

        // Incoming bursts
        std::vector<UdpBurstInfo> m_incoming_bursts;
        std::map<int64_t, uint64_t> m_incoming_bursts_received_counter;       //!< Counter for how many packets received
        std::map<int64_t, uint64_t> m_incoming_bursts_enable_precise_logging; //!< True iff enable precise logging for each burst
        std::map<int64_t, std::map<TrafficClass, std::vector<int64_t>>> m_incoming_bursts_packet_delay; // !< Delay of each packet received

        // On Off argument(pareto distribution)
        static int64_t on_average_period_ms;
        static int64_t off_average_period_ms;
        static int64_t on_shape;
        static int64_t off_shape;
        Ptr<ParetoRandomVariable> m_pareto_time;

        // Traffic classification
        static double class_A_rate;     //  has the highest priority and the delay-sensitive interactive applications such as VoIP are involved;
                                        //  never detour
        static double class_B_rate;     //  consisting of relatively delay-robust applications such as real-time video streaming applications 
                                        //  only detour in LEO layer
        static double class_C_rate;     //  represents best effort traffic, has robustness to long delays and delay changes
                                        //  detour to GEO satellites
    };

} // namespace ns3

#endif /* UDP_BURST_APPLICATION_H */
