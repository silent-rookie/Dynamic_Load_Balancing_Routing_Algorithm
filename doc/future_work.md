# Future Work

## 1. Simulate a complete period
Currently this algorithm only simulates 200 seconds, and the complete period is 5736 seconds(Starlink S1 shell, altitude 550km). Simulating a full period may cause unexpected problems.

## 2. Change the way of traffic generation
The current way of traffic generation may result in a congested area being declared, but has no chance to be removed. You can see detail in `(project_dir)/ns3-sat-sim/simulator/contrib/basic-sim/model/apps/udp-burst-application.cc`.

## 3. Change the way of deleting congested areas
Currently the way we delete congested areas is to **determine at each time interval** whether the LEO satellite flew time `P` within a congested area and did not change to warning state.  
But we know that the motion of LEO satellites is periodic, and we can calculate the speed based on the current orbit altitude of the LEO satellite, and then calculate when to enter a congested area and when to leave the congested area.  
According to the time calculated above, **we can set a timer to notify the program that a certain LEO satellite enters a congestion area at a certain time**, so as to avoid detection at every time interval.  
But this requires a good familiarity with the calculations associated with satellite orbits. ns3 uses the `sgp4` module to calculate the satellite orbit. My ability is limited, I didn't finish the work, perhaps you can finish it `:-)`.

## 4. Use Cesium to visualize ILL utilization
`Hypatia` framework only supports the visualization of ISL utilization. We can extend it to support visualizing ILL utilization. You can learn from `(project_dir)/ns3-sat-sim/simulator/contrib/satellite-network/model/point-to-point-laser-net-device.cc, PointToPointLaserNetDevice::TrackUtilization` and implement in `(project_dir)/ns3-sat-sim/simulator/contrib/satellite-network/model/gsl-net-device.cc`(because we use gsl netdevice as ill netdevice). 