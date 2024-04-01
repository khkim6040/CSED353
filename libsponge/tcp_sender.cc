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

void TCPSender::fill_window() {
    while (can_send_more_data()) {
        TCPSegment segment = create_segment();
        if (segment.length_in_sequence_space() == 0) {
            break;
        }
        increase_next_seqno(segment.length_in_sequence_space());
        send_segment(segment);
        buffer_push(segment);
        if (!_timer.is_running()) {
            _timer.fire();
        }
    }
}

bool TCPSender::can_send_more_data() const {
    return !_has_syn_sent || next_seqno_absolute() < _recent_abs_ackno + _window_size;
}

TCPSegment TCPSender::create_segment() {
    TCPSegment segment;
    TCPHeader &header = segment.header();
    header.seqno = next_seqno();
    set_syn_flag_if_needed(header);
    size_t window_right_edge = calculate_window_right_edge();
    string payload = read_payload(window_right_edge);
    segment.payload() = Buffer(move(payload));
    set_fin_flag_if_needed(header, segment, window_right_edge);
    return segment;
}

void TCPSender::set_syn_flag_if_needed(TCPHeader &header) {
    if (next_seqno_absolute() == 0) {
        header.syn = true;
        _has_syn_sent = true;
    }
}

size_t TCPSender::calculate_window_right_edge() const {
    return _recent_abs_ackno + _window_size - next_seqno_absolute();
}

string TCPSender::read_payload(size_t window_right_edge) {
    size_t payload_size = min(window_right_edge, static_cast<size_t>(TCPConfig::MAX_PAYLOAD_SIZE));
    return _stream.read(payload_size);
}

void TCPSender::set_fin_flag_if_needed(TCPHeader &header, const TCPSegment &segment, size_t window_right_edge) {
    if (_stream.eof() && !_has_fin_sent && window_right_edge > segment.length_in_sequence_space()) {
        header.fin = true;
        _has_fin_sent = true;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // Case when window_size is 0
    handle_window_size(window_size);
    size_t abs_ackno = unwrap(ackno, _isn, _recent_abs_ackno);
    // abs_ackno should be less than next_seqno_absolute()
    if (abs_ackno > next_seqno_absolute()) {
        return;
    }
    update_outstanding_buffer(abs_ackno);
    _recent_abs_ackno = abs_ackno;

    if (_outstanding_buffer.empty()) {
        _timer.stop();
    }
}

void TCPSender::handle_window_size(const uint16_t window_size) {
    _window_size = max(uint16_t{1}, window_size);
    _is_window_zero = window_size == 0;
}

void TCPSender::update_outstanding_buffer(const uint64_t abs_ackno) {
    while (!_outstanding_buffer.empty() &&
           is_ackno_greater_than_seqno_plus_length(abs_ackno, _outstanding_buffer.front())) {
        TCPSegment seg = _outstanding_buffer.front();
        _outstanding_buffer.pop();
        _outstanding_buffer_size -= seg.length_in_sequence_space();
        // Reset timer and consecutive retransmission count
        _timer.reset();
        _consecutive_retransmiss_count = 0;
    }
}

bool TCPSender::is_ackno_greater_than_seqno_plus_length(const uint64_t abs_ackno, const TCPSegment &seg) const {
    return abs_ackno >= unwrap(seg.header().seqno, _isn, _recent_abs_ackno) + seg.length_in_sequence_space();
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
        if (_is_window_zero) {
            _timer.reset();
        } else {
            _timer.double_timout_limit();
            _consecutive_retransmiss_count++;
        }
    }
}

uint64_t TCPSender::bytes_in_flight() const { return _outstanding_buffer_size; }

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
