#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _timer(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const {
    // return size of outstanding structure
    return _outstanding_buffer_size;
}

void TCPSender::fill_window() {
    // If Closed state, send SYN
    TCPSegment seg;
    if (next_seqno_absolute() == 0) {
        seg.header().syn = true;
        seg.header().seqno = next_seqno();  // Should be _isn
        // _segments_out.push()
        // increment _next_seqno
    }
    // Read payload from _stream as much as can min(PAYLOAD_MAX, window_size, _stream.size())
    // End with FIN

    _outstanding_buffer.push(seg);
    _next_seqno += seg.length_in_sequence_space();
    _timer.fire();
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // set rwnd = min(1, window_size)
    if (_checkpoint == 0) {
        _checkpoint = _isn.raw_value();
    }
    _window_size = window_size;
    uint64_t abs_ackno = unwrap(ackno, _isn, _checkpoint);
    while (!_outstanding_buffer.empty() && abs_ackno >= 1LL * _outstanding_buffer.front().header().seqno.raw_value() +
                                                            _outstanding_buffer.front().length_in_sequence_space()) {
        TCPSegment seg = _outstanding_buffer.front();
        _outstanding_buffer.pop();
        _outstanding_buffer_size -= seg.length_in_sequence_space();
    }
    _checkpoint = abs_ackno;

    if (_outstanding_buffer.empty()) {
        _timer.stop();
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer.is_running())
        return;
    _timer.increment(ms_since_last_tick);
    if (_timer.is_expired()) {
        // retransmission logic here
        // send(_out_queue.front())
        // Set RTO double
        // _consecutive_retransmiss_cnt++
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { _consecutive_retransmiss_count; }

void TCPSender::send_segment(TCPSegment segment) { _segments_out.push(segment); }

void TCPSender::send_empty_segment() {
    TCPSegment empty_seg;
    empty_seg.header().seqno = next_seqno();
    _segments_out.push(empty_seg);
}
