NS3_VERSION="ns-3.31"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash test.sh [--help] [--core, --apps, --distributed, --coverage]*"
  exit 0
fi

# Test results output folder
rm -rf test_results
mkdir -p test_results

# Rebuild to have the most up-to-date
bash rebuild.sh || exit 1

# Go into the ns-3 folder
cd ${NS3_VERSION} || exit 1

# Empty coverage counters
echo "Zeroing coverage counters"
lcov --directory build/debug_all --zerocounters

# Core tests
if [ "$1" == "" ] || [ "$1" == "--core" ] || [ "$2" == "--core" ] || [ "$3" == "--core" ] || [ "$4" == "--core" ]; then
  echo "Performing core tests"
  python test.py -v -s "basic-sim-core" -t ../test_results/test_results_core || exit 1
  cat ../test_results/test_results_core.txt
fi

# Apps tests
if [ "$1" == "" ] || [ "$1" == "--apps" ] || [ "$2" == "--apps" ] || [ "$3" == "--apps" ] || [ "$4" == "--apps" ]; then
  python test.py -v -s "basic-sim-apps" -t ../test_results/test_results_apps || exit 1
  cat ../test_results/test_results_apps.txt
fi

# Back to build/ directory
cd .. || exit 1

# Distributed tests
if [ "$1" == "" ] || [ "$1" == "--distributed" ] || [ "$2" == "--distributed" ] || [ "$3" == "--distributed" ] || [ "$4" == "--distributed" ]; then

  # Baseline
  bash run_assist.sh "example_run_folders/leaf_spine" 0 || exit 1

  # 1 core tests
  bash run_assist.sh "example_run_folders/leaf_spine_distributed_1_core_default" 1 || exit 1
  python test_distributed_exactly_equal.py "example_run_folders/leaf_spine" "example_run_folders/leaf_spine_distributed_1_core_default" 1 || exit 1

  bash run_assist.sh "example_run_folders/leaf_spine_distributed_1_core_nullmsg" 1 || exit 1
  python test_distributed_exactly_equal.py "example_run_folders/leaf_spine" "example_run_folders/leaf_spine_distributed_1_core_nullmsg" 1 || exit 1

  # 2 core tests
  bash run_assist.sh "example_run_folders/leaf_spine_distributed_2_core_default" 2 || exit 1
  python test_distributed_exactly_equal.py "example_run_folders/leaf_spine" "example_run_folders/leaf_spine_distributed_2_core_default" 2 || exit 1

  bash run_assist.sh "example_run_folders/leaf_spine_distributed_2_core_nullmsg" 2 || exit 1
  python test_distributed_exactly_equal.py "example_run_folders/leaf_spine" "example_run_folders/leaf_spine_distributed_2_core_nullmsg" 2 || exit 1

fi

# Coverage report
if [ "$1" == "" ] || [ "$1" == "--coverage" ] || [ "$2" == "--coverage" ] || [ "$3" == "--coverage" ] || [ "$4" == "--coverage" ]; then

  # Make coverage report
  rm -rf coverage_report
  mkdir -p coverage_report
  cd ${NS3_VERSION}/build/debug_all/ || exit 1
  lcov --capture --directory contrib/basic-sim --output-file ../../../coverage_report/coverage.info

  # Remove directories from coverage report which we don't want
  lcov -r ../../../coverage_report/coverage.info "/usr/*" "*/build/debug_all/ns3/*" "*/test/*" "test/*" --output-file ../../../coverage_report/coverage.info

  # Generate html
  cd ../../../ || exit 1
  genhtml --output-directory coverage_report coverage_report/coverage.info
  echo "Coverage report is located at: coverage_report/index.html"

  # Show results
  echo "Display test results"
  if [ "$1" == "" ] || [ "$1" == "--core" ] || [ "$2" == "--core" ] || [ "$3" == "--core" ] || [ "$4" == "--core" ]; then
    cat test_results/test_results_core.txt
  fi
  if [ "$1" == "" ] || [ "$1" == "--apps" ] || [ "$2" == "--apps" ] || [ "$3" == "--apps" ] || [ "$4" == "--apps" ]; then
    cat test_results/test_results_apps.txt
  fi

fi
