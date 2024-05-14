# Author: silent-rookie     2024

import exputil
import numpy as np
from run_list import get_run_list

local_shell = exputil.LocalShell()

# Create the data files
local_shell.remove_force_recursive("data")
local_shell.make_full_dir("data")

# Define file paths
file_paths = {
    "SingleForward_A_PacketDropRate": "data/SingleForward_A_PacketDropRate.csv",
    "SingleForward_B_PacketDropRate": "data/SingleForward_B_PacketDropRate.csv",
    "SingleForward_C_PacketDropRate": "data/SingleForward_C_PacketDropRate.csv",
    "SingleForward_A_Throughput": "data/SingleForward_A_Throughput.csv",
    "SingleForward_B_Throughput": "data/SingleForward_B_Throughput.csv",
    "SingleForward_C_Throughput": "data/SingleForward_C_Throughput.csv",
    "SingleForward_Total_Throughput": "data/SingleForward_Total_Throughput.csv",
    "SingleForward_A_Delay": "data/SingleForward_A_Delay.csv",
    "SingleForward_B_Delay": "data/SingleForward_B_Delay.csv",
    "SingleForward_C_Delay": "data/SingleForward_C_Delay.csv",
    "DetourTrafficClassify_A_PacketDropRate": "data/DetourTrafficClassify_A_PacketDropRate.csv",
    "DetourTrafficClassify_B_PacketDropRate": "data/DetourTrafficClassify_B_PacketDropRate.csv",
    "DetourTrafficClassify_C_PacketDropRate": "data/DetourTrafficClassify_C_PacketDropRate.csv",
    "DetourTrafficClassify_A_Throughput": "data/DetourTrafficClassify_A_Throughput.csv",
    "DetourTrafficClassify_B_Throughput": "data/DetourTrafficClassify_B_Throughput.csv",
    "DetourTrafficClassify_C_Throughput": "data/DetourTrafficClassify_C_Throughput.csv",
    "DetourTrafficClassify_Total_Throughput": "data/DetourTrafficClassify_Total_Throughput.csv",
    "DetourTrafficClassify_A_Delay": "data/DetourTrafficClassify_A_Delay.csv",
    "DetourTrafficClassify_B_Delay": "data/DetourTrafficClassify_B_Delay.csv",
    "DetourTrafficClassify_C_Delay": "data/DetourTrafficClassify_C_Delay.csv"
}

# Initialize an empty dictionary to store file objects
files = {}

try:
    # Attempt to open all files and store the file objects in the dictionary
    for key, path in file_paths.items():
        try:
            files[key] = open(path, "w+")
        except IOError as e:
            # If an error occurs, print the error and raise an exception
            print(f"Error opening file {path}: {e}")
            raise

    # Process each run
    for run in get_run_list():
        # Determine run directory
        run_dir = "runs/" + run["name"]
        performance_dir = run_dir + "/algorithm_performance"
        logs_ns3_dir = run_dir + "/logs_ns3"

        # Finished filename to check if done
        finished_filename = logs_ns3_dir + "/finished.txt"
        if not (local_shell.file_exists(finished_filename)
                and local_shell.read_file(finished_filename).strip() == "Yes"):
            print("Skipping: " + run_dir)
            continue

        print("Processing: " + run["name"])

        # collect performance information
        performance_columns = exputil.read_csv_direct_in_columns(
            performance_dir + "/algorithm_performance.csv",
            "pos_int,pos_float,pos_float,"
            "pos_float,pos_float,pos_float,pos_float,"
            "pos_float,pos_float,pos_float,pos_float,"
            "pos_float,pos_float,pos_float,pos_float"
        )
        # Expand to a one-dimensional list
        performance_columns = [item  for sublist in performance_columns  for item in sublist]
        print(performance_columns)

        # write into data files
        if run["algorithm"] == "SingleForward":
            total_throughput = performance_columns[7] + performance_columns[8] + performance_columns[9]
            files["SingleForward_A_PacketDropRate"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[3]))
            files["SingleForward_B_PacketDropRate"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[4]))
            files["SingleForward_C_PacketDropRate"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[5]))
            files["SingleForward_A_Throughput"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[7]))
            files["SingleForward_B_Throughput"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[8]))
            files["SingleForward_C_Throughput"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[9]))
            files["SingleForward_Total_Throughput"].write("%.10f,%.10f\n" % (performance_columns[1], total_throughput))
            files["SingleForward_A_Delay"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[11]))
            files["SingleForward_B_Delay"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[12]))
            files["SingleForward_C_Delay"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[13]))
        elif run["algorithm"] == "DetourTrafficClassify":
            total_throughput = performance_columns[7] + performance_columns[8] + performance_columns[9]
            files["DetourTrafficClassify_A_PacketDropRate"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[3]))
            files["DetourTrafficClassify_B_PacketDropRate"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[4]))
            files["DetourTrafficClassify_C_PacketDropRate"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[5]))
            files["DetourTrafficClassify_A_Throughput"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[7]))
            files["DetourTrafficClassify_B_Throughput"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[8]))
            files["DetourTrafficClassify_C_Throughput"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[9]))
            files["DetourTrafficClassify_Total_Throughput"].write("%.10f,%.10f\n" % (performance_columns[1], total_throughput))
            files["DetourTrafficClassify_A_Delay"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[11]))
            files["DetourTrafficClassify_B_Delay"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[12]))
            files["DetourTrafficClassify_C_Delay"].write("%.10f,%.10f\n" % (performance_columns[1], performance_columns[13]))
finally:
    # Close all files
    for f in files.values():
        f.close()

# Execute plots
local_shell.remove_force_recursive("figures")
local_shell.make_full_dir("figures")
local_shell.perfect_exec("cd plots; gnuplot plot_pkt_drop_rate.plt")
local_shell.perfect_exec("cd plots; gnuplot plot_throughput.plt")
local_shell.perfect_exec("cd plots; gnuplot plot_total_throughput.plt")
local_shell.perfect_exec("cd plots; gnuplot plot_end-to-end_delay.plt")