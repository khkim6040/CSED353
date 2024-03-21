#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    // Set ISN
    bool syn_flag = seg.header().syn;
    bool fin_flag = seg.header().fin;
    // If SYN has arrived, set ISN
    if (!_has_SYN_arrived && syn_flag) {
        _has_SYN_arrived = true;
        _ISN = seg.header().seqno;
    }
    // If FIN has arrived, set FIN_abs_seqno to compare with ackno
    if (!_has_FIN_arrived && fin_flag) {
        _has_FIN_arrived = true;
        _FIN_abs_seqno = unwrap(seg.header().seqno + seg.length_in_sequence_space() - 1, _ISN, _recent_abs_seqno);
    }
    // Unavailable case
    if (!_has_SYN_arrived || seg.length_in_sequence_space() == 0) {
        return;
    }
    // Unwrap seg seqno
    size_t abs_seqno = unwrap(seg.header().seqno, _ISN, _recent_abs_seqno);
    _recent_abs_seqno = abs_seqno;
    // If SYN has first arrived and size is more than 0, abs_seqno should be equal to the stream_index, which is 0
    // It is enough to check syn_flag because we have already checked the availability of SYN
    size_t stream_index = syn_flag ? abs_seqno : abs_seqno - 1;
    _reassembler.push_substring(seg.payload().copy(), stream_index, fin_flag);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    // If no SYN has been received, return empty
    if (!_has_SYN_arrived) {
        return {};
    }
    size_t abs_ackno = _has_SYN_arrived + _reassembler.get_next_read_point();
    abs_ackno = abs_ackno == _FIN_abs_seqno ? abs_ackno + _has_FIN_arrived : abs_ackno;
    return wrap(abs_ackno, _ISN);
}

size_t TCPReceiver::window_size() const { return _reassembler.window_size(); }
