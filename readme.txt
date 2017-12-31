2017, Jurgen Van Ham

tunmon is a tool that monitors via /proc/net/dev the incoming traffic of specified interfaces. The network interfaces  are listed as net_device elements in the net_devices part of its xml configuration.

When these interfaces are tunnels (e.g., sit), this tools can monitor the state of the tunnel, without sending pings in case there is (other) incoming traffic. 

When an interface does not receive any traffic during a specified number of iterations (of approx 1 sec), specified actions are executed.

When no traffic is received, it makes sense to send an icmp ping over this interface, some response is expected. When this does not trigger any incoming traffic, it can indicate stronger recovery is needed.


<tun_mon>
  <net_devices>
    <net_device>tun0</net_device>
    <net_device>tun1</net_device>
  </net_devices>
  <actions>
    <action><time>15</time><script>./script/ping.sh</script></action>
  </actions>
</tun_mon>

The action scripts are called with arguments, the first is the name of the interface, the second the number of iterations (time) that no traffic was received.

using the arguments '--trace 1', shows total traffic over specified network interfaces and other runtime info

This tool was intended to use with a linux kernel, but other systems that have a similar /proc/net/dev file with a correct layout might work as well. The provided files test.xml and script/ping.sh are only fit as a demonstration, I plan to use this tool inside a router, which handles multiple sit tunnels, its design is out of scope for this readme file. Multiple ping actions might deal with network glitches, when this does not help, reconstruction the tunnel with updated end points could help, a final action can trigger an alert to a human operator.
