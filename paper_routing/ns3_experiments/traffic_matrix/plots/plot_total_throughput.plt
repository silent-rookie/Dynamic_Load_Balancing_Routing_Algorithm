# Author: silent-rookie     2024

########################################
### STYLING

# Terminal
set terminal pngcairo size 800,600 enhanced font "Verdana,10"

# Line styles
set style line 1 lc rgb "black" lt 1 pt 1
set style line 2 lc rgb "red" lt 1 pt 2

########################################
### AXES AND KEY

# Title
# set title "Total throughput"

# Axes labels
set xlabel "Average bitrate per flow [Mbps]"
set ylabel "Total throughput [Mbps]"

# Axes ranges
set xtics 0.5

# Key
set key top left

########################################
### PLOTS

# Output
set output "../figures/plot_total_throughput.png"

# Plot
set datafile separator ","
plot    "../data/SingleForward_Total_Throughput.csv" using 1:2 w lp ls 1 title "DSP", \
        "../data/DetourTrafficClassify_Total_Throughput.csv" using 1:2 w lp ls 2 title "Proposed algorithm"