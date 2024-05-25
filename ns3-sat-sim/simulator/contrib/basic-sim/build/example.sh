# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash example.sh [--help]"
  exit 0
fi

# Rebuild
bash rebuild.sh || exit 1

# Single process (without MPI)
bash run_assist.sh "example_run_folders/single" 0 || exit 1
bash run_assist.sh "example_run_folders/ring" 0 || exit 1
bash run_assist.sh "example_run_folders/leaf_spine" 0 || exit 1
bash run_assist.sh "example_run_folders/leaf_spine_servers" 0 || exit 1
bash run_assist.sh "example_run_folders/grid" 0 || exit 1
bash run_assist.sh "example_run_folders/fat_tree_k4_servers" 0 || exit 1

# Check cores
num_cores=$(nproc --all)

# Distributed
bash run_assist.sh "example_run_folders/leaf_spine_distributed_1_core_default" 1 || exit 1
bash run_assist.sh "example_run_folders/leaf_spine_distributed_1_core_nullmsg" 1 || exit 1
if [ "${num_cores}" -ge "4" ]; then
  bash run_assist.sh "example_run_folders/leaf_spine_distributed_2_core_default" 2 || exit 1
  bash run_assist.sh "example_run_folders/leaf_spine_distributed_2_core_nullmsg" 2 || exit 1
fi

# Single process with some plotting
bash run_assist.sh "example_run_folders/fat_tree_k4_servers_poisson" 0 || exit 1
cd ../tools/plotting/plot_tcp_flows_ecdfs || exit 1
python plot_tcp_flows_ecdfs.py ../../../build/example_run_folders/fat_tree_k4_servers_poisson/logs_ns3 \
                               ../../../build/example_run_folders/fat_tree_k4_servers_poisson/logs_ns3/data \
                               ../../../build/example_run_folders/fat_tree_k4_servers_poisson/logs_ns3/pdf || exit 1
