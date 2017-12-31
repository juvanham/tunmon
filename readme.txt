

tunmon is a tool that monitors via /proc/net/dev the incoming traffic of specified interfaces. The are listed as net_device elements in the net_devices part of its xml configuration

When these interfaces are tunnels (e.g., sit), this tools can monitor the state of the tunnel, without sending pings in case there is other incoming traffic.

When an interface does not receive any traffic during a specified number of iterations (of approx 1 sec), specified actions are executed.

When no traffic is received, it makes sense to send an icmp ping over this interface, then some response is expected. When this does not trigger any incoming traffic, it can indicate stronger recovery is needed.


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

using the arguments --trace 1, shows total traffic and other runtime info
