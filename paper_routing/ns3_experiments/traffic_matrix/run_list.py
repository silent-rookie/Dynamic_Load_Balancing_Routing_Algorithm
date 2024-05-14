# Author: silent-rookie     2024

# core
trafic_judge_rate_non_jam=0.9           # judge rate: non-jam area -> jam area
trafic_judge_rate_in_jam=0.8            # judge rate: if need detour in a jam area
trafic_judge_rate_jam_to_normal=0.5     # judge rate: jam area -> non-jam area
trafic_jam_update_interval_ns= 300 * 1000 * 1000 * 1000   # 300s

# update
dynamic_state_update_interval_ns = 1 * 1000 * 1000 * 1000   # 1s
dynamic_state_update_ms = dynamic_state_update_interval_ns / 1000 / 1000

# rate
isl_data_rate_megabit_per_s = 10
gsl_data_rate_megabit_per_s = 20
ill_data_rate_megabit_per_s = 100
isl_max_queue_size_pkt = 100
gsl_max_queue_size_pkt = 100
ill_max_queue_size_pkt = 1000

# test information
test_info = ""

def get_run_list():
    run_list = []
    # for algorithm in ["SingleForward"]:
    # for algorithm in ["DetourTrafficClassify"]:
    for algorithm in ["SingleForward", "DetourTrafficClassify"]:
        for config in [
            #   (durations, udp_burst_Mbps)

            (200, 0.5),
            (200, 1),
            (200, 1.5),
            (200, 2),
            (200, 2.5),
            (200, 3),
            (200, 3.5),
            (200, 4),
            (200, 4.5),
            (200, 5),
            (200, 5.5),
            (200, 6),
            (200, 6.5),
            (200, 7)
            #(5736, 3)
        ]:
            # Retrieve values from the config
            duration_s = config[0]
            udp_burst_Mbps = config[1]

            name = "%s_judge-rate%.2f_update-state_%dms_%.3fMbps_for_%ds" % (
                                    algorithm, trafic_judge_rate_non_jam, dynamic_state_update_ms, udp_burst_Mbps, duration_s)
            if(test_info != ""):
                name = test_info + "_" + name

            run_list.append(
                {
                    "name": name,
                    "algorithm": algorithm,
                    "dynamic_state_update_interval_ns": dynamic_state_update_interval_ns,
                    "dynamic_state_dir": "dynamic_state_%dms_for_%ds" % (dynamic_state_update_ms, duration_s),
                    "duration": duration_s,
                    "udp_burst_Mbps": udp_burst_Mbps,
                    "isl_Mbps": isl_data_rate_megabit_per_s,
                    "gsl_Mbps": gsl_data_rate_megabit_per_s,
                    "ill_Mbps": ill_data_rate_megabit_per_s,
                    "isl_queue_pkt": isl_max_queue_size_pkt,
                    "gsl_queue_pkt": gsl_max_queue_size_pkt,
                    "ill_queue_pkt": ill_max_queue_size_pkt,
                    "trafic_judge_rate_non_jam": trafic_judge_rate_non_jam,
                    "trafic_judge_rate_in_jam": trafic_judge_rate_in_jam,
                    "trafic_judge_rate_jam_to_normal": trafic_judge_rate_jam_to_normal,
                    "trafic_jam_update_interval_ns": trafic_jam_update_interval_ns,
                }
            )
    return run_list