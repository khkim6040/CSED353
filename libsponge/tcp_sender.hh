#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "util/buffer.cc"
#include "util/buffer.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>

using namespace std;

class Timer {
  private:
    const uint32_t _initial_timeout;
    uint64_t _timeout_limit;
    uint64_t _current_time;
    bool _is_running;

  public:
    Timer(uint32_t _initial_retransmission_timeout)
        : _initial_timeout{_initial_retransmission_timeout}
        , _timeout_limit{_initial_retransmission_timeout}
        , _current_time{0}
        , _is_running{false} {};
    void increment(const uint64_t tick) { _current_time += tick; };
    bool is_expired() { return _is_running && _current_time >= _timeout_limit; };
    void reset() {
        _timeout_limit = _initial_timeout;
        _current_time = 0;
        _is_running = true;
    };
    void double_timout_limit() {
        _timeout_limit *= 2;
        _current_time = 0;
    }
    bool is_running() { return _is_running; };
    void fire() { _is_running = true; };
    void stop() { _is_running = false; };
};

//! \brief The "sender" part of a TCP implementation.

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
  private:
    Timer _timer;
    // Outstanding buffer list
    queue<TCPSegment> _outstanding_buffer{};
    uint64_t _outstanding_buffer_size{0};
    // The most recent absolute ackno, also be used as checkpoint
    uint64_t _recent_abs_ackno{0};
    // Consecutive retransmission
    uint64_t _consecutive_retransmiss_count{0};
    // Window size
    uint64_t _window_size{0};
    bool has_syn_sent{false};
    bool has_fin_sent{false};
    //! our initial sequence number, the number for our SYN.
    WrappingInt32 _isn;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};

    //! retransmission timer for the connection
    unsigned int _initial_retransmission_timeout;

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;

    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};

  public:
    //! Initialize a TCPSender
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    //! \name "Input" interface for the writer
    //!@{
    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }
    //!@}

    //! \name Methods that can cause the TCPSender to send a segment
    //!@{

    //! \brief A new acknowledgment was received
    void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    //! \brief Send a segment
    void send_segment(TCPSegment segment);

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    //! \brief Push a segment into the outstanding buffer
    void buffer_push(TCPSegment segment);

    //! \brief Increase the next sequence number by the size of the segment
    void increase_next_seqno(size_t size) { _next_seqno += size; };

    //! \brief create and send segments to fill as much of the window as possible
    void fill_window();

    //! \brief Notifies the TCPSender of the passage of time
    void tick(const size_t ms_since_last_tick);
    //!@}

    //! \name Accessors
    //!@{

    //! \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //! \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    //! \brief TCPSegments that the TCPSender has enqueued for transmission.
    //! \note These must be dequeued and sent by the TCPConnection,
    //! which will need to fill in the fields that are set by the TCPReceiver
    //! (ackno and window size) before sending.
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    //!@}

    //! \name What is the next sequence number? (used for testing)
    //!@{

    //! \brief absolute seqno for the next byte to be sent
    uint64_t next_seqno_absolute() const { return _next_seqno; }

    //! \brief relative seqno for the next byte to be sent
    WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
    //!@}
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
