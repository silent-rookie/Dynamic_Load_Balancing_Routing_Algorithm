# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash tutorial.sh [--help]"
  exit 0
fi

# Rebuild
bash rebuild.sh || exit 1

# Tutorial
bash run_assist.sh "example_run_folders/tutorial" 0 || exit 1

# Go plot helpers
cd ../tools/plotting || exit 1

# Link utilization plots
cd plot_link_utilization || exit 1
python plot_link_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 1 || exit 1
python plot_link_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 0 || exit 1
python plot_link_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 2 || exit 1
python plot_link_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 2 0 || exit 1
python plot_link_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 3 || exit 1
python plot_link_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 3 0 || exit 1
cd .. || exit 1

# Link queue plots
cd plot_link_queue || exit 1
python plot_link_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 1 || exit 1
python plot_link_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 0 || exit 1
python plot_link_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 2 || exit 1
python plot_link_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 2 0 || exit 1
python plot_link_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 3 || exit 1
python plot_link_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 3 0 || exit 1
cd .. || exit 1

# Ping plots
cd plot_ping || exit 1
python plot_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 2 100000000 || exit 1
python plot_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 3 100000000 || exit 1
python plot_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 2 1 100000000 || exit 1
python plot_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 2 3 100000000 || exit 1
python plot_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 3 1 100000000 || exit 1
python plot_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 3 2 100000000 || exit 1
cd .. || exit 1

# TCP flow plots
cd plot_tcp_flow || exit 1
python plot_tcp_flow.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 100000000 || exit 1
python plot_tcp_flow.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 100000000 || exit 1
python plot_tcp_flow.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 2 100000000 || exit 1
cd .. || exit 1

# UDP burst plots
cd plot_udp_burst || exit 1
python plot_udp_burst.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 100000000 || exit 1
python plot_udp_burst.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 100000000 || exit 1
cd .. || exit 1
