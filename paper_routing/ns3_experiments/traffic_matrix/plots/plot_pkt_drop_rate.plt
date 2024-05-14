# Author: silent-rookie     2024

########################################
### STYLING

# Terminal
set terminal pngcairo size 800,600 enhanced font "Verdana,10"

# Line styles
set style line 1 lc rgb "black" lt 1 pt 4
set style line 2 lc rgb "black" lt 1 pt 6
set style line 3 lc rgb "black" lt 1 pt 8
set style line 4 lc rgb "red" lt 1 pt 5
set style line 5 lc rgb "red" lt 1 pt 7
set style line 6 lc rgb "red" lt 1 pt 9

########################################
### AXES AND KEY

# Title
# set title "Packet drop rate"

# Axes labels
set xlabel "Average bitrate per flow [Mbps]"
set ylabel "Packet drop rate [%]"

# Axes ranges
set xtics 0.5

# Key
set key top left

########################################
### PLOTS

# Output
set output "../figures/plot_packet_drop_rate.png"

# Plot
set datafile separator ","
plot    "../data/SingleForward_A_PacketDropRate.csv" using 1:2 w lp ls 1 title "DSP(A)", \
        "../data/SingleForward_B_PacketDropRate.csv" using 1:2 w lp ls 2 title "DSP(B)", \
        "../data/SingleForward_C_PacketDropRate.csv" using 1:2 w lp ls 3 title "DSP(C)", \
        "../data/DetourTrafficClassify_A_PacketDropRate.csv" using 1:2 w lp ls 4 title "Proposed algorithm(A)", \
        "../data/DetourTrafficClassify_B_PacketDropRate.csv" using 1:2 w lp ls 5 title "Proposed algorithm(B)", \
        "../data/DetourTrafficClassify_C_PacketDropRate.csv" using 1:2 w lp ls 6 title "Proposed algorithm(C)"