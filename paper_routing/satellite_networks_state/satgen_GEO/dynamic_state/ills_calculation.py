import sys
sys.path.append("../../../../satgenpy")
import satgen
from astropy import units as u
import numpy as np



def calculate_ills(
        output_dynamic_state_dir,
        epoch,
        time_since_epoch_ns,
        satellites,
        GEOsatellites,
        max_ill_length_m,
        prev_ills,
        enable_verbose_logs
):
    """
    
    we assume that every satellite have only one ILL, and the ILL interface is the last
    interface of satellite

    """

    # ills state
    ills = {}


    output_ill_filename = output_dynamic_state_dir + "/ills/ills_" + str(time_since_epoch_ns) + ".txt"
    with open(output_ill_filename, 'w+') as f_out:
        if enable_verbose_logs:
            print("\nILL INFORMATIOIN")

        time = epoch + time_since_epoch_ns * u.ns

        list_ills = []
        total_num_ills = 0
        changed_ills = 0
        num_ills_per_geo = [0] * len(GEOsatellites)
        for sat in range(len(satellites)):
            min_ill_distance_m = max_ill_length_m
            target_geo = -1
            for geo in range(len(GEOsatellites)):
                # find the geo have the min distance of ill
                ill_distance_m = satgen.distance_m_between_satellites(
                        satellites[sat], GEOsatellites[geo], str(epoch), str(time))
                if ill_distance_m < min_ill_distance_m:
                    min_ill_distance_m = ill_distance_m
                    target_geo = geo

            list_ills.append(target_geo)
            if target_geo != -1:
                num_ills_per_geo[target_geo] += 1   # the link is "geo-sat"
                total_num_ills += 1
            else:
                raise ValueError("No suitable ILL be found, something wrong")

            if not prev_ills or prev_ills[sat] != target_geo:
                changed_ills += 1
                f_out.write(str(sat) + " " + str(target_geo) + "\n")
            ills[sat] = target_geo

        if enable_verbose_logs:
            print("  > Total ILLs................ " + str(total_num_ills))
            print("  > Changed ILLs.............. " + str(changed_ills))
            print("  > Min. ILLs/GEOsatellite.... " + str(np.min(num_ills_per_geo)))
            print("  > Max. ILLs/GEOsatellite.... " + str(np.max(num_ills_per_geo)))
    
    return ills