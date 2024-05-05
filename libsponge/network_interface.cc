#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    // check if we have the Ethernet address of the next hop in our ARP table
    auto iter = _arp_table.find(next_hop_ip);
    // if it is in pending state(= assigned MAC address is ETHERNET_BROADCAST), do nothing
    // in tick(), we will check if the pending timeout is reached
    if (iter != _arp_table.end() && iter->second.first == ETHERNET_BROADCAST) {
        return;
    }

    if (iter != _arp_table.end()) {
        // if we have the Ethernet address of the next hop
        // create an Ethernet frame with the IP datagram as payload
        EthernetFrame frame;
        frame.header().dst = _arp_table[next_hop_ip].first;
        frame.header().src = _ethernet_address;
        frame.header().type = EthernetHeader::TYPE_IPv4;
        frame.payload().append(dgram.serialize());

        // push frame onto outbound queue
        _frames_out.push(frame);

    } else {
        // if not, send an ARP request to the next hop
        ARPMessage arp_request;
        arp_request.opcode = ARPMessage::OPCODE_REQUEST;
        arp_request.sender_ethernet_address = _ethernet_address;
        arp_request.sender_ip_address = _ip_address.ipv4_numeric();
        arp_request.target_ip_address = next_hop_ip;

        // create Ethernet frame with ARP message as payload
        EthernetFrame frame;
        frame.header().dst = ETHERNET_BROADCAST;
        frame.header().src = _ethernet_address;
        frame.header().type = EthernetHeader::TYPE_ARP;
        frame.payload().append(arp_request.serialize());

        // push request frame onto outbound queue
        _frames_out.push(frame);
        // set arp table with pending state(== ETHERNET_BROADCAST) to estimate pending timeout in tick()
        _arp_table[next_hop_ip] = {ETHERNET_BROADCAST, 0};
        // push datagram and next hop address to pending deque
        _pending_deque.push_back({dgram, next_hop});
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if (frame.header().dst != ETHERNET_BROADCAST && frame.header().dst != _ethernet_address) {
        // frame is not for us
        return nullopt;
    }

    switch (frame.header().type) {
        case EthernetHeader::TYPE_IPv4: {
            // frame contains an IPv4 datagram
            InternetDatagram dgram;
            // parse as IPv4 datagram and return to caller
            if (dgram.parse(frame.payload()) == ParseResult::NoError) {
                return dgram;
            }
            break;
        }
        case EthernetHeader::TYPE_ARP: {
            // frame contains an ARP message
            ARPMessage arp_message;
            // parse as ARP message
            if (arp_message.parse(frame.payload()) == ParseResult::NoError) {
                auto iter = _arp_table.find(arp_message.sender_ip_address);
                // learn sender information if it is not in arp table or in pending state
                if (iter == _arp_table.end() || iter->second.first == ETHERNET_BROADCAST) {
                    _arp_table[arp_message.sender_ip_address] = {arp_message.sender_ethernet_address, 0};
                }

                switch (arp_message.opcode) {
                    case ARPMessage::OPCODE_REQUEST: {
                        // ARP request
                        if (arp_message.target_ip_address == _ip_address.ipv4_numeric()) {
                            // ARP request is for us
                            ARPMessage arp_reply;
                            arp_reply.opcode = ARPMessage::OPCODE_REPLY;
                            arp_reply.sender_ethernet_address = _ethernet_address;
                            arp_reply.sender_ip_address = _ip_address.ipv4_numeric();
                            arp_reply.target_ethernet_address = arp_message.sender_ethernet_address;
                            arp_reply.target_ip_address = arp_message.sender_ip_address;

                            // create Ethernet frame with ARP reply as payload
                            EthernetFrame reply_frame;
                            reply_frame.header().dst = arp_message.sender_ethernet_address;
                            reply_frame.header().src = _ethernet_address;
                            reply_frame.header().type = EthernetHeader::TYPE_ARP;
                            reply_frame.payload().append(arp_reply.serialize());

                            // push frame onto outbound queue
                            _frames_out.push(reply_frame);
                        }
                        break;
                    }
                    case ARPMessage::OPCODE_REPLY: {
                        // ARP reply
                        for (auto it = _pending_deque.begin(); it != _pending_deque.end(); it++) {
                            // send datagram if the sender IP address of the ARP reply matches
                            // the target IP address of the pending datagram
                            if (it->second.ipv4_numeric() == arp_message.sender_ip_address) {
                                send_datagram(it->first, it->second);
                                _pending_deque.erase(it);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            break;
        }
    }

    return nullopt;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    // update ARP table
    for (auto it = _arp_table.begin(); it != _arp_table.end();) {
        it->second.second += ms_since_last_tick;
        // handle two timeout cases
        if (it->second.first == ETHERNET_BROADCAST) {
            // first, pending timeout
            if (it->second.second >= ARP_REQUEST_TIMEOUT) {
                it = _arp_table.erase(it);
            } else {
                it++;
            }
        } else {
            // second, normal timeout
            if (it->second.second >= ARP_MEMORY_TIMEOUT) {
                it = _arp_table.erase(it);
            } else {
                it++;
            }
        }
    }
}
