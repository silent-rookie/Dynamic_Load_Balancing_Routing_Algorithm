import math
import networkx as nx


def calculate_fstate_shortest_path_without_gs_relaying(
        output_dynamic_state_dir,
        time_since_epoch_ns,
        num_satellites,
        num_ground_stations,
        sat_net_graph_only_satellites_with_isls,
        num_isls_per_sat,
        gid_to_sat_gsl_if_idx,
        ground_station_satellites_in_range_candidates,
        sat_neighbor_to_if,
        prev_fstate,
        enable_verbose_logs
):

    # Calculate shortest path distances
    if enable_verbose_logs:
        print("  > Calculating Floyd-Warshall for graph without ground-station relays")
    # (Note: Numpy has a deprecation warning here because of how networkx uses matrices)
    dist_sat_net_without_gs = nx.floyd_warshall_numpy(sat_net_graph_only_satellites_with_isls)

    # Forwarding state
    fstate = {}

    # Now write state to file for complete graph
    output_filename = output_dynamic_state_dir + "/fstate/fstate_" + str(time_since_epoch_ns) + ".txt"
    if enable_verbose_logs:
        print("  > Writing forwarding state to: " + output_filename)
    with open(output_filename, "w+") as f_out:

        # Satellites to ground stations
        # From the satellites attached to the destination ground station,
        # select the one which promises the shortest path to the destination ground station (getting there + last hop)
        dist_satellite_to_ground_station = {}
        for curr in range(num_satellites):
            for dst_gid in range(num_ground_stations):
                dst_gs_node_id = num_satellites + dst_gid

                # Among the satellites in range of the destination ground station,
                # find the one which promises the shortest distance
                possible_dst_sats = ground_station_satellites_in_range_candidates[dst_gid]
                possibilities = []
                for b in possible_dst_sats:
                    if not math.isinf(dist_sat_net_without_gs[(curr, b[1])]):  # Must be reachable
                        possibilities.append(
                            (
                                dist_sat_net_without_gs[(curr, b[1])] + b[0],
                                b[1]
                            )
                        )
                possibilities = list(sorted(possibilities))

                # By default, if there is no satellite in range for the
                # destination ground station, it will be dropped (indicated by -1)
                # pick 3 neighbors are add to candidate lists for detour
                next_hop_lists = [[-1, -1, -1], [-1, -1, -1], [-1, -1, -1]]
                distance_to_ground_station_m = float("inf")
                if len(possibilities) > 0:
                    dst_sat = possibilities[0][1]
                    distance_to_ground_station_m = possibilities[0][0]

                    # If the current node is not that satellite, determine how to get to the satellite
                    if curr != dst_sat:

                        # pick 3 neighbors are add to candidate lists for detour
                        next_hop_candidates = []
                        for neighbor_id in sat_net_graph_only_satellites_with_isls.neighbors(curr):
                            distance_m = (
                                    sat_net_graph_only_satellites_with_isls.edges[(curr, neighbor_id)]["weight"]
                                    +
                                    dist_sat_net_without_gs[(neighbor_id, dst_sat)]
                            )
                            next_hop_candidates.append((distance_m, 
                                                        neighbor_id, 
                                                        sat_neighbor_to_if[(curr, neighbor_id)], 
                                                        sat_neighbor_to_if[(neighbor_id, curr)]))
                        next_hop_candidates.sort()
                        for i in range(len(next_hop_candidates)):
                            if i >= 3:
                                break
                            next_hop_lists[i] = [next_hop_candidates[i][1], next_hop_candidates[i][2], next_hop_candidates[i][3]]

                    else:
                        # This is the destination satellite, as such the next hop is the ground station itself
                        next_hop_lists[0] = [dst_gs_node_id, num_isls_per_sat[dst_sat] + gid_to_sat_gsl_if_idx[dst_gid], 0]

                # In any case, save the distance of the satellite to the ground station to re-use
                # when we calculate ground station to ground station forwarding
                dist_satellite_to_ground_station[(curr, dst_gs_node_id)] = distance_to_ground_station_m

                # because the sat_neighbor_to_if is never change
                # we just use sat node id for prev_fstate
                prev_next_node = [next_hop_lists[0][0], next_hop_lists[1][0], next_hop_lists[2][0]]

                # Write to forwarding state
                if not prev_fstate or prev_fstate[(curr, dst_gs_node_id)] != prev_next_node:
                    f_out.write("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n" % (
                        curr, dst_gs_node_id,
                        next_hop_lists[0][0], next_hop_lists[0][1], next_hop_lists[0][2],
                        next_hop_lists[1][0], next_hop_lists[1][1], next_hop_lists[1][2],
                        next_hop_lists[2][0], next_hop_lists[2][1], next_hop_lists[2][2]
                    ))
                fstate[(curr, dst_gs_node_id)] = prev_next_node

        # Ground stations to ground stations
        # Choose the source satellite which promises the shortest path
        for src_gid in range(num_ground_stations):
            for dst_gid in range(num_ground_stations):
                if src_gid != dst_gid:
                    src_gs_node_id = num_satellites + src_gid
                    dst_gs_node_id = num_satellites + dst_gid

                    # Among the satellites in range of the source ground station,
                    # find the one which promises the shortest distance
                    possible_src_sats = ground_station_satellites_in_range_candidates[src_gid]
                    possibilities = []
                    for a in possible_src_sats:
                        best_distance_offered_m = dist_satellite_to_ground_station[(a[1], dst_gs_node_id)]
                        if not math.isinf(best_distance_offered_m):
                            possibilities.append(
                                (
                                    a[0] + best_distance_offered_m,
                                    a[1]
                                )
                            )
                    possibilities = sorted(possibilities)

                    # By default, if there is no satellite in range for one of the
                    # ground stations, it will be dropped (indicated by -1)
                    next_hop_lists = [[-1, -1, -1], [-1, -1, -1], [-1, -1, -1]]
                    if len(possibilities) > 0:
                        for i in range(len(possibilities)):
                            # we just need 3 candidates
                            if(i >= 3):
                                break

                            src_sat_id = possibilities[i][1]
                            next_hop_lists[i] = [src_sat_id, 0, num_isls_per_sat[src_sat_id] + gid_to_sat_gsl_if_idx[src_gid]]

                    prev_next_node = [next_hop_lists[0][0], next_hop_lists[1][0], next_hop_lists[2][0]]

                    # Update forwarding state
                    if not prev_fstate or prev_fstate[(src_gs_node_id, dst_gs_node_id)] != prev_next_node:
                        f_out.write("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n" % (
                            src_gs_node_id, dst_gs_node_id,
                            next_hop_lists[0][0], next_hop_lists[0][1], next_hop_lists[0][2],
                            next_hop_lists[1][0], next_hop_lists[1][1], next_hop_lists[1][2],
                            next_hop_lists[2][0], next_hop_lists[2][1], next_hop_lists[2][2]
                        ))
                    fstate[(src_gs_node_id, dst_gs_node_id)] = prev_next_node

    # Finally return result
    return fstate
