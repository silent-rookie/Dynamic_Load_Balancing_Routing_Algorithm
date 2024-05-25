Fat-tree k=4 w/ servers experiment:

1. Fat-tree (k=4) has 20 Mbit/s links, 10 microseconds delay, 60p drop-tail queue, no traffic-control queueing discipline

2. Poisson arrival of flows at 10 flows/s

3. Flow size is i.i.d. drawn from pFabric web search distribution (1.7MB average)

4. Load is `10 * 1.7MB = 17MB/s` on average, vs. a bi-section bandwidth of `k^3/4 * 20 Mbit/s = 40 MB/s`.

5. Source / destination drawn i.i.d. uniformly at random

6. Simulation is run for 10s
