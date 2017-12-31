#!/bin/sh

case "$1" in
    tun0)
	ping6 -r -c 3 -i 1 -W 3 -I tun0 2a01:d0:0:1c::244 
	;;
    tun1)
	ping6 -r -c 3 -i 1 -W 3 -I tun1 2001:470:0:7b::1
	;;
esac
echo "$0 dev:$1 time:$2" >> log
