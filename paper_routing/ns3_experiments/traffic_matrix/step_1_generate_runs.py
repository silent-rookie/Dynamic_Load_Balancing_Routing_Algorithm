# Author: silent-rookie     2024

import exputil
import random
from generate_from_to_random_pair import generate_from_to_random_pair_top100_city, generate_pairs_logging_ids
from run_list import get_run_list

pairs_num = 100
random_num = 100

local_shell = exputil.LocalShell()

# local_shell.remove_force_recursive("runs")
# local_shell.remove_force_recursive("pdf")
# local_shell.remove_force_recursive("data")

for run in get_run_list():

    # Prepare run directory
    run_dir = "runs/" + run["name"]
    local_shell.remove_force_recursive(run_dir)
    local_shell.make_full_dir(run_dir)

    config = run_dir + "/config_ns3.properties"
    # config_ns3.properties
    local_shell.copy_file("templates/template_config_ns3_udp.properties", config)
    local_shell.sed_replace_in_file_plain(config, "[SIMULATION-END-TIME-NS]", str(run["duration"] * 1000 * 1000 * 1000))
    local_shell.sed_replace_in_file_plain(config, "[DYNAMIC-STATE-DIR]", run["dynamic_state_dir"])
    local_shell.sed_replace_in_file_plain(config, "[DYNAMIC-STATE-UPDATE-INTERVAL-NS]", str(run["dynamic_state_update_interval_ns"]))
    local_shell.sed_replace_in_file_plain(config, "[ISL-DATA-RATE-MEGABIT-PER-S]", str(run["isl_Mbps"]))
    local_shell.sed_replace_in_file_plain(config, "[GSL-DATA-RATE-MEGABIT-PER-S]", str(run["gsl_Mbps"]))
    local_shell.sed_replace_in_file_plain(config, "[ILL-DATA-RATE-MEGABIT-PER-S]", str(run["ill_Mbps"]))
    local_shell.sed_replace_in_file_plain(config, "[ISL-MAX-QUEUE-SIZE-PKT]", str(run["isl_queue_pkt"]))
    local_shell.sed_replace_in_file_plain(config, "[GSL-MAX-QUEUE-SIZE-PKT]", str(run["gsl_queue_pkt"]))
    local_shell.sed_replace_in_file_plain(config, "[ILL-MAX-QUEUE-SIZE-PKT]", str(run["ill_queue_pkt"]))
    local_shell.sed_replace_in_file_plain(config, "[TRAFIC-JUDGE-RATE-NON-JAM]", str(run["trafic_judge_rate_non_jam"]))
    local_shell.sed_replace_in_file_plain(config, "[TRAFIC-JUDGE-RATE-IN-JAM]", str(run["trafic_judge_rate_in_jam"]))
    local_shell.sed_replace_in_file_plain(config, "[TRAFIC-JUDGE-RATE-JAM-TO-NORMAL]", str(run["trafic_judge_rate_jam_to_normal"]))
    local_shell.sed_replace_in_file_plain(config, "[TRAFIC-JAM-UPDATE-INTERVAL-NS]", str(run["trafic_jam_update_interval_ns"]))
    local_shell.sed_replace_in_file_plain(config, "[ALGORITHM_CHOOSE]", run["algorithm"])

    # Make logs_ns3 already for console.txt mapping
    local_shell.make_full_dir(run_dir + "/logs_ns3")

    # .gitignore (legacy reasons)
    local_shell.write_file(run_dir + "/.gitignore", "logs_ns3")

    # Schedule
    random.seed(123456789)
    seed_from_to = random.randint(0, 100000000) # Legacy reasons
    pairs = generate_from_to_random_pair_top100_city(pairs_num, random_num, seed_from_to)

    # local_shell.sed_replace_in_file_plain(config, "[UDP_LOGGING_BURST_IDS]", generate_pairs_logging_ids(pairs))
    local_shell.sed_replace_in_file_plain(config, "[UDP_LOGGING_BURST_IDS]", "set()")

    # udp_burst_schedule.csv
    with open(run_dir + "/udp_burst_schedule.csv", "w+") as f_out:
        for i in range(len(pairs)):
            f_out.write(
                "%d,%d,%d,%.10f,%d,%d,,\n" % (
                    i,
                    pairs[i][0],
                    pairs[i][1],
                    run["udp_burst_Mbps"],
                    0,
                    run["duration"] * 1000 * 1000 * 1000
                )
            )



print("Success: generated runs")

