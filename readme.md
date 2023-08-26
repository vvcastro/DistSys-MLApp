In broadcasting, a sender node tries to send a message to all the nodes in the system. **Reliable broadcast** has one basic property: that a message to be broadcast should be received by all the nodes that are operational.

**Virtual synchronity** -> if node A sends a message at time=t and node B sends a message at time=t, we have (1) a resolution mechanism ( prob the first to enter the system ) and (2) if the mechanism says that A has priority, then all the other nodes will see the message from A before than the one from B.

**_Notes_**: Eventually I will have to find a way to sync receiving messages with sendind them. Seq numbers needs to be added to sent messages and updated when reading
-> Probably a lock system.
