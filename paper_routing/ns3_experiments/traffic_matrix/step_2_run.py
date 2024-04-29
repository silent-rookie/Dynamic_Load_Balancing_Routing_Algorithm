# Author: silent-rookie     2024

import exputil
import time
local_shell = exputil.LocalShell()

# Get workload identifier from argument
max_num_thread = 8

# Clear old runs
local_shell.perfect_exec("rm -rf runs/*/logs_ns3")

# Check that no screen is running
if local_shell.count_screens() != 0:
    print("There is a screen already running. "
          "Please kill all screens before running this analysis script (killall screen).")
    exit(1)

# Generate the commands
commands_to_run = []


for config in [
    # (ISL Rate in Mbit/s, GSL Rate in Mbit/s, ILL Rate in Mbit/s, 
    #   isl_max_queue_size_pkts, gsl_max_queue_size_pkts, ill_max_queue_size_pkts,
    #   durations, udp_burst_Kbps)

    (2.5, 5, 25, 20000, 20000, 200000, 200, 100)
    #(2.5, 5, 25, 20000, 20000, 200000, 5736, 100)
]:

    # Retrieve values from the config
    duration_s = config[6]
    udp_burst_Kbps = config[7]

    # unsupport tcp now
    for protocol_chosen in ["udp"]:

        # Prepare run directory
        run_dir = "runs/run_loaded_tm_pairing_%d_Kbps_for_%ds_with_%s" % (
            udp_burst_Kbps, duration_s, protocol_chosen
        )
        logs_ns3_dir = run_dir + "/logs_ns3"
        local_shell.remove_force_recursive(logs_ns3_dir)
        local_shell.make_full_dir(logs_ns3_dir)

        # Perform run
        commands_to_run.append(
            "cd ../../../ns3-sat-sim/simulator; "
            "./waf --run=\"main_satnet "
            "--run_dir='../../paper_routing/ns3_experiments/traffic_matrix/" + run_dir + "'\""
            " 2>&1 | tee '../../paper_routing/ns3_experiments/traffic_matrix/" + logs_ns3_dir + "/console.txt'",
        )


# Run the commands
print("Running commands (at most %d in parallel)..." % max_num_thread)
for i in range(len(commands_to_run)):
    print("Starting command %d out of %d: %s" % (i + 1, len(commands_to_run), commands_to_run[i]))
    local_shell.detached_exec(commands_to_run[i])
    while local_shell.count_screens() >= max_num_thread:
        time.sleep(2)

# Awaiting final completion before exiting
print("Waiting completion of the last %d..." % max_num_thread)
while local_shell.count_screens() > 0:
    time.sleep(2)
print("Finished.")