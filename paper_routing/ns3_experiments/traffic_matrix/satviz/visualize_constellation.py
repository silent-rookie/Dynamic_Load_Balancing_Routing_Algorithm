# Author: silent-rookie     2024

import math
try:
    from . import util
except (ImportError, SystemError):
    import util

# Generate static visualizations for GEO/LEO constellation.

EARTH_RADIUS_M = 6378135.0 # WGS72 value; taken from https://geographiclib.sourceforge.io/html/NET/NETGeographicLib_8h_source.html

# CONSTELLATION GENERATION GENERAL CONSTANTS
ECCENTRICITY = 0.0000001  # Circular orbits are zero, but pyephem does not permit 0, so lowest possible value
ARG_OF_PERIGEE_DEGREE = 0.0
PHASE_DIFF = True
EPOCH = "2000-01-01 00:00:00"

# Shell wise color codes
# COLOR = [[255, 0, 0, 200], [32, 128, 46, 200], [0, 0, 255, 200], [245, 66, 242, 200], [245, 126, 66, 200]]
COLOR = ['CRIMSON', 'FORESTGREEN', 'DODGERBLUE', 'PERU', 'BLUEVIOLET', 'DARKMAGENTA']

# CONSTELLATION SPECIFIC PARAMETERS

NAME = "Starlink_GEO_Structure"

# STARLINK S1 Shell
MEAN_MOTION_REV_PER_DAY_LEO = 15.19  # Altitude ~550 km
ALTITUDE_LEO_M = 550000  # Altitude ~550 km
NUM_ORBS_LEO = 72
NUM_SATS_PER_ORB_LEO = 22
INCLINATION_DEGREE_LEO = 53

# GEO
MEAN_MOTION_REV_PER_DAY_GEO = 1 # 0.997268
ALTITUDE_GEO_M = 35786000  # Altitude ~35786 km
NUM_ORBS_GEO = 1
NUM_SATS_PER_ORB_GEO = 3
INCLINATION_DEGREE_GEO = 0.001 # pyephem does not permit 0, so lowest possible value



# General files needed to generate visualizations; Do not change for different simulations
topFile = "static_html/top.html"
bottomFile = "static_html/bottom.html"

# Output directory for creating visualization html files
OUT_DIR = "../figures/"
OUT_HTML_FILE = OUT_DIR + NAME + ".html"



def generate_satellites_structure():
    """
    Generates satellite structure to visualization.
    :return: viz_string
    """
    viz_string = ""

    # Helper function to add satellite
    def add_satellite(sat_obj, alt_km, color, pixelSize):
        return f"""
            var redSphere = viewer.entities.add({{
                name: '', 
                position: Cesium.Cartesian3.fromDegrees({math.degrees(sat_obj.sublong)}, {math.degrees(sat_obj.sublat)}, {alt_km * 1000}), 
                point : {{
                    pixelSize: {pixelSize}, color: Cesium.Color.{color}.withAlpha(1)
                }}
            }});\n
        """

    # Helper function to add orbit links
    def add_leo_orbit_link(sat_objs, first, second, color):
        sat1 = sat_objs[first]["sat_obj"]
        sat2 = sat_objs[second]["sat_obj"]
        return f"""
            viewer.entities.add({{
                name: '',
                polyline: {{
                    positions: Cesium.Cartesian3.fromDegreesArrayHeights([
                        {math.degrees(sat1.sublong)}, {math.degrees(sat1.sublat)}, {sat_objs[first]["alt_km"] * 1000},
                        {math.degrees(sat2.sublong)}, {math.degrees(sat2.sublat)}, {sat_objs[second]["alt_km"] * 1000}
                    ]),
                    width: 0.5,
                    arcType: Cesium.ArcType.NONE,
                    material: new Cesium.PolylineOutlineMaterialProperty({{
                        color: Cesium.Color.{color}.withAlpha(0.4),
                        outlineWidth: 0,
                        outlineColor: Cesium.Color.BLACK
                    }})
                }}
            }});\n
        """
    def add_geo_orbit(color, radius):
        return f"""
            var radius = {radius}; // GEO altitude + earth radius 

            var circlePoints = []
            for (var i = 0; i < 360; i++) {{
                var angle = Cesium.Math.toRadians(i);
                circlePoints.push(new Cesium.Cartesian3(radius * Math.cos(angle), radius * Math.sin(angle), 0));
            }}

            viewer.entities.add({{
                polyline: {{
                    positions: circlePoints,
                    width: 2,
                    material: Cesium.Color.{color}
                }}
            }});
        """

    def add_to_image_LEO(sat_objs, NUM_ORBS, NUM_SATS_PER_ORB, color):
        res = ""
        for sat_obj in sat_objs:
            sat_obj["sat_obj"].compute(EPOCH)
            res += add_satellite(sat_obj["sat_obj"], sat_obj["alt_km"], "BLACK", 2)
        orbit_links = util.generate_orbit_links(sat_objs, NUM_ORBS, NUM_SATS_PER_ORB)
        for key in orbit_links:
            first = orbit_links[key]["sat1"]
            second = orbit_links[key]["sat2"]
            res += add_leo_orbit_link(sat_objs, first, second, color)
        
        return res
    
    def add_to_image_GEO(sat_objs, NUM_ORBS, NUM_SATS_PER_ORB, color):
        res = ""
        for sat_obj in sat_objs:
            sat_obj["sat_obj"].compute(EPOCH)
            res += add_satellite(sat_obj["sat_obj"], sat_obj["alt_km"], "RED", 8)
        res += add_geo_orbit(color, ALTITUDE_GEO_M + EARTH_RADIUS_M)
        
        return res

    # Starlink
    sat_objs = util.generate_sat_obj_list(
        NUM_ORBS_LEO,
        NUM_SATS_PER_ORB_LEO,
        EPOCH,
        PHASE_DIFF,
        INCLINATION_DEGREE_LEO,
        ECCENTRICITY,
        ARG_OF_PERIGEE_DEGREE,
        MEAN_MOTION_REV_PER_DAY_LEO,
        ALTITUDE_LEO_M
    )
    viz_string += add_to_image_LEO(sat_objs, NUM_ORBS_LEO, NUM_SATS_PER_ORB_LEO, COLOR[0])
    
    # GEO
    sat_objs = util.generate_sat_obj_list(
        NUM_ORBS_GEO,
        NUM_SATS_PER_ORB_GEO,
        EPOCH,
        PHASE_DIFF,
        INCLINATION_DEGREE_GEO,
        ECCENTRICITY,
        ARG_OF_PERIGEE_DEGREE,
        MEAN_MOTION_REV_PER_DAY_GEO,
        ALTITUDE_GEO_M
    )
    viz_string += add_to_image_GEO(sat_objs, NUM_ORBS_GEO, NUM_SATS_PER_ORB_GEO, COLOR[1])

    
    return viz_string

viz_string = generate_satellites_structure()
util.write_viz_files(viz_string, topFile, bottomFile, OUT_HTML_FILE)