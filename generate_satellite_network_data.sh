echo "Generating GEO/LEO satellite network data"
echo ""

cd paper_routing/satellite_networks_state || exit 1
bash generate_all_local.sh || exit 1
cd ../.. || exit 1

echo ""
echo "Generate complete"