def generate_simple_ill_interfaces_info(filename_ill_interfaces_info, 
                                        number_of_satellites, number_of_ground_stations, number_of_GEOsatellites,
                                        num_ill_interfaces_per_satellite, num_ill_interfaces_per_GEOsatellite,
                                        agg_max_bandwidth_satellite, agg_max_bandwidth_GEOsatellite):
    """
    Read for each node the ILL interface information.

    Note: the unit of the aggregate max bandwidth per satellite is not known, but they both must be the same unit.
    Note: the node_id of GEOsatellite is after satellite and ground station

    :param filename_ill_interfaces_info: Filename of ILL interfaces info file to write to
                                         (typically /path/to/ill_interfaces_info.txt)
                                         Line format: <node id>,<number of interfaces>,<aggregate max. bandwidth Mbit/s>
    :param number_of_satellites:                    Number of satellites
    :param number_of_ground_stations:               Number of ground stations
    :param number_of_GEOsatellites:                 Number of GEOsatellites
    :param num_ill_interfaces_per_satellite:        Number of ILL interfaces per satellite
    :param num_ill_interfaces_per_GEOsatellite:     Number of ILL interfaces per GEOsatellite
    :param agg_max_bandwidth_satellite:             Aggregate bandwidth of all interfaces on a satellite
    :param agg_max_bandwidth_GEOsatellite:          Aggregate bandwidth of all interfaces on a GEOsatellite

    """
    with open(filename_ill_interfaces_info, 'w+') as f:
        for node_id in range(number_of_satellites):
            f.write("%d,%d,%f\n" % (node_id, num_ill_interfaces_per_satellite, agg_max_bandwidth_satellite))
        for node_id in range(number_of_GEOsatellites):
            f.write("%d,%d,%f\n" % (node_id + number_of_satellites + number_of_ground_stations, 
                                    num_ill_interfaces_per_GEOsatellite, agg_max_bandwidth_GEOsatellite))