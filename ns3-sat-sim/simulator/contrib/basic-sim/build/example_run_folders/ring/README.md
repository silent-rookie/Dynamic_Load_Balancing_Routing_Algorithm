Ring experiment:

1. Start 10 TCP flows from 0 to 3
2. Some of these 10 will use the up route, some of these will use the down route
3. The upper route has no queueing discipline, only FIFO queueing
4. The down route has FQ-CoDel queueing discipline, so gives fair sharing to all flows
5. On the upper route there, there is a UDP burst happening from 1 -> 3
6. Because of the burst, all flows that go upper route will get little to no bandwidth
7. The flows that chose the lower route will get a fair share of that bandwidth
