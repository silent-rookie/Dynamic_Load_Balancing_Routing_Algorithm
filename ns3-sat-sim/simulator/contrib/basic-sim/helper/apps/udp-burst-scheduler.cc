/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 ETH Zurich
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
 *
 * Author: Simon
 * Originally based on the TcpFlowScheduler (which is authored by Simon, Hussain)
 */

#include "udp-burst-scheduler.h"

namespace ns3 {

    UdpBurstScheduler::UdpBurstScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology) {
        printf("UDP BURST SCHEDULER\n");

        m_basicSimulation = basicSimulation;
        m_topology = topology;

        // Check if it is enabled explicitly
        m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_udp_burst_scheduler", "false"));
        if (!m_enabled) {
            std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

        } else {
            std::cout << "  > UDP burst scheduler is enabled" << std::endl;

            // Properties we will use often
            m_nodes = m_topology->GetNodes();
            m_simulation_end_time_ns = m_basicSimulation->GetSimulationEndTimeNs();
            m_enable_logging_for_udp_burst_ids = parse_set_positive_int64(m_basicSimulation->GetConfigParamOrDefault("udp_burst_enable_logging_for_udp_burst_ids", "set()"));

            // Distributed run information
            m_system_id = m_basicSimulation->GetSystemId();
            m_enable_distributed = m_basicSimulation->IsDistributedEnabled();
            m_distributed_node_system_id_assignment = m_basicSimulation->GetDistributedNodeSystemIdAssignment();

            // Read schedule
            m_schedule = read_udp_burst_schedule(
                    m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("udp_burst_schedule_filename"),
                    m_topology,
                    m_simulation_end_time_ns
            );

            // Check that the UDP burst IDs exist in the logging
            for (int64_t udp_burst_id : m_enable_logging_for_udp_burst_ids) {
                if ((size_t) udp_burst_id >= m_schedule.size()) {
                    throw std::invalid_argument("Invalid UDP burst ID in udp_burst_enable_logging_for_udp_burst_ids: " + std::to_string(udp_burst_id));
                }
            }

            // Schedule read
            printf("  > Read schedule (total UDP bursts: %lu)\n", m_schedule.size());
            m_basicSimulation->RegisterTimestamp("Read UDP burst schedule");

            // Determine filenames
            if (m_enable_distributed) {
                m_udp_bursts_outgoing_csv_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts_outgoing.csv";
                m_udp_bursts_outgoing_txt_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts_outgoing.txt";
                m_udp_bursts_incoming_csv_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts_incoming.csv";
                m_udp_bursts_incoming_txt_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts_incoming.txt";
            } else {
                m_udp_bursts_outgoing_csv_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_outgoing.csv";
                m_udp_bursts_outgoing_txt_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_outgoing.txt";
                m_udp_bursts_incoming_csv_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_incoming.csv";
                m_udp_bursts_incoming_txt_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_incoming.txt";

                m_udp_bursts_performance_csv_filename = m_basicSimulation->GetRunDir() + "/algorithm_performance/udp_bursts_performance.csv";
                m_udp_bursts_performance_txt_filename = m_basicSimulation->GetRunDir() + "/algorithm_performance/udp_bursts_performance.txt";
                m_algorithm_performance_csv_filename = m_basicSimulation->GetRunDir() + "/algorithm_performance/algorithm_performance.csv";
                m_algorithm_performance_txt_filename = m_basicSimulation->GetRunDir() + "/algorithm_performance/algorithm_performance.txt";
            }

            // Create algorithm
            mkdir_if_not_exists(m_basicSimulation->GetRunDir() + "/algorithm_performance");

            // Remove files if they are there
            remove_file_if_exists(m_udp_bursts_outgoing_csv_filename);
            remove_file_if_exists(m_udp_bursts_outgoing_txt_filename);
            remove_file_if_exists(m_udp_bursts_incoming_csv_filename);
            remove_file_if_exists(m_udp_bursts_incoming_txt_filename);
            remove_file_if_exists(m_udp_bursts_performance_csv_filename);
            remove_file_if_exists(m_udp_bursts_performance_txt_filename);
            remove_file_if_exists(m_algorithm_performance_csv_filename);
            remove_file_if_exists(m_algorithm_performance_txt_filename);
            printf("  > Removed previous UDP burst log files if present\n");
            m_basicSimulation->RegisterTimestamp("Remove previous UDP burst log files");

            // Initialize pareto argument
            UdpBurstApplication::Initialize(basicSimulation);

            // Install sink on each endpoint node
            std::cout << "  > Setting up UDP burst applications on all endpoint nodes" << std::endl;
            for (int64_t endpoint : m_topology->GetEndpoints()) {
                if (!m_enable_distributed || m_distributed_node_system_id_assignment[endpoint] == m_system_id) {

                    // Setup the application
                    UdpBurstHelper udpBurstHelper(1026, m_basicSimulation->GetLogsDir());
                    ApplicationContainer app = udpBurstHelper.Install(m_nodes.Get(endpoint));
                    app.Start(Seconds(0.0));
                    m_apps.push_back(app);

                    // Register all bursts being sent from there and being received
                    Ptr<UdpBurstApplication> udpBurstApp = app.Get(0)->GetObject<UdpBurstApplication>();
                    for (UdpBurstInfo entry : m_schedule) {
                        if (entry.GetFromNodeId() == endpoint) {
                            udpBurstApp->RegisterOutgoingBurst(
                                    entry,
                                    InetSocketAddress(m_nodes.Get(entry.GetToNodeId())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                                    m_enable_logging_for_udp_burst_ids.find(entry.GetUdpBurstId()) != m_enable_logging_for_udp_burst_ids.end()
                            );
                            m_responsible_for_outgoing_bursts.push_back(std::make_pair(entry, udpBurstApp));
                        }
                        if (entry.GetToNodeId() == endpoint) {
                            udpBurstApp->RegisterIncomingBurst(
                                    entry,
                                    m_enable_logging_for_udp_burst_ids.find(entry.GetUdpBurstId()) != m_enable_logging_for_udp_burst_ids.end()
                            );
                            m_responsible_for_incoming_bursts.push_back(std::make_pair(entry, udpBurstApp));
                        }
                    }

                }
            }
            m_basicSimulation->RegisterTimestamp("Setup UDP burst applications");

        }

        std::cout << std::endl;
    }

    void UdpBurstScheduler::WritePerformance(){
        std::cout << "\n  > Opening algorithm performance files:" << std::endl;
        FILE* file_udp_csv = fopen(m_udp_bursts_performance_csv_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_udp_bursts_performance_csv_filename << std::endl;
        FILE* file_udp_txt = fopen(m_udp_bursts_performance_txt_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_udp_bursts_performance_txt_filename << std::endl;
        FILE* file_csv = fopen(m_algorithm_performance_csv_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_algorithm_performance_csv_filename << std::endl;
        FILE* file_txt = fopen(m_algorithm_performance_txt_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_algorithm_performance_txt_filename << std::endl;

        fprintf(
                file_udp_txt, "%-16s%-10s%-10s%-20s%-16s%-24s%-24s%-24s%-24s%-28s%-28s%-28s%-28s%-28s%-28s%-28s%-28s\n",
                "UDP burst ID", "From", "To", "Target rate(Mbps)", "Duration(s)",
                "Packet Drop Rate(A)", "Packet Drop Rate(B)", "Packet Drop Rate(C)", "Packet Drop Rate(d)",
                "Throughput(Mbps)(A)", "Throughput(Mbps)(B)", "Throughput(Mbps)(C)", "Throughput(Mbps)(d)",
                "End-To-End Delay(ms)(A)", "End-To-End Delay(ms)(B)", "End-To-End Delay(ms)(C)", "End-To-End Delay(ms)(d)"
        );

        fprintf(
                file_txt, "%-16s%-28s%-16s%-24s%-24s%-24s%-24s%-28s%-28s%-28s%-28s%-28s%-28s%-28s%-28s\n",
                "UDP pairs num", "UDP burst rate(Mbps)", "Duration(s)",
                "Packet Drop Rate(A)", "Packet Drop Rate(B)", "Packet Drop Rate(C)", "Packet Drop Rate(d)",
                "Throughput(Mbps)(A)", "Throughput(Mbps)(B)", "Throughput(Mbps)(C)", "Throughput(Mbps)(d)",
                "End-To-End Delay(ms)(A)", "End-To-End Delay(ms)(B)", "End-To-End Delay(ms)(C)", "End-To-End Delay(ms)(d)"
        );

        std::map<TrafficClass, double> total_drop_rate;
        std::map<TrafficClass, double> total_throughput;
        std::map<TrafficClass, double> total_endtoend_delay;
        for(TrafficClass pclass : TrafficClassVec){
            total_drop_rate[pclass] = total_throughput[pclass] = total_endtoend_delay[pclass] = 0;
        }

        for(size_t i = 0; i < m_schedule.size(); ++i){
            UdpBurstInfo info = m_responsible_for_incoming_bursts[i].first;
            Ptr<UdpBurstApplication> app_out = m_responsible_for_outgoing_bursts[i].second;
            Ptr<UdpBurstApplication> app_in = m_responsible_for_incoming_bursts[i].second;
            std::map<TrafficClass, uint64_t> send_conter = app_out->GetSendCounterTrafficOf(info.GetUdpBurstId());
            std::map<TrafficClass, uint64_t> recieve_conter = app_in->GetReceivedCounterTrafficOf(info.GetUdpBurstId());
            std::map<TrafficClass, int64_t> recieve_delay = app_in->GetReceivedDelayNS(info.GetUdpBurstId());

            std::map<TrafficClass, double> tmp_drop_rate;
            std::map<TrafficClass, double> tmp_throughput;
            std::map<TrafficClass, double> tmp_endtoend_delay;
            for(TrafficClass pclass : TrafficClassVec){
                tmp_drop_rate[pclass] = tmp_throughput[pclass] = tmp_endtoend_delay[pclass] = 0;
            }

            for(TrafficClass pclass : TrafficClassVec){
                // packet drop rate
                double rate1 = send_conter.at(pclass) == 0 ? 1 : (double)recieve_conter.at(pclass) / send_conter.at(pclass);
                rate1 = (1 - rate1) * 100;
                tmp_drop_rate.at(pclass) = rate1;
                total_drop_rate.at(pclass) += rate1 / m_schedule.size();

                // throughput mbps(we use incoming rate)
                uint32_t complete_packet_size = 1500;
                int64_t effective_duration_ns = info.GetStartTimeNs() + info.GetDurationNs() >= m_simulation_end_time_ns ? 
                                                m_simulation_end_time_ns - info.GetStartTimeNs() : info.GetDurationNs();
                double rate2 = byte_to_megabit(recieve_conter.at(pclass) * complete_packet_size) / nanosec_to_sec(effective_duration_ns);
                tmp_throughput.at(pclass) = rate2;
                total_throughput.at(pclass) += rate2 / m_schedule.size();

                // end-to-end delay
                double delay = nanosec_to_millisec(recieve_delay.at(pclass));
                tmp_endtoend_delay.at(pclass) = delay;
                total_endtoend_delay.at(pclass) += delay / m_schedule.size();
            }

            // Write to udp csv file
            fprintf(
                    file_udp_csv, "%" PRId64 ",%" PRId64 ",%" PRId64 ",%.5f,%.5f",
                    info.GetUdpBurstId(), info.GetFromNodeId(), info.GetToNodeId(), 
                    info.GetTargetRateMegabitPerSec(), nanosec_to_sec(info.GetDurationNs())
            );
            for(TrafficClass pclass : TrafficClassVec) fprintf(file_udp_csv, ",%.5f", tmp_drop_rate.at(pclass));
            for(TrafficClass pclass : TrafficClassVec) fprintf(file_udp_csv, ",%.5f", tmp_throughput.at(pclass));
            for(TrafficClass pclass : TrafficClassVec) fprintf(file_udp_csv, ",%.5f", tmp_endtoend_delay.at(pclass));
            fprintf(file_udp_csv, "\n");

            // Write to udp txt file
            char str_target_rate[100];
            sprintf(str_target_rate, "%.5f Mbit/s", info.GetTargetRateMegabitPerSec());
            char str_duration_s[100];
            sprintf(str_duration_s, "%.2f s", nanosec_to_sec(info.GetDurationNs()));
            fprintf(
                file_udp_txt,
                "%-16" PRId64 "%-10" PRId64 "%-10" PRId64 "%-20s%-16s",
                info.GetUdpBurstId(), info.GetFromNodeId(), info.GetToNodeId(), str_target_rate, str_duration_s
            );
            for(TrafficClass pclass : TrafficClassVec){
                char str_drop[100];
                sprintf(str_drop, "%.5f%%", tmp_drop_rate.at(pclass));
                fprintf(file_udp_txt, "%-24s", str_drop);
            }
            for(TrafficClass pclass : TrafficClassVec){
                char str_put[100];
                sprintf(str_put, "%.5f Mbit/s", tmp_throughput.at(pclass));
                fprintf(file_udp_txt, "%-28s", str_put);
            }
            for(TrafficClass pclass : TrafficClassVec){
                char str_delay[100];
                sprintf(str_delay, "%.5f ms", tmp_endtoend_delay.at(pclass));
                fprintf(file_udp_txt, "%-28s", str_delay);
            }
            fprintf(file_udp_txt, "\n");
        }

        // Write to csv file
        size_t num_info = m_schedule.size();
        double target_rate = m_schedule[0].GetTargetRateMegabitPerSec();
        double duration_s = nanosec_to_sec(m_schedule[0].GetDurationNs());
        fprintf(
                file_csv, "%ld,%.5f,%.5f", num_info, target_rate, duration_s
        );
        for(TrafficClass pclass : TrafficClassVec) fprintf(file_csv, ",%.5f", total_drop_rate.at(pclass));
        for(TrafficClass pclass : TrafficClassVec) fprintf(file_csv, ",%.5f", total_throughput.at(pclass));
        for(TrafficClass pclass : TrafficClassVec) fprintf(file_csv, ",%.5f", total_endtoend_delay.at(pclass));
        fprintf(file_csv, "\n");

        // Write to txt file
        char str_num[100];
        sprintf(str_num, "%ld", num_info);
        char str_target_rate[100];
        sprintf(str_target_rate, "%.5f Mbit/s", target_rate);
        char str_duration_s[100];
        sprintf(str_duration_s, "%.2f s", duration_s);
        fprintf(
                file_txt, "%-16s%-28s%-16s", str_num, str_target_rate, str_duration_s
        );
        for(TrafficClass pclass : TrafficClassVec){
            char str_drop[100];
            sprintf(str_drop, "%.5f%%", total_drop_rate.at(pclass));
            fprintf(file_txt, "%-24s", str_drop);
        }
        for(TrafficClass pclass : TrafficClassVec){
            char str_put[100];
            sprintf(str_put, "%.5f Mbit/s", total_throughput.at(pclass));
            fprintf(file_txt, "%-28s", str_put);
        }
        for(TrafficClass pclass : TrafficClassVec){
            char str_delay[100];
            sprintf(str_delay, "%.5f ms", total_endtoend_delay.at(pclass));
            fprintf(file_txt, "%-28s", str_delay);
        }
        fprintf(file_txt, "\n");

        // Close files
        std::cout << "  > Closing performance log files:" << std::endl;
        fclose(file_udp_csv);
        std::cout << "    >> Closed: " << m_udp_bursts_performance_csv_filename << std::endl;
        fclose(file_udp_txt);
        std::cout << "    >> Closed: " << m_udp_bursts_performance_txt_filename << std::endl;
        fclose(file_csv);
        std::cout << "    >> Closed: " << m_algorithm_performance_csv_filename << std::endl;
        fclose(file_txt);
        std::cout << "    >> Closed: " << m_algorithm_performance_txt_filename << std::endl;


    }

    void UdpBurstScheduler::WriteResults() {
        std::cout << "STORE UDP BURST RESULTS" << std::endl;

        // Check if it is enabled explicitly
        if (!m_enabled) {
            std::cout << "  > Not enabled, so no UDP burst results are written" << std::endl;

        } else {

            // Open files
            std::cout << "  > Opening UDP burst log files:" << std::endl;
            FILE* file_outgoing_csv = fopen(m_udp_bursts_outgoing_csv_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_outgoing_csv_filename << std::endl;
            FILE* file_outgoing_txt = fopen(m_udp_bursts_outgoing_txt_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_outgoing_txt_filename << std::endl;
            FILE* file_incoming_csv = fopen(m_udp_bursts_incoming_csv_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_incoming_csv_filename << std::endl;
            FILE* file_incoming_txt = fopen(m_udp_bursts_incoming_txt_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_incoming_txt_filename << std::endl;

            // Header
            std::cout << "  > Writing udp_bursts_{incoming, outgoing}.txt headers" << std::endl;
            fprintf(
                    file_outgoing_txt, "%-16s%-10s%-10s%-20s%-16s%-16s%-28s%-28s%-16s%-28s%-28s%s\n",
                    "UDP burst ID", "From", "To", "Target rate", "Start time", "Duration",
                    "Outgoing rate (w/ headers)", "Outgoing rate (payload)", "Packets sent",
                    "Data sent (w/headers)", "Data sent (payload)", "Metadata"
            );
            fprintf(
                    file_incoming_txt, "%-16s%-10s%-10s%-20s%-16s%-16s%-28s%-28s%-19s%-28s%-28s%s\n",
                    "UDP burst ID", "From", "To", "Target rate", "Start time", "Duration",
                    "Incoming rate (w/ headers)", "Incoming rate (payload)", "Packets received",
                    "Data received (w/headers)", "Data received (payload)", "Metadata"
            );

            // Sort ascending to preserve UDP burst schedule order
            struct ascending_paired_udp_burst_id_key
            {
                inline bool operator() (const std::pair<UdpBurstInfo, Ptr<UdpBurstApplication>>& a, const std::pair<UdpBurstInfo, Ptr<UdpBurstApplication>>& b)
                {
                    return (a.first.GetUdpBurstId() < b.first.GetUdpBurstId());
                }
            };
            std::sort(m_responsible_for_outgoing_bursts.begin(), m_responsible_for_outgoing_bursts.end(), ascending_paired_udp_burst_id_key());
            std::sort(m_responsible_for_incoming_bursts.begin(), m_responsible_for_incoming_bursts.end(), ascending_paired_udp_burst_id_key());

            // Write algorithm performance
            WritePerformance();

            // Outgoing bursts
            std::cout << "  > Writing outgoing log files" << std::endl;
            for (std::pair<UdpBurstInfo, Ptr<UdpBurstApplication>> p : m_responsible_for_outgoing_bursts) {
                UdpBurstInfo info = p.first;
                Ptr<UdpBurstApplication> udpBurstAppOutgoing = p.second;
                
                // Fetch data from the application
                uint32_t complete_packet_size = 1500;
                uint32_t max_udp_payload_size_byte = udpBurstAppOutgoing->GetMaxUdpPayloadSizeByte();
                uint64_t sent_counter = udpBurstAppOutgoing->GetSendCounterOf(info.GetUdpBurstId());

                // Calculate outgoing rate
                int64_t effective_duration_ns = info.GetStartTimeNs() + info.GetDurationNs() >= m_simulation_end_time_ns ? m_simulation_end_time_ns - info.GetStartTimeNs() : info.GetDurationNs();
                double rate_incl_headers_megabit_per_s = byte_to_megabit(sent_counter * complete_packet_size) / nanosec_to_sec(effective_duration_ns);
                double rate_payload_only_megabit_per_s = byte_to_megabit(sent_counter * max_udp_payload_size_byte) / nanosec_to_sec(effective_duration_ns);

                // Write plain to the CSV
                fprintf(
                        file_outgoing_csv, "%" PRId64 ",%" PRId64 ",%" PRId64 ",%f,%" PRId64 ",%" PRId64 ",%f,%f,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%s\n",
                        info.GetUdpBurstId(), info.GetFromNodeId(), info.GetToNodeId(), info.GetTargetRateMegabitPerSec(), info.GetStartTimeNs(),
                        info.GetDurationNs(), rate_incl_headers_megabit_per_s, rate_payload_only_megabit_per_s, sent_counter,
                        sent_counter * complete_packet_size, sent_counter * max_udp_payload_size_byte, info.GetMetadata().c_str()
                );

                // Write nicely formatted to the text
                char str_target_rate[100];
                sprintf(str_target_rate, "%.2f Mbit/s", info.GetTargetRateMegabitPerSec());
                char str_start_time[100];
                sprintf(str_start_time, "%.2f ms", nanosec_to_millisec(info.GetStartTimeNs()));
                char str_duration_ms[100];
                sprintf(str_duration_ms, "%.2f ms", nanosec_to_millisec(info.GetDurationNs()));
                char str_eff_rate_incl_headers[100];
                sprintf(str_eff_rate_incl_headers, "%.2f Mbit/s", rate_incl_headers_megabit_per_s);
                char str_eff_rate_payload_only[100];
                sprintf(str_eff_rate_payload_only, "%.2f Mbit/s", rate_payload_only_megabit_per_s);
                char str_sent_incl_headers[100];
                sprintf(str_sent_incl_headers, "%.2f Mbit", byte_to_megabit(sent_counter * complete_packet_size));
                char str_sent_payload_only[100];
                sprintf(str_sent_payload_only, "%.2f Mbit", byte_to_megabit(sent_counter * max_udp_payload_size_byte));
                fprintf(
                        file_outgoing_txt,
                        "%-16" PRId64 "%-10" PRId64 "%-10" PRId64 "%-20s%-16s%-16s%-28s%-28s%-16" PRIu64 "%-28s%-28s%s\n",
                        info.GetUdpBurstId(),
                        info.GetFromNodeId(),
                        info.GetToNodeId(),
                        str_target_rate,
                        str_start_time,
                        str_duration_ms,
                        str_eff_rate_incl_headers,
                        str_eff_rate_payload_only,
                        sent_counter,
                        str_sent_incl_headers,
                        str_sent_payload_only,
                        info.GetMetadata().c_str()
                );
            }

            // Incoming bursts
            std::cout << "  > Writing incoming log files" << std::endl;
            for (std::pair<UdpBurstInfo, Ptr<UdpBurstApplication>> p : m_responsible_for_incoming_bursts) {
                UdpBurstInfo info = p.first;
                Ptr<UdpBurstApplication> udpBurstAppIncoming = p.second;

                // Fetch data from the application
                uint32_t complete_packet_size = 1500;
                uint32_t max_udp_payload_size_byte = udpBurstAppIncoming->GetMaxUdpPayloadSizeByte();
                uint64_t received_counter = udpBurstAppIncoming->GetReceivedCounterOf(info.GetUdpBurstId());

                // Calculate incoming rate
                int64_t effective_duration_ns = info.GetStartTimeNs() + info.GetDurationNs() >= m_simulation_end_time_ns ? m_simulation_end_time_ns - info.GetStartTimeNs() : info.GetDurationNs();
                double rate_incl_headers_megabit_per_s = byte_to_megabit(received_counter * complete_packet_size) / nanosec_to_sec(effective_duration_ns);
                double rate_payload_only_megabit_per_s = byte_to_megabit(received_counter * max_udp_payload_size_byte) / nanosec_to_sec(effective_duration_ns);

                // Write plain to the CSV
                fprintf(
                        file_incoming_csv, "%" PRId64 ",%" PRId64 ",%" PRId64 ",%f,%" PRId64 ",%" PRId64 ",%f,%f,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%s\n",
                        info.GetUdpBurstId(), info.GetFromNodeId(), info.GetToNodeId(), info.GetTargetRateMegabitPerSec(), info.GetStartTimeNs(),
                        info.GetDurationNs(), rate_incl_headers_megabit_per_s, rate_payload_only_megabit_per_s, received_counter,
                        received_counter * complete_packet_size, received_counter * max_udp_payload_size_byte, info.GetMetadata().c_str()
                );

                // Write nicely formatted to the text
                char str_target_rate[100];
                sprintf(str_target_rate, "%.2f Mbit/s", info.GetTargetRateMegabitPerSec());
                char str_start_time[100];
                sprintf(str_start_time, "%.2f ms", nanosec_to_millisec(info.GetStartTimeNs()));
                char str_duration_ms[100];
                sprintf(str_duration_ms, "%.2f ms", nanosec_to_millisec(info.GetDurationNs()));
                char str_eff_rate_incl_headers[100];
                sprintf(str_eff_rate_incl_headers, "%.2f Mbit/s", rate_incl_headers_megabit_per_s);
                char str_eff_rate_payload_only[100];
                sprintf(str_eff_rate_payload_only, "%.2f Mbit/s", rate_payload_only_megabit_per_s);
                char str_received_incl_headers[100];
                sprintf(str_received_incl_headers, "%.2f Mbit", byte_to_megabit(received_counter * complete_packet_size));
                char str_received_payload_only[100];
                sprintf(str_received_payload_only, "%.2f Mbit", byte_to_megabit(received_counter * max_udp_payload_size_byte));
                fprintf(
                        file_incoming_txt,
                        "%-16" PRId64 "%-10" PRId64 "%-10" PRId64 "%-20s%-16s%-16s%-28s%-28s%-19" PRIu64 "%-28s%-28s%s\n",
                        info.GetUdpBurstId(),
                        info.GetFromNodeId(),
                        info.GetToNodeId(),
                        str_target_rate,
                        str_start_time,
                        str_duration_ms,
                        str_eff_rate_incl_headers,
                        str_eff_rate_payload_only,
                        received_counter,
                        str_received_incl_headers,
                        str_received_payload_only,
                        info.GetMetadata().c_str()
                );

            }

            // Close files
            std::cout << "  > Closing UDP burst log files:" << std::endl;
            fclose(file_outgoing_csv);
            std::cout << "    >> Closed: " << m_udp_bursts_outgoing_csv_filename << std::endl;
            fclose(file_outgoing_txt);
            std::cout << "    >> Closed: " << m_udp_bursts_outgoing_txt_filename << std::endl;
            fclose(file_incoming_csv);
            std::cout << "    >> Closed: " << m_udp_bursts_incoming_csv_filename << std::endl;
            fclose(file_incoming_txt);
            std::cout << "    >> Closed: " << m_udp_bursts_incoming_txt_filename << std::endl;

            // Register completion
            std::cout << "  > UDP burst log files have been written" << std::endl;
            m_basicSimulation->RegisterTimestamp("Write UDP burst log files");

        }

        std::cout << std::endl;
    }

}
