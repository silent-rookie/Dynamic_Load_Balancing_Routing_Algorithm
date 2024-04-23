import os


# combine all tle files under tles_path to tles_filename
# the tle files must begin with "tles"
def combine_tles(tles_path, tles_filename):
    fullname = os.path.join(tles_path, tles_filename)
    with open(fullname, "a+") as f_out:
        for root, ds, fs in os.walk(tles_path):
            for f in fs:
                if (f != tles_filename) and f.startswith("tles") :
                    fullname = os.path.join(root, f)
                    with open(fullname, "r") as f_in:
                        f_out.write(f_in.read())