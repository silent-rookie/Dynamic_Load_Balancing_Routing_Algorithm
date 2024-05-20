echo "Run simulate for the algorithm"
echo ""

cd paper_routing/ns3_experiments/traffic_matrix/ || exit 1

python3 step_1_generate_runs.py || exit 1
python3 step_2_run.py || exit 1
python3 step_3_generate_plots.py || exit 1

cd ../../../ || exit 1

echo ""
echo "Simulate complete"