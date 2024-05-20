# Author: silent-rookie     2024

import ephem



def generate_sat_obj_list(
        num_orbit,
        num_sats_per_orbit,
        epoch,
        phase_diff,
        inclination,
        eccentricity,
        arg_perigee,
        mean_motion,
        altitude
):
    """
    Generates list of satellite objects based on orbital elements
    :param num_orbit: Number of orbits
    :param num_sats_per_orbit: Number of satellites per orbit
    :param epoch: Epoch (start time)
    :param phase_diff: Phase difference between adjacent orbits
    :param inclination: Angle of inclination
    :param eccentricity: Eccentricity of orbits
    :param arg_perigee: Argument of perigee of orbits
    :param mean_motion: Mean motion in revolutions per day
    :param altitude: Altitude in metres
    :return: List of satellite objects
    """
    sat_objs = [None] * (num_orbit * num_sats_per_orbit)
    counter = 0
    for orb in range(0, num_orbit):
        raan = orb * 360 / num_orbit
        orbit_wise_shift = 0
        if orb % 2 == 1:
            if phase_diff:
                orbit_wise_shift = 360 / (num_sats_per_orbit * 2)

        for n_sat in range(0, num_sats_per_orbit):
            mean_anomaly = orbit_wise_shift + (n_sat * 360 / num_sats_per_orbit)

            sat = ephem.EarthSatellite()
            sat._epoch = epoch
            sat._inc = ephem.degrees(inclination)
            sat._e = eccentricity
            sat._raan = ephem.degrees(raan)
            sat._ap = arg_perigee
            sat._M = ephem.degrees(mean_anomaly)
            sat._n = mean_motion

            sat_objs[counter] = {
                "sat_obj": sat,
                "alt_km": altitude / 1000,
                "orb_id": orb,
                "orb_sat_id": n_sat

            }
            counter += 1
    return sat_objs

def generate_orbit_links(sat_objs, num_orbit, num_sats_per_orbit):
    """
    Orbit is visualized by connecting consecutive satellites within the orbit.
    This function returns such satellite-satellite connections
    :param sat_objs: List of satellite objects
    :param num_orbit: Number of orbits
    :param num_sats_per_orbit: Number of satellites per orbit
    :return: Components of orbit
    """
    orbit_links = {}
    cntr = 0
    for i in range(0, len(sat_objs)):
        sat_next = i + 1
        if sat_next % num_sats_per_orbit == 0:
            sat_next -= num_sats_per_orbit
        
        orbit_links[cntr] = {
            "sat1": i,
            "sat2": sat_next
        }
        cntr += 1
    return orbit_links

def generate_isls_links(sat_objs, num_orbit, num_sats_per_orbit):
    """
    Generates +Grid(4 ISLs) connectivity between satellites
    :param sat_objs: List of satellite objects
    :param num_orbit: Number of orbits
    :param num_sats_per_orbit: Number of satellites per orbit
    :return: +Grid links
    """
    if num_orbit < 3 or num_sats_per_orbit < 3:
        raise ValueError("num_orbit and num_sats_per_orbit must each be at least 3")

    grid_links = {}
    cntr = 0
    for i in range(num_orbit):
        for j in range(num_sats_per_orbit):
            sat = i * num_sats_per_orbit + j

            # Link to the next in the orbit
            sat_same_orbit = i * num_sats_per_orbit + ((j + 1) % num_sats_per_orbit)
            sat_adjacent_orbit = ((i + 1) % num_orbit) * num_sats_per_orbit + j

            # Same orbit
            grid_links[cntr] = {
                "sat1": sat,
                "sat2": sat_same_orbit
            }
            # Adjacent orbit
            grid_links[cntr] = {
                "sat1": sat,
                "sat2": sat_adjacent_orbit
            }
            cntr += 1

    return grid_links

def write_viz_files(viz_string, top_file, bottom_file, out_file):
    """
    Generates HTML visualization file
    :param viz_string: HTML formatted string
    :param top_file: top part of the HTML file
    :param bottom_file: bottom part of the HTML file
    :param out_file: output HTML file
    :return: None
    """
    writer_html = open(out_file, 'w')
    with open(top_file, 'r') as fi:
        writer_html.write(fi.read())
    writer_html.write(viz_string)
    with open(bottom_file, 'r') as fb:
        writer_html.write(fb.read())
    writer_html.close()