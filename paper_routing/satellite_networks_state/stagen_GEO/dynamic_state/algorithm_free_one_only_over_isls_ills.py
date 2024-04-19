from ills_calculation import calculate_ills
from fstate_calculation import calculate_fstate_shortest_path_without_gs_relaying



def algorithm_free_one_only_over_isls_ills(
        output_dynamic_state_dir,
        epoch,
        time_since_epoch_ns,
        satellites,
        GEOsatellites,
        ground_stations,
        sat_net_graph_only_satellites_with_isls,
        ground_station_satellites_in_range,
        num_isls_per_sat,
        sat_neighbor_to_if,
        list_gsl_interfaces_info,
        prev_output,
        enable_verbose_logs
):
    """
    FREE-ONE ONLY OVER INTER-SATELLITE LINKS AND INTER-LAYER LINKS  ALGORITHM

    "one"
    This algorithm assumes that every satellite and ground station has exactly 1 GSL interface.
    This algorithm assumes that every satellite and GEOsatellite station has exactly 1 ILL interface. 

    "free"
    This 1 interface is bound to a maximum outgoing bandwidth, but can send to any other
    GSL interface (well, satellite -> ground-station, and ground-station -> satellite) in
    range. ("free") There is no reciprocation of the bandwidth asserted.
    ILL is also same to GSL.

    "only_over_isls"
    It calculates a forwarding state, which is essentially a single shortest path.
    It only considers paths which go over the inter-satellite network, and does not make use of ground
    stations relay. This means that every path looks like:
    (src gs) - (sat) - (sat) - ... - (sat) - (dst gs)

    """

    if enable_verbose_logs:
        print("\nALGORITHM: FREE ONE ONLY OVER ISLS")

    # Check the graph
    if sat_net_graph_only_satellites_with_isls.number_of_nodes() != len(satellites):
        raise ValueError("Number of nodes in the graph does not match the number of satellites")
    for sid in range(len(satellites)):
        for n in sat_net_graph_only_satellites_with_isls.neighbors(sid):
            if n >= len(satellites):
                raise ValueError("Graph cannot contain satellite-to-ground-station links")

    #################################
    # BANDWIDTH STATE
    #

    # There is only one GSL interface for each node (pre-condition), which as-such will get the entire bandwidth
    output_filename = output_dynamic_state_dir + "/gsl_if_bandwidth_" + str(time_since_epoch_ns) + ".txt"
    if enable_verbose_logs:
        print("  > Writing interface bandwidth state to: " + output_filename)
    with open(output_filename, "w+") as f_out:
        if time_since_epoch_ns == 0:
            for node_id in range(len(satellites)):
                f_out.write("%d,%d,%f\n"
                            % (node_id, num_isls_per_sat[node_id],
                               list_gsl_interfaces_info[node_id]["aggregate_max_bandwidth"]))
            for node_id in range(len(satellites), len(satellites) + len(ground_stations)):
                f_out.write("%d,%d,%f\n"
                            % (node_id, 0, list_gsl_interfaces_info[node_id]["aggregate_max_bandwidth"]))

    #################################
    # ILLS STATE
    # 

    # Previous forwarding state (to only write delta)
    prev_ills = None
    if prev_output is not None:
        prev_ills = prev_output["ills"]

    ills = calculate_ills(
        output_dynamic_state_dir,
        epoch,
        time_since_epoch_ns,
        satellites,
        GEOsatellites,
        prev_ills,
        enable_verbose_logs
    )

    #################################
    # FORWARDING STATE
    #

    # Previous forwarding state (to only write delta)
    prev_fstate = None
    if prev_output is not None:
        prev_fstate = prev_output["fstate"]

    # GID to satellite GSL interface index
    gid_to_sat_gsl_if_idx = [0] * len(ground_stations)  # (Only one GSL interface per satellite, so the first)

    # Forwarding state using shortest paths
    fstate = calculate_fstate_shortest_path_without_gs_relaying(
        output_dynamic_state_dir,
        time_since_epoch_ns,
        len(satellites),
        len(ground_stations),
        sat_net_graph_only_satellites_with_isls,
        num_isls_per_sat,
        gid_to_sat_gsl_if_idx,
        ground_station_satellites_in_range,
        sat_neighbor_to_if,
        prev_fstate,
        enable_verbose_logs
    )

    #################################

    if enable_verbose_logs:
        print("")

    return {
        "fstate": fstate,
        "ills":   ills
    }
