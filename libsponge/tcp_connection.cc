#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (!active()) {
        return;
    }

    // Reset case: RST segment or ack when no SYN has been sent yet
    if (seg.header().rst) {
        handle_error(false);
        return;
    }

    _receiver.segment_received(seg);  // send seg to receiver
    // update sender's ackno, window size if ack is set
    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
        // fill window if there is left data to send
        if (_sender.has_sin_sent()) {
            _sender.fill_window();
        }
    }

    // keep-alive segment
    if (_receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) &&
        seg.header().seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();
    }

    // send empty segment to ack if there is any non-zero length segment to send
    // it will ignore the ackno, whose size is 0
    if (seg.length_in_sequence_space() != 0) {
        if (seg.header().syn && !_sender.has_sin_sent()) {
            // receive SYN before sending SYN
            // send SYN/ACK
            _sender.fill_window();
        } else {
            _sender.send_empty_segment();
        }
    }

    // passive close case: outbound stream has not sent FIN && inbound stream has ended after received segment
    if (!_sender.has_fin_sent() && _receiver.stream_out().input_ended()) {
        _linger_after_streams_finish = false;
    }

    // reset elapsed time
    _time_since_last_segment_received = 0;
    send_packet();
}

bool TCPConnection::active() const {
    if (_RST_flag) {
        return false;
    } else if (_is_active) {
        // TODO: remove this
        return true;
    } else {
        // TODO: fix this
        return false;
    }
}

size_t TCPConnection::write(const string &data) {
    if (!data.size())
        return 0;
    if (!active()) {
        return 0;
    }

    size_t written = _sender.stream_in().write(data);
    _sender.fill_window();
    send_packet();
    return written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!active()) {
        return;
    }

    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    // TODO: Fix this
    // ----------
    // retransmission limit reached
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        handle_error(true);
        return;
    }

    // close connection if all data sent and received
    if (unassembled_bytes() == 0 && _receiver.stream_out().input_ended() && bytes_in_flight() == 0 &&
        _sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2) {
        // active close case: all data has been sent and received
        if (_linger_after_streams_finish) {
            if (_time_since_last_segment_received >= 10 * _cfg.rt_timeout) {
                _is_active = false;
            }
        } else {
            // passive close case
            _is_active = false;
        }
    }
    // ----------
    send_packet();
}

void TCPConnection::end_input_stream() {
    if (!active()) {
        return;
    }

    _sender.stream_in().end_input();
    _sender.fill_window();
    send_packet();
}

void TCPConnection::connect() {
    if (!active()) {
        return;
    }

    _sender.fill_window();
    send_packet();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            handle_error(true);
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
            // Else, send segment with other flags
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
            seg.header().win = min(size_t(numeric_limits<uint16_t>::max()), _receiver.window_size());
        }
        _sender.segments_out().pop();
        // send seg to network
        _segments_out.push(seg);
    }
}

void TCPConnection::handle_error(bool sending) {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _RST_flag = true;
    if (sending) {
        _sender.send_empty_segment();
        TCPSegment seg = _sender.segments_out().front();
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
            seg.header().win = min(size_t(numeric_limits<uint16_t>::max()), _receiver.window_size());
        }
        _sender.segments_out().pop();
        seg.header().rst = true;
        _segments_out.push(seg);
    }
}