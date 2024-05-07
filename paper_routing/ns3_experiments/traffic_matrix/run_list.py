# Author: silent-rookie     2024

# core
trafic_judge_rate_non_jam=0.9           # judge rate: non-jam area -> jam area
trafic_judge_rate_in_jam=0.8            # judge rate: if need detour in a jam area
trafic_judge_rate_jam_to_normal=0.5     # judge rate: jam area -> non-jam area
trafic_jam_update_interval_ns= 300 * 1000 * 1000 * 1000   # 2s

# update
dynamic_state_update_interval_ns = 1 * 1000 * 1000 * 1000   # 1s
dynamic_state_update_ms = dynamic_state_update_interval_ns / 1000 / 1000

# rate
isl_data_rate_megabit_per_s = 2.5
gsl_data_rate_megabit_per_s = 5
ill_data_rate_megabit_per_s = 25
isl_max_queue_size_kB = 20
gsl_max_queue_size_kB = 20
ill_max_queue_size_kB = 200

def get_run_list():
    run_list = []
    # for algorithm in ["DetourBasic"]:
    # for algorithm in ["DetourTrafficClassify"]:
    for algorithm in ["SingleForward", "DetourBasic", "DetourTrafficClassify"]:
        for config in [
            #   (durations, udp_burst_Mbps)

            # (200, 0.1),
            # (200, 0.15),
            # (200, 0.2),
            # (200, 0.25),
            # (200, 0.3),
            # (200, 0.35),
            # (200, 0.4),
            # (200, 0.45),
            # (200, 0.5),
            (200, 0.8),
            # (200, 1)
            #(5736, 0.2)
        ]:
            # Retrieve values from the config
            duration_s = config[0]
            udp_burst_Mbps = config[1]
            run_list.append(
                {
                    "name": "%s_judge-rate%.2f_update-state_%dms_%.3fMbps_for_%ds" % (algorithm, trafic_judge_rate_non_jam, dynamic_state_update_ms, udp_burst_Mbps, duration_s),
                    "algorithm": algorithm,
                    "dynamic_state_update_interval_ns": dynamic_state_update_interval_ns,
                    "dynamic_state_dir": "dynamic_state_%dms_for_%ds" % (dynamic_state_update_ms, duration_s),
                    "duration": duration_s,
                    "udp_burst_Mbps": udp_burst_Mbps,
                    "isl_Mbps": isl_data_rate_megabit_per_s,
                    "gsl_Mbps": gsl_data_rate_megabit_per_s,
                    "ill_Mbps": ill_data_rate_megabit_per_s,
                    "isl_queue_kB": isl_max_queue_size_kB,
                    "gsl_queue_kB": gsl_max_queue_size_kB,
                    "ill_queue_kB": ill_max_queue_size_kB,
                    "trafic_judge_rate_non_jam": trafic_judge_rate_non_jam,
                    "trafic_judge_rate_in_jam": trafic_judge_rate_in_jam,
                    "trafic_judge_rate_jam_to_normal": trafic_judge_rate_jam_to_normal,
                    "trafic_jam_update_interval_ns": trafic_jam_update_interval_ns,
                }
            )
    return run_list