#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return {}; }

size_t TCPConnection::bytes_in_flight() const { return {}; }

size_t TCPConnection::unassembled_bytes() const { return {}; }

size_t TCPConnection::time_since_last_segment_received() const { return {}; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (seg.header().rst) {
        // TODO: set in_bound, out_bound stream to error state
        _is_active = false;
        return;
    }
    _receiver.segment_received(seg);  // send seg to receiver
    // update sender's ackno, window size if ack is set
    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }

    // keep-alive segment
    if (_receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) &&
        seg.header().seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();
    }

    // send empty segment to ack if there is any non-zero length segment to send
    // it will ignore the ackno
    if (seg.length_in_sequence_space() != 0) {
        _sender.send_empty_segment();
    }

    send_packet();
}

bool TCPConnection::active() const { return _is_active; }

size_t TCPConnection::write(const string &data) {
    DUMMY_CODE(data);
    return {};
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }

void TCPConnection::end_input_stream() {}

void TCPConnection::connect() {
    _sender.fill_window();
    _is_active = true;
    send_packet();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::send_packet() {
    // seqno가 있는 걸 받았을 때는 ackno, window size를 포함한 패킷 보내야 함
    // 그러기 위해 _sender.send_empty_segment();를 호출해서 빈 패킷을 segment_out에 넣어놓아야
    // 아래 while문 안에서 header 세팅할 수 있음
    // sender에 있는 것도 보내야 함, 어떻게 할까?

    while (!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
            seg.header().win = _receiver.window_size();
        }
        _sender.segments_out().pop();
        // send seg to network
        _segments_out.push(seg);
    }
}