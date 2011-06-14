SinOsc s => dac;

OscRecv recv;
8765 => recv.port;
recv.listen();
recv.event("/update, f f") @=> OscEvent recv_event;

while (true) {
	recv_event => now;
	while (recv_event.nextMsg() != 0) {
		recv_event.getFloat() => s.freq;
		//recv_event.getFloat() => s.volume;
	}
}
