# A Dynamic Load Balancing Routing Algorithm for GEO/LEO Hybrid Satellite Networks
 
 This algorithm is a dynamic load balancing routing algorithm for GEO/LEO hybrid satellite networks. It is based on congestion prediction, and improve QoS(quality of service) by classifying network traffic. And this algorithm is implemented by [`Hypatia`](https://github.com/snkas/hypatia) framework.  
 Keyworks: **:GEO/LEO hybrid network**, **congestion prediction**, **load balancing**, **traffic classification**, **traffic detour**

 <a href="#"><img alt="first0" src="readme_image/geo_leo_satellite_network_structure0.png" width="30%" /></a>
 <a href="#"><img alt="algorithm utilization0" src="readme_image/algorithm_utilization0.png" width="30%" /></a>
 <a href="#"><img alt="first1" src="readme_image/plot_packet_drop_rate.png" width="30%" /></a>

Algorithm overview:  
**Congestion prediction**: This algorithm declares the network congestion area as the congestion area, and the low-orbit satellite moving to the area can predict the congestion in advance.  
**Traffic detour**: This algorithm divides network traffic into three categories according to the sensitivity to delay, and executes different detour strategies for different traffic when the network is congested.

**Details about this algorithm can be found [`here`](doc/algorithm_details.md).**  
The paper proposing this algorithm is:
```
{
    title:      Load Balancing and QoS Provisioning Based on Congestion Prediction for GEO/LEO Hybrid Satellite Networks
    author:     Hiroki Nishiyama, Daigo Kudoh, Nei Kato and Naoto Kadowaki
    booktitle:  IEEE
    year:       2011
}
```
(In addition, **I am not the author of this algorithm**, I just implemented the algorithm in code and modified some details)

## Getting started

1. System setup:
   - Python version 3.8+
   - Recent Linux operating system (e.g., Ubuntu 20+)

2. Install Hypatia dependencies:
   ```
   bash hypatia_install_dependencies.sh
   ```
   
3. Build ns3 modules:
   ```
   bash hypatia_build.sh
   ```
   
4. Generate GEO/LEO satellite network data:  
   ```
   bash generate_satellite_network_data.sh
   ```
   (On machine with 4 cores and 4G memory, it takes about 1 hour)

5. Run ns3 to simulate:
    ```
    bash run_simulate.sh
    ```
    All simulation results and log are in `paper_routing/ns3_experiments/traffic_matrix/runs`  
    Algorithm performance picture are in `paper_routing/ns3_experiments/traffic_matrix/figures`  
    (On machine with 4 cores and 4G memory, it takes about 5~7 hour)

6. (optional) Cesium Visualization:  
    > Before generate Cesium visualization, you need:  
    > 1. Obtain a Cesium access token at [https://cesium.com/]()   
    > 2. Edit `paper_routing/ns3_experiments/traffic_matrix/satviz/static_html/top.html`, and insert your Cesium access token at line 10:
    >    ```javascript
    >    Cesium.Ion.defaultAccessToken = '<CESIUM_ACCESS_TOKEN>';
    >    ```
    After that:
    ```
    bash generate_cesium_visualization.sh
    ```
    It will generate `.html` file. You should open it with a **linux** browser(e.g., Firefox)

7. Change Parameter  
If you want to change the parameter of this algorithm, you can read the documents under `doc/`


## Visualizations
### Algorithm Performance
- Packet Drop Rate  
<a href="#"><img alt="Packet Drop Rate" src="readme_image/plot_packet_drop_rate.png" width="70%" /></a>
- Throughput  
<a href="#"><img alt="Throughput" src="readme_image/plot_throughput.png" width="70%" /></a>
- End-to-end delay  
<a href="#"><img alt="End-to-end delay" src="readme_image/plot_end-to-end_delay.png" width="70%" /></a>

### Cesium Visualizations
- GEO/LEO satellite network structure  
<a href="#"><img alt="satellite networke0" src="readme_image/geo_leo_satellite_network_structure0.png" width="40%" /></a>
<a href="#"><img alt="satellite networke1" src="readme_image/geo_leo_satellite_network_structure1.png" width="40%" /></a>
 
- this algorithm isl utilization  
<a href="#"><img alt="algorithm utilization0" src="readme_image/algorithm_utilization0.png" width="40%" /></a>
<a href="#"><img alt="algorithm utilization1" src="readme_image/algorithm_utilization1.png" width="50%" /></a>

- Dijkstra shortest path(DSP) isl utilization  
<a href="#"><img alt="DSP0" src="readme_image/DSP_utilization0.png" width="40%" /></a>
<a href="#"><img alt="DSP1" src="readme_image/DSP_utilization1.png" width="50%" /></a>