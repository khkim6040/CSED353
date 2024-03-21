#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.
#include <iostream>

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    // Set ISN
    if (!_has_SYN_arrived) {
        if (seg.header().syn) {
            _isn = seg.header().seqno;
            _has_SYN_arrived = true;
            // _first_unacceptable = 1 + _capacity;
        }
    }

    if (!_has_FIN_arrived) {
        if (seg.header().fin) {
            _has_FIN_arrived = true;
            _FIN_abs_seqno = unwrap(seg.header().seqno + seg.length_in_sequence_space() - 1, _isn, _recent_abs_seqno);
        }
    }

    // Push seg into reassembler
    // Unpack seg seqno
    uint64_t abs_seqno = unwrap(seg.header().seqno, _isn, _recent_abs_seqno);
    _recent_abs_seqno = abs_seqno;  // can i remove it?

    if (abs_seqno == 0) {
        // only SYN exists
        if (seg.length_in_sequence_space() == 1) {
            return;
        } else {
            _reassembler.push_substring(seg.payload().copy(), 0, _has_FIN_arrived);
            return;
        }
    }
    // seg.serialize().concatenate()
    _reassembler.push_substring(seg.payload().copy(), abs_seqno - 1, _has_FIN_arrived);
    // Set FIN
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    // If no SYN has been received, return empty
    if (!_has_SYN_arrived) {
        return {};
        // return windows left edge
    }
    uint64_t temp = _has_SYN_arrived + _reassembler.get_next_read_point();
    if (temp == _FIN_abs_seqno) {
        // std::cout << "unassembled_bytes: " << _reassembler.unassembled_bytes() << endl;
        return wrap(temp + _has_FIN_arrived, _isn);
    }
    return wrap(temp, _isn);
}

size_t TCPReceiver::window_size() const {
    // return first unacceptable - first unassembled
    return _capacity - stream_out().buffer_size();
}
