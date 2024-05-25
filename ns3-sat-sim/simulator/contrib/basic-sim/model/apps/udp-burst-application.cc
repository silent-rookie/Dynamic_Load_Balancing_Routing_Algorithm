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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/abort.h"
#include <random>

#include "udp-burst-application.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("UdpBurstApplication");

    NS_OBJECT_ENSURE_REGISTERED (UdpBurstApplication);

    int64_t UdpBurstApplication::on_average_period_ms = 0;
    int64_t UdpBurstApplication::off_average_period_ms = 0;
    int64_t UdpBurstApplication::on_shape = 0;
    int64_t UdpBurstApplication::off_shape = 0;
    double UdpBurstApplication::class_A_rate = 0;
    double UdpBurstApplication::class_B_rate = 0;
    double UdpBurstApplication::class_C_rate = 0;

    TypeId
    UdpBurstApplication::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::UdpBurstApplication")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<UdpBurstApplication>()
                .AddAttribute("Port", "Port on which we listen for incoming packets.",
                              UintegerValue(9),
                              MakeUintegerAccessor(&UdpBurstApplication::m_port),
                              MakeUintegerChecker<uint16_t>())
                .AddAttribute ("BaseLogsDir",
                               "Base logging directory (logging will be placed here, i.e. logs_dir/udp_burst_[UDP burst id]_{incoming, outgoing}.csv",
                               StringValue (""),
                               MakeStringAccessor (&UdpBurstApplication::m_baseLogsDir),
                               MakeStringChecker ())
                .AddAttribute("MaxUdpPayloadSizeByte", "Total UDP payload size (byte) before it gets fragmented.",
                              UintegerValue(1472), // 1500 (point-to-point default) - 20 (IP) - 8 (UDP) = 1472
                              MakeUintegerAccessor(&UdpBurstApplication::m_max_udp_payload_size_byte),
                              MakeUintegerChecker<uint32_t>());
        return tid;
    }

    void UdpBurstApplication::Initialize(Ptr<BasicSimulation> basicSimulation){
        on_average_period_ms = parse_positive_int64(basicSimulation->GetConfigParamOrDefault("on_average_period_ms", "200"));
        off_average_period_ms = parse_positive_int64(basicSimulation->GetConfigParamOrDefault("off_average_period_ms", "200"));
        on_shape = parse_positive_int64(basicSimulation->GetConfigParamOrDefault("on_shape", "2"));
        off_shape = parse_positive_int64(basicSimulation->GetConfigParamOrDefault("off_shape", "2"));

        class_A_rate = parse_positive_double(basicSimulation->GetConfigParamOrFail("class_A_rate"));
        class_B_rate = parse_positive_double(basicSimulation->GetConfigParamOrFail("class_B_rate"));
        class_C_rate = parse_positive_double(basicSimulation->GetConfigParamOrFail("class_C_rate"));
        NS_ABORT_MSG_IF(class_A_rate < 0 || class_A_rate > 1, "class A invalid: " + std::to_string(class_A_rate));
        NS_ABORT_MSG_IF(class_B_rate < 0 || class_B_rate > 1, "class B invalid: " + std::to_string(class_B_rate));
        NS_ABORT_MSG_IF(class_C_rate < 0 || class_C_rate > 1, "class C invalid: " + std::to_string(class_C_rate));
        NS_ABORT_MSG_IF(class_A_rate + class_B_rate + class_C_rate != 1, "the sum of 3 class rate not 1");
    }

    UdpBurstApplication::UdpBurstApplication(): 
        m_pareto_time(CreateObject<ParetoRandomVariable>())   
    {
        NS_LOG_FUNCTION(this);
        m_next_internal_burst_idx = 0;
    }

    UdpBurstApplication::~UdpBurstApplication() {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
    }

    uint32_t UdpBurstApplication::GetMaxUdpPayloadSizeByte() {
        return m_max_udp_payload_size_byte;
    }

    void
    UdpBurstApplication::RegisterOutgoingBurst(UdpBurstInfo burstInfo, InetSocketAddress targetAddress, bool enable_precise_logging) {
        NS_ABORT_MSG_IF(burstInfo.GetFromNodeId() != this->GetNode()->GetId(), "Source node identifier is not that of this node.");
        if (m_outgoing_bursts.size() >= 1 && burstInfo.GetStartTimeNs() < std::get<0>(m_outgoing_bursts[m_outgoing_bursts.size() - 1]).GetStartTimeNs()) {
            throw std::runtime_error("Bursts must be added weakly ascending on start time");
        }
        m_outgoing_bursts.push_back(std::make_tuple(burstInfo, targetAddress));

        std::map<TrafficClass, uint64_t> tmap;
        for(TrafficClass classp : TrafficClassVec){
            tmap[classp] = 0;
        }
        m_outgoing_bursts_packets_sent_counter.push_back(tmap);

        m_outgoing_bursts_event_id.push_back(EventId());
        m_outgoing_bursts_enable_precise_logging.push_back(enable_precise_logging);
        if (enable_precise_logging) {
            std::ofstream ofs;
            ofs.open(m_baseLogsDir + "/" + format_string("udp_burst_%" PRIu64 "_outgoing.csv", burstInfo.GetUdpBurstId()));
            ofs.close();
        }
    }

    void
    UdpBurstApplication::RegisterIncomingBurst(UdpBurstInfo burstInfo, bool enable_precise_logging) {
        NS_ABORT_MSG_IF(burstInfo.GetToNodeId() != this->GetNode()->GetId(), "Destination node identifier is not that of this node.");
        m_incoming_bursts.push_back(burstInfo);
        m_incoming_bursts_received_counter[burstInfo.GetUdpBurstId()] = 0;

        std::map<TrafficClass, std::vector<int64_t>> tmap;
        for(TrafficClass classp : TrafficClassVec){
            tmap[classp] = std::vector<int64_t>();
        }
        m_incoming_bursts_packet_delay[burstInfo.GetUdpBurstId()] = tmap;

        m_incoming_bursts_enable_precise_logging[burstInfo.GetUdpBurstId()] = enable_precise_logging;
        if (enable_precise_logging) {
            std::ofstream ofs;
            ofs.open(m_baseLogsDir + "/" + format_string("udp_burst_%" PRIu64 "_incoming.csv", burstInfo.GetUdpBurstId()));
            ofs.close();
        }
    }

    void
    UdpBurstApplication::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    void
    UdpBurstApplication::StartApplication(void) {
        NS_LOG_FUNCTION(this);

        // Bind a socket to the UDP port
        if (m_socket == 0) {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            if (m_socket->Bind(local) == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
        }

        // Receive of packets
        m_socket->SetRecvCallback(MakeCallback(&UdpBurstApplication::HandleRead, this));

        // First process call is for the start of the first burst
        if (m_outgoing_bursts.size() > 0) {
            m_startNextBurstEvent = Simulator::Schedule(NanoSeconds(std::get<0>(m_outgoing_bursts[0]).GetStartTimeNs()), &UdpBurstApplication::StartNextBurst, this);
        }

    }

    void
    UdpBurstApplication::StartNextBurst()
    {
        int64_t now_ns = Simulator::Now().GetNanoSeconds();

        // If this function is called, there must be a next burst
        if (m_next_internal_burst_idx >= m_outgoing_bursts.size() || std::get<0>(m_outgoing_bursts[m_next_internal_burst_idx]).GetStartTimeNs() != now_ns) {
            throw std::runtime_error("No next burst available; this function should not have been called.");
        }

        // schedule upd burst with pareto distribution
        ScheduleWithPareto(m_next_internal_burst_idx);

        // Schedule the start of the next burst if there are more
        m_next_internal_burst_idx += 1;
        if (m_next_internal_burst_idx < m_outgoing_bursts.size()) {
            m_startNextBurstEvent = Simulator::Schedule(NanoSeconds(std::get<0>(m_outgoing_bursts[m_next_internal_burst_idx]).GetStartTimeNs() - now_ns), &UdpBurstApplication::StartNextBurst, this);
        }

    }

    void 
    UdpBurstApplication::ScheduleWithPareto(size_t internal_burst_idx)
    {
        Time on_time_interval = MilliSeconds(m_pareto_time->GetValue(on_average_period_ms, on_shape, 0));
        Time on_end_time = on_time_interval + Simulator::Now();
        // Start the self-calling (and self-ending) process of sending out packets of the burst
        BurstSendOut(internal_burst_idx, on_end_time);
        // plan next period
        Time off_time_interval = MilliSeconds(m_pareto_time->GetValue(off_average_period_ms, off_shape, 0));
        Time next_period = on_end_time + off_time_interval;
        UdpBurstInfo info = std::get<0>(m_outgoing_bursts[internal_burst_idx]);
        if(next_period < NanoSeconds(info.GetStartTimeNs() + info.GetDurationNs())){
            Simulator::Schedule(on_time_interval + off_time_interval, &UdpBurstApplication::ScheduleWithPareto, this, internal_burst_idx);
        }
    }

    void
    UdpBurstApplication::BurstSendOut(size_t internal_burst_idx, Time on_end_time)
    {

        // Send out the packet as desired
        TransmitFullPacket(internal_burst_idx);

        // Schedule the next if the packet gap would not exceed the rate
        uint64_t now_ns = Simulator::Now().GetNanoSeconds();
        UdpBurstInfo info = std::get<0>(m_outgoing_bursts[internal_burst_idx]);
        uint64_t packet_gap_nanoseconds = std::ceil(1500.0 / (info.GetTargetRateMegabitPerSec() / 8000.0));
        if (now_ns + packet_gap_nanoseconds < (uint64_t) (info.GetStartTimeNs() + info.GetDurationNs()) && 
            Simulator::Now() + NanoSeconds(packet_gap_nanoseconds) <= on_end_time) {   // we will lost some time to send pack, but it doesn't matter much
            m_outgoing_bursts_event_id.at(internal_burst_idx) = Simulator::Schedule(NanoSeconds(packet_gap_nanoseconds), &UdpBurstApplication::BurstSendOut, this, internal_burst_idx, on_end_time);
        }

    }

    void
    UdpBurstApplication::TransmitFullPacket(size_t internal_burst_idx) {

        // Header with (udp_burst_id, seq_no)
        IdSeqTsHeader header;
        header.SetId(std::get<0>(m_outgoing_bursts[internal_burst_idx]).GetUdpBurstId());
        uint64_t seq = 0;
        for(TrafficClass classp : TrafficClassVec){
            seq += m_outgoing_bursts_packets_sent_counter.at(internal_burst_idx).at(classp);
        }
        header.SetSeq(seq);

        TrafficClass pclass = GenerateRandomTrafficClass();

        // One more packet will be sent out
        m_outgoing_bursts_packets_sent_counter[internal_burst_idx][pclass] += 1;

        // Log precise timestamp sent away of the sequence packet if needed
        if (m_outgoing_bursts_enable_precise_logging[internal_burst_idx]) {
            std::ofstream ofs;
            ofs.open(m_baseLogsDir + "/" + format_string("udp_burst_%" PRIu64 "_outgoing.csv", header.GetId()), std::ofstream::out | std::ofstream::app);
            ofs << header.GetId() << "," << header.GetSeq() << "," << Simulator::Now().GetNanoSeconds() << ",";
            ofs << TrafficClass2String(pclass) << std::endl;

            ofs.close();
        }

        // A full payload packet
        Ptr<Packet> p = Create<Packet>(m_max_udp_payload_size_byte - header.GetSerializedSize());
        p->AddHeader(header);

        // Add flow tos tag for traffic claffify.
        // Note!!! we can not use IdSeqTsTosHeader to identify traffic class
        // because the packet will add udpheader.
        FlowTosTag tag;
        tag.SetTos(static_cast<uint8_t>(pclass));
        p->AddPacketTag(tag);

        // Send out the packet to the target address
        m_socket->SendTo(p, 0, std::get<1>(m_outgoing_bursts[internal_burst_idx]));

    }

    void
    UdpBurstApplication::StopApplication() {
        NS_LOG_FUNCTION(this);
        if (m_socket != 0) {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
            Simulator::Cancel(m_startNextBurstEvent);
            for (EventId& eventId : m_outgoing_bursts_event_id) {
                Simulator::Cancel(eventId);
            }
        }
    }

    void
    UdpBurstApplication::HandleRead(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packet;
        Address from;
        while ((packet = socket->RecvFrom(from))) {

            FlowTosTag tag;
            bool found = packet->PeekPacketTag(tag);
            if(!found){
                Icmpv4TimeExceeded icmp;
                packet->PeekHeader(icmp);
                Ipv4Header header = icmp.GetHeader();
                if(header.GetTtl() == 0){
                    // Icmp packet(see Ipv4L3Protocol::IpForward, line 1066, and Icmpv4L4Protocol::SendTimeExceededTtl)
                    std::cout << "Receive a Icmp packet" << std::endl;

                    // we can do more check: if(ResolveNodeIdFromIp(header.GetSource().Get()) == (uint32_t)target_node_id).
                    // but I think it is time-consuming, so I did not use it.
                }
                else{
                    NS_ABORT_MSG("Receive a unexpected packet");
                }
            }

            // Extract burst identifier and packet sequence number
            IdSeqTsHeader header;
            packet->RemoveHeader (header);

            // Count packets from incoming bursts
            m_incoming_bursts_received_counter.at(header.GetId()) += 1;

            // Calculate end-to-end delay
            int64_t now_ns = Simulator::Now().GetNanoSeconds();
            int64_t delay = now_ns - header.GetTs().GetNanoSeconds();
            TrafficClass pclass = Tos2TrafficClass(tag.GetTos());
            m_incoming_bursts_packet_delay.at(header.GetId()).at(pclass).push_back(delay);

            // Log precise timestamp received of the sequence packet if needed
            if (m_incoming_bursts_enable_precise_logging[header.GetId()]) {
                std::ofstream ofs;
                ofs.open(m_baseLogsDir + "/" + format_string("udp_burst_%" PRIu64 "_incoming.csv", header.GetId()), std::ofstream::out | std::ofstream::app);
                ofs << header.GetId() << "," << header.GetSeq() << "," << now_ns << "," << delay << ",";
                ofs << TrafficClass2String(pclass) << std::endl;
                
                ofs.close();
            }

        }
    }

    uint64_t 
    UdpBurstApplication::GetSendCounterOf(int64_t udp_burst_id){
        for (size_t i = 0; i < m_outgoing_bursts.size(); i++) {
            if (std::get<0>(m_outgoing_bursts[i]).GetUdpBurstId() == udp_burst_id) {
                uint64_t res = 0;
                for(TrafficClass classp : TrafficClassVec){
                    res += m_outgoing_bursts_packets_sent_counter.at(i).at(classp);
                }
                return res;
            }
        }
        throw std::runtime_error("Send counter for unknown UDP burst ID was requested");
    }

    uint64_t
    UdpBurstApplication::GetReceivedCounterOf(int64_t udp_burst_id) {
        return m_incoming_bursts_received_counter.at(udp_burst_id);
    }

    std::map<TrafficClass, uint64_t> 
    UdpBurstApplication::GetSendCounterTrafficOf(int64_t udp_burst_id){
        for (size_t i = 0; i < m_outgoing_bursts.size(); i++) {
            if (std::get<0>(m_outgoing_bursts[i]).GetUdpBurstId() == udp_burst_id) {
                return m_outgoing_bursts_packets_sent_counter.at(i);
            }
        }
        throw std::runtime_error("Traffic Send counter for unknown UDP burst ID was requested");
    }

    std::map<TrafficClass, uint64_t> 
    UdpBurstApplication::GetReceivedCounterTrafficOf(int64_t udp_burst_id){
        std::map<TrafficClass, uint64_t> res;
        for(TrafficClass classp : TrafficClassVec){
            res[classp] = m_incoming_bursts_packet_delay.at(udp_burst_id).at(classp).size();
        }
        return res;
    }

    std::map<TrafficClass, int64_t> 
    UdpBurstApplication::GetReceivedDelayNS(int64_t udp_burst_id){
        std::map<TrafficClass, int64_t> res;
        std::map<TrafficClass, std::vector<int64_t>>& tmap = m_incoming_bursts_packet_delay.at(udp_burst_id);

        for(TrafficClass classp : TrafficClassVec){
            size_t sz = tmap.at(classp).size();
            double tmp = 0;
            for(int64_t p : tmap.at(classp)){
                tmp += (double)p / sz;
            }
            res[classp] = tmp;
        }

        return res;
    }

    TrafficClass 
    UdpBurstApplication::GenerateRandomTrafficClass()
    {
        NS_ABORT_MSG_IF(class_A_rate + class_B_rate + class_C_rate != 1, "class rate have not been initialize");

        std::vector<double> probabilities = {class_A_rate, class_B_rate, class_C_rate};
        std::discrete_distribution<int> dist(probabilities.begin(), probabilities.end());

        std::random_device rd;  // random seed
        std::mt19937 gen(rd()); // use Mersenne Twister engine
        int random_index = dist(gen);

        switch (random_index)
        {
            case 0:
                return TrafficClass::class_A;
            case 1:
                return TrafficClass::class_B;
            case 2:
                return TrafficClass::class_C;
            default:
                NS_ABORT_MSG("generate random index error: " + std::to_string(random_index));
        }
    }

} // Namespace ns3
