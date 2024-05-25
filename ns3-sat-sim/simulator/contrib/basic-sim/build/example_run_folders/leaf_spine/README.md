Leaf-spine experiment:

1. Start 4 TCP flows from 0 to 1
2. Some of these 4 will use the left route, some of these will use the right route
3. On one of these routes, here is a UDP burst happening from 0 -> 1 at 30 Mb/s
4. This means that the TCP flows that go on that route will share 20 Mb/s, whereas the other ones will share 50 Mb/s
5. Pings are sent in a 50ms interval

(the distributed versions of this experiment do the same, except that the nodes are distributed; results should be very similar)
