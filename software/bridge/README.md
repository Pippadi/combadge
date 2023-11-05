# combridge

A bridge which aggregates and routes audio streams from multiple badges/clients, implementing the [Combadge Real-time Audio Protocol](/software/combadge).

Audio transmissions from clients are aggregated by the bridge such that each client receives a single audio stream to play back.

For now, the aggregated stream for a client is the sample-by-sample sum of the streams from other clients.
In time, streams will be converted to frequency domain and aggregated that way. This makes handling of streams with different sample rates and packet sizes easier.
Acoustic echo cancellation, active noice cancellation, and automatic gain control are also on the list of things to implement.

In its current state, the bridge is very rudimentary, kludgy, and buggy.
