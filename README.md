# OSTEP
Code For OS TEP for projects and homework.

1. Completed the web server project. did a lot of soul searching before completing it :p.
2. The next one(pzip) has been more interesting and beautiful. My code is complex. Not sure, if its the right approach but tested
    a few times. didn't see any deadlock/race issue. Not able to check if the performance was optimal.
3. Complete the Project based on MapReduce paper by Google. It was quite elegant and simple idea. Implementing on single computer doesn't seem to
    be that difficult but multiple machines machine involved would be fun.

4. Next will see if should read some chapters or one more project can be done before moving on to persistence.

5. Completed presistence. Worked on creating library for RPC calls for sending and receiving. Implemented timeout/retry approach along with arbitrary size packet transfer.
If the packet is larger in size, it is fragmented in the transmit library and sent in pieces. Receiver will assimilate these pieces and send it to the upper level.
