About Conjist
-------------
It logs into your jabber account to find people on your roster who are also 
running this software. Scans (using bonjour/zeroconf) for iTunes shares
or any other DAAP servers on your LAN, and makes them appear on your
friends' machines. Their shares appear on your machine.

Should be visible in the iTunes sidebar, in the "Local Music" section in
amarok, or any other player that supports DAAP servers.

You can browse and play music from friends' collections. Similar to what
simplify media does, but only relays existing DAAP shares, doesn't create 
its own DAAP server. Will relay as many DAAP shares as it finds on your LAN.

On linux you can use mt-daapd to share a collection if your media player 
doesn't act as a server.

Making it work
--------------
1) qmake && make
2) ./conjist <your jabber id> <your jabber password>
3) profit! (check iTunes/Amarok/etc for new shares)

Deps
----
* Qt (tested on 4.6)

* QJson
  On ubuntu: apt-get install libqjson-dev libqjson0 
  or from source: http://qjson.sourceforge.net/download.html

* Bonjour/zeroconf/avahi dev package for you platform.
  On ubuntu: apt-get install libavahi-compat-libdnssd-dev

Misc
----
To filter mDNS (bonjour) msgs from a given IP (good for some tests):
iptables -t filter -A INPUT -p udp --dport 5353 --source 192.168.1.6 -j DROP
