#include "tcp_sender.hh"

#include <random>

#include "tcp_config.hh"

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
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()})), _initial_retransmission_timeout{retx_timeout}, _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const {
    // return size of outstanding structure
    return _outstanding_buffer.size();
}

void TCPSender::fill_window() {
    // If Closed state, send SYN
    TCPSegment seg;
    if (next_seqno_absolute() == 0) {
        seg.header().syn = true;
        seg.header().seqno = next_seqno();  // Should be _isn
        // _segments_out.push()
    }
    // Read payload from _stream as much as can min(PAYLOAD_MAX, window_size, _stream.size())
    // End with FIN

    _outstanding_buffer.push(seg);
    _next_seqno += seg.length_in_sequence_space();
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // set rwnd = min(1, window_size)
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    // May retransmit
    // Set RTO double
}

unsigned int TCPSender::consecutive_retransmissions() const { return {}; }

void TCPSender::send_empty_segment() {
    TCPSegment empty_seg;
    empty_seg.header().seqno = next_seqno();
    _segments_out.push(empty_seg);
}
