echo "Generate Cesium visualization"
echo ""

cd paper_routing/ns3_experiments/traffic_matrix/satviz || exit 1

python3 visualize_constellation.py || exit 1
python3 visualize_utilization.py || exit 1

cd ../../../../ || exit 1

echo ""
echo "Generate Cesium visualization complete"