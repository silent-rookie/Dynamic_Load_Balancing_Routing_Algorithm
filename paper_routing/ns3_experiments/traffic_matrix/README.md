# Traffic Matrix

This directory is used for ns3 simulation. It tests this algorithm performance under different udp traffic rates.

## How to Run
1. `python3 step_1_generate_runs.py`
2. `python3 step_2_run.py`
3. `python3 step_3_generate_plots.py`

### Results and Logs
All results and logs are in `runs/(algorithm_name)/`.  
Each algorithm performance at different udp burst rate is in `runs/(algorithm_name)/algorithm_performance/algorithm_performance.txt`.  
Program detail time is in `runs/(algorithm_name)/logs_ns3/console.txt`.

### Figures
The figures of this algorithm performance is in `figures/`.

## How to generate Cesium figures
Before generate Cesium visualization, you need:  
1. Obtain a Cesium access token at [https://cesium.com/]()   
2. Edit `satviz/static_html/top.html`, and insert your Cesium access token at line 10:
```javascript
Cesium.Ion.defaultAccessToken = '<CESIUM_ACCESS_TOKEN>';
```
After that:
```
cd satviz
python3 visualize_constellation.py
python3 visualize_utilization.py
```
It will generate `.html` file in `figures/`. You should open it with a **linux** browser(e.g. Firefox)

## How to change parameter
If you want to change the parameter of **Congestion prediction**, and (ISL, GSL, ILL) configuration, and total time of simulation, you can find in `run_list.py`.  
You can also find something in `templates/template_config_ns3_udp.properties`.  

### Simulate a complete period
change durations from `200` to `5736` in file `run_list.py`.  
 
***Note***: If you change the total time of simualtion, and `dynamic_state_update_interval_ns`(granularity), **you should generate the corresponding data beforehand**(see `(project_dir)/paper_routing/satellite_networks_state/README.md`).