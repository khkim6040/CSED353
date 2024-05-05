Assignment 5 Writeup
=============

My name: Gwanho Kim

My POVIS ID: khkim6040

My student ID (numeric): 20190650

This assignment took me about 4 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:
- I used map data structure to map IP addresses to MAC addresses and its time.
- To handle ARP_REQUEST_TIMEOUT and ARP_MEMORY_TIMEOUT, I put both MAC address and its time in the map.
- Especially for ARP_REQUEST_TIMEOUT, I set its MAC address part as `ETHERNET_BROADCAST` to discern it from the normal MAC address.
- I made pending queue to store the packets that are waiting for ARP reply. 

Implementation Challenges:
- Understanding the ARP protocol
- Looking through the EthernetFrame, EthernetHeader, IPv4Datagram, IPv4Header and ARPMessage classes and understanding how to use it.

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: Understanding parse method logic which automatically sets the values of the fields in the classes. 

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: Data structure of pending deque. I used deque because I thought there is no gaurentee that the ARP reply will come in order. So, is it enough to use queue, not deque?
