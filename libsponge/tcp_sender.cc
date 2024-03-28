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
    while (!has_syn_sent || next_seqno_absolute() < _recent_abs_ackno + _window_size) {
        TCPSegment seg;
        TCPHeader &header = seg.header();
        header.seqno = next_seqno();
        if (next_seqno_absolute() == 0) {
            header.syn = true;
            has_syn_sent = true;
        }
        size_t window_size = _recent_abs_ackno + _window_size - next_seqno_absolute();
        size_t payload_size = min(window_size, static_cast<size_t>(TCPConfig::MAX_PAYLOAD_SIZE));
        string payload = _stream.read(payload_size);
        seg.payload() = Buffer(move(payload));
        if (_stream.eof() && !has_fin_sent && window_size > seg.length_in_sequence_space()) {
            header.fin = true;
            has_fin_sent = true;
        }

        if (seg.length_in_sequence_space() == 0) {
            break;
        }
        increase_next_seqno(seg.length_in_sequence_space());
        send_segment(seg);
        buffer_push(seg);
        _timer.fire();
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _window_size = window_size;
    size_t abs_ackno = unwrap(ackno, _isn, _recent_abs_ackno);
    while (!_outstanding_buffer.empty() &&
           abs_ackno >= 1LL * unwrap(_outstanding_buffer.front().header().seqno, _isn, _recent_abs_ackno) +
                            _outstanding_buffer.front().length_in_sequence_space()) {
        TCPSegment seg = _outstanding_buffer.front();
        _outstanding_buffer.pop();
        _outstanding_buffer_size -= seg.length_in_sequence_space();
        // Reset timer and consecutive retransmission count
        _timer.reset();
        _consecutive_retransmiss_count = 0;
    }
    _recent_abs_ackno = abs_ackno;

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
        // Retransmit the oldest segment
        TCPSegment oldest_seg = _outstanding_buffer.front();
        send_segment(oldest_seg);
        // Set RTO double
        _timer.double_timout_limit();
        _consecutive_retransmiss_count++;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmiss_count; }

void TCPSender::send_segment(TCPSegment segment) { _segments_out.push(segment); }

void TCPSender::send_empty_segment() {
    TCPSegment empty_seg;
    empty_seg.header().seqno = next_seqno();
    _segments_out.push(empty_seg);
}

void TCPSender::buffer_push(TCPSegment segment) {
    _outstanding_buffer.push(segment);
    _outstanding_buffer_size += segment.length_in_sequence_space();
}
