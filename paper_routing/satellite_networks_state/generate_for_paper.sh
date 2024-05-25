# Usage help
if [ "$1" == "--help" ] || [ "$#" != "2" ]; then
  echo "Usage: bash generate_for_paper.sh [id: 0 - 14] [number of threads]"
  exit 0
fi

# Fetch arguments
id="$1"
num_threads=$2

# Check validity of arguments
if [ "${id}" -lt "0" ] || [ "${id}" -gt "2" ]; then
  echo "Invalid workload id: ${id}"
  exit 1
fi
if [ "${num_threads}" -lt "0" ] || [ "${num_threads}" -gt "128" ]; then
  echo "Invalid number of threads: ${num_threads}"
  exit 1
fi

# Print what is being run
echo "Running workload ${id} with ${num_threads} threads"

# Starlink-550 with ISLs
if [ "${id}" = "0" ]; then
  python3 main_starlink_GEO.py 200 50 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls_ills ${num_threads}
fi
if [ "${id}" = "1" ]; then
  python3 main_starlink_GEO.py 200 100 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls_ills ${num_threads}
fi
if [ "${id}" = "2" ]; then
  python3 main_starlink_GEO.py 200 1000 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls_ills ${num_threads}
fi

# # Kuiper-630 with ISLs
# if [ "${id}" = "3" ]; then
#   python main_kuiper_630.py 200 50 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
# fi
# if [ "${id}" = "4" ]; then
#   python main_kuiper_630.py 200 100 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
# fi
# if [ "${id}" = "5" ]; then
#   python main_kuiper_630.py 200 1000 isls_plus_grid ground_stations_top_100 algorithm_free_one_only_over_isls ${num_threads}
# fi