import sys
sys.path.append("../../../../satgenpy")
import satgen
from .generate_dynamic_state import generate_dynamic_state
import os
import math
from multiprocessing.dummy import Pool as ThreadPool


def worker(args):

    # Extract arguments
    (
        output_dynamic_state_dir,
        epoch,
        simulation_end_time_ns,
        time_step_ns,
        offset_ns,
        satellites,
        GEOsatellites,
        ground_stations,
        list_isls,
        list_gsl_interfaces_info,
        max_gsl_length_m,
        max_isl_length_m,
        max_ill_length_m,
        dynamic_state_algorithm,
        print_logs
     ) = args

    # Generate dynamic state
    generate_dynamic_state(
        output_dynamic_state_dir,
        epoch,
        simulation_end_time_ns,
        time_step_ns,
        offset_ns,
        satellites,
        GEOsatellites,
        ground_stations,
        list_isls,
        list_gsl_interfaces_info,
        max_gsl_length_m,
        max_isl_length_m,
        max_ill_length_m,
        dynamic_state_algorithm,  # Options:
                                  # "algorithm_free_one_only_gs_relays"
                                  # "algorithm_free_one_only_over_isls"
                                  # "algorithm_free_gs_one_sat_many_only_over_isls"
                                  # "algorithm_paired_many_only_over_isls"
        print_logs
    )


def help_dynamic_state(
        output_generated_data_dir, 
        num_threads, 
        name, 
        time_step_ms, 
        duration_s,
        max_gsl_length_m, 
        max_isl_length_m, 
        max_ill_length_m,
        dynamic_state_algorithm, 
        print_logs
):

    # Directory
    output_dynamic_state_dir = output_generated_data_dir + "/" + name + "/dynamic_state_" + str(time_step_ms) \
                               + "ms_for_" + str(duration_s) + "s"
    if not os.path.isdir(output_dynamic_state_dir):
        os.makedirs(output_dynamic_state_dir)
    output_ills_dir = output_dynamic_state_dir + "/ills"
    if not os.path.isdir(output_ills_dir):
        os.makedirs(output_ills_dir)
    output_fstate_dir = output_dynamic_state_dir + "/fstate"
    if not os.path.isdir(output_fstate_dir):
        os.makedirs(output_fstate_dir)

    # In nanoseconds
    simulation_end_time_ns = duration_s * 1000 * 1000 * 1000
    time_step_ns = time_step_ms * 1000 * 1000

    num_calculations = math.floor(simulation_end_time_ns / time_step_ns)
    calculations_per_thread = int(math.floor(float(num_calculations) / float(num_threads)))
    num_threads_with_one_more = num_calculations % num_threads

    # Prepare arguments
    current = 0
    list_args = []
    for i in range(num_threads):

        # How many time steps to calculate for
        num_time_steps = calculations_per_thread
        if i < num_threads_with_one_more:
            num_time_steps += 1

        # Variables (load in for each thread such that they don't interfere)
        ground_stations = satgen.read_ground_stations_extended(output_generated_data_dir + "/" + name + "/ground_stations.txt")
        tles = satgen.read_tles(output_generated_data_dir + "/" + name + "/tles.txt")
        satellites = tles["satellites"]
        GEOtles = satgen.read_tles(output_generated_data_dir + "/" + name + "/tles_GEO.txt")
        GEOsatellites = GEOtles["satellites"]
        list_isls = satgen.read_isls(output_generated_data_dir + "/" + name + "/isls.txt", len(satellites))
        list_gsl_interfaces_info = satgen.read_gsl_interfaces_info(
            output_generated_data_dir + "/" + name + "/gsl_interfaces_info.txt",
            len(satellites),
            len(ground_stations)
        )
        epoch = tles["epoch"]

        # Print goal
        print("Thread %d does interval [%.2f ms, %.2f ms]" % (
            i,
            (current * time_step_ns) / 1e6,
            ((current + num_time_steps) * time_step_ns) / 1e6
        ))

        list_args.append((
            output_dynamic_state_dir,
            epoch,
            (current + num_time_steps) * time_step_ns + (time_step_ns if (i + 1) != num_threads else 0),
            time_step_ns,
            current * time_step_ns,
            satellites,
            GEOsatellites,
            ground_stations,
            list_isls,
            list_gsl_interfaces_info,
            max_gsl_length_m,
            max_isl_length_m,
            max_ill_length_m,
            dynamic_state_algorithm,
            print_logs
        ))

        current += num_time_steps

    # Run in parallel
    pool = ThreadPool(num_threads)
    pool.map(worker, list_args)
    pool.close()
    pool.join()
