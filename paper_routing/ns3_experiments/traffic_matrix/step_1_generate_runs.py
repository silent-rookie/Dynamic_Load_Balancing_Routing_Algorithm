# Author: silent-rookie     2024

import exputil
import random
from generate_from_to_random_pair import generate_from_to_random_pair_top100_city, generate_pairs_logging_ids

dynamic_state_update_interval_ns = 1000000000   # 1s
pairs_num = 100

local_shell = exputil.LocalShell()

local_shell.remove_force_recursive("runs")
local_shell.remove_force_recursive("pdf")
local_shell.remove_force_recursive("data")

for config in [
    # (ISL Rate in Mbit/s, GSL Rate in Mbit/s, ILL Rate in Mbit/s, 
    #   isl_max_queue_size_pkts, gsl_max_queue_size_pkts, ill_max_queue_size_pkts,
    #   durations, udp_burst_Mbps)

    (2.5, 5, 25, 20000, 20000, 200000, 200, 0.1)
    #(2.5, 5, 25, 20000, 20000, 200000, 5736, 100)
]:

    # Retrieve values from the config
    isl_data_rate_megabit_per_s = config[0]
    gsl_data_rate_megabit_per_s = config[1]
    ill_data_rate_megabit_per_s = config[2]
    isl_max_queue_size_pkts = config[3]
    gsl_max_queue_size_pkts = config[4]
    ill_max_queue_size_pkts = config[5]
    duration_s = config[6]
    udp_burst_Mbps = config[7]

    # unsupport tcp now
    for protocol_chosen in ["udp"]:

        # Prepare run directory
        run_dir = "runs/run_loaded_tm_pairing_%fMbps_for_%ds_with_%s" % (
                udp_burst_Mbps, duration_s, protocol_chosen
        )
        local_shell.remove_force_recursive(run_dir)
        local_shell.make_full_dir(run_dir)

        # config_ns3.properties
        local_shell.copy_file(
            "templates/template_config_ns3_" + protocol_chosen + ".properties",
            run_dir + "/config_ns3.properties"
        )
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[SIMULATION-END-TIME-NS]", str(duration_s * 1000 * 1000 * 1000))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[DYNAMIC-STATE-UPDATE-INTERVAL-NS]", str(dynamic_state_update_interval_ns))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[ISL-DATA-RATE-MEGABIT-PER-S]", str(isl_data_rate_megabit_per_s))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[GSL-DATA-RATE-MEGABIT-PER-S]", str(gsl_data_rate_megabit_per_s))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[ILL-DATA-RATE-MEGABIT-PER-S]", str(ill_data_rate_megabit_per_s))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[ISL-MAX-QUEUE-SIZE-PKTS]", str(isl_max_queue_size_pkts))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[GSL-MAX-QUEUE-SIZE-PKTS]", str(gsl_max_queue_size_pkts))
        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[ILL-MAX-QUEUE-SIZE-PKTS]", str(ill_max_queue_size_pkts))

        # Make logs_ns3 already for console.txt mapping
        local_shell.make_full_dir(run_dir + "/logs_ns3")

        # .gitignore (legacy reasons)
        local_shell.write_file(run_dir + "/.gitignore", "logs_ns3")

        # Schedule
        random.seed(123456789)
        seed_from_to = random.randint(0, 100000000) # Legacy reasons
        pairs = generate_from_to_random_pair_top100_city(pairs_num, seed_from_to)

        local_shell.sed_replace_in_file_plain(run_dir + "/config_ns3.properties",
                                              "[UDP_LOGGING_BURST_IDS]", generate_pairs_logging_ids(pairs))

        # udp_burst_schedule.csv
        if protocol_chosen == "udp":
            with open(run_dir + "/udp_burst_schedule.csv", "w+") as f_out:
                for i in range(len(pairs)):
                    f_out.write(
                        "%d,%d,%d,%.10f,%d,%d,,\n" % (
                            i,
                            pairs[i][0],
                            pairs[i][1],
                            udp_burst_Mbps,
                            0,
                            duration_s * 1000 * 1000 * 1000
                        )
                    )
        # tcp_flow_schedule.csv
        # Todo
        # elif protocol_chosen == "tcp":
        #     networkload.write_schedule(
        #         run_dir + "/tcp_flow_schedule.csv",
        #         len(pairs),
        #         pairs,
        #         [1000000000000] * len(pairs),
        #         [0] * len(pairs)
        #     )

print("Success: generated runs")

