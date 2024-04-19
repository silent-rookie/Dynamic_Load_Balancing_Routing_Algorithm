import sys
import math
from main_helper import MainHelper

# WGS72 value; taken from https://geographiclib.sourceforge.io/html/NET/NETGeographicLib_8h_source.html
EARTH_RADIUS = 6378135.0

# GENERATION CONSTANTS

BASE_NAME = "starlink_550"
NICE_NAME = "Starlink-550"

# STARLINK 550

ECCENTRICITY = 0.0000001  # Circular orbits are zero, but pyephem does not permit 0, so lowest possible value
ARG_OF_PERIGEE_DEGREE = 0.0
PHASE_DIFF = True

################################################################
# The below constants are taken from Starlink's FCC filing as below:
# [1]: https://fcc.report/IBFS/SAT-MOD-20190830-00087
################################################################

MEAN_MOTION_REV_PER_DAY = 15.19  # Altitude ~550 km
ALTITUDE_M = 550000  # Altitude ~550 km

# From https://fcc.report/IBFS/SAT-MOD-20181108-00083/1569860.pdf (minimum angle of elevation: 25 deg)
SATELLITE_CONE_RADIUS_M = 940700

MAX_GSL_LENGTH_M = math.sqrt(math.pow(SATELLITE_CONE_RADIUS_M, 2) + math.pow(ALTITUDE_M, 2))

# ISLs are not allowed to dip below 80 km altitude in order to avoid weather conditions
MAX_ISL_LENGTH_M = 2 * math.sqrt(math.pow(EARTH_RADIUS + ALTITUDE_M, 2) - math.pow(EARTH_RADIUS + 80000, 2))

NUM_ORBS = 72
NUM_SATS_PER_ORB = 22
INCLINATION_DEGREE = 53

################################################################


main_helper = MainHelper(
    BASE_NAME,
    NICE_NAME,
    ECCENTRICITY,
    ARG_OF_PERIGEE_DEGREE,
    PHASE_DIFF,
    MEAN_MOTION_REV_PER_DAY,
    ALTITUDE_M,
    MAX_GSL_LENGTH_M,
    MAX_ISL_LENGTH_M,
    NUM_ORBS,
    NUM_SATS_PER_ORB,
    INCLINATION_DEGREE,
)


def main():
    args = sys.argv[1:]
    if len(args) != 6:
        print("Must supply exactly six arguments")
        print("Usage: python main_starlink_550.py [duration (s)] [time step (ms)] "
              "[isls_plus_grid / isls_none] "
              "[ground_stations_{top_100, paris_moscow_grid}] "
              "[algorithm_{free_one_only_over_isls, free_one_only_gs_relays, paired_many_only_over_isls}] "
              "[num threads]")
        exit(1)
    else:
        main_helper.calculate(
            "gen_data",
            int(args[0]),
            int(args[1]),
            args[2],
            args[3],
            args[4],
            int(args[5]),
        )


if __name__ == "__main__":
    main()
