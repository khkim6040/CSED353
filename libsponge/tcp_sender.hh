#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>

using namespace std;

class Timer {
  private:
    const uint64_t _initial_timeout;
    uint64_t _timeout_limit;
    uint64_t _current_time;
    bool _is_running;

  public:
    Timer(uint64_t _initial_retransmission_timeout)
        : _initial_timeout{_initial_retransmission_timeout}
        , _timeout_limit{_initial_retransmission_timeout}
        , _current_time{0}
        , _is_running{false} {};
    void increment(const uint64_t tick) {
        if (!_is_running)
            return;
        _current_time += tick;
    };
    void reset() {
        _timeout_limit = _initial_timeout;
        _current_time = 0;
    };
    void double_timout_limit() {
        _timeout_limit *= 2;
        _current_time = 0;
    }
    void fire() {
        _is_running = true;
        _current_time = 0;
    };
    void stop() {
        _is_running = false;
        _current_time = 0;
    };
    bool is_running() const { return _is_running; };
    bool is_expired() const { return _is_running && _current_time >= _timeout_limit; };
};

//! \brief The "sender" part of a TCP implementation.

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
  private:
    // Outstanding buffer list
    queue<TCPSegment> _outstanding_buffer{};
    uint64_t _outstanding_buffer_size{0};
    // The most recent absolute ackno, also be used as checkpoint
    uint64_t _recent_abs_ackno{0};
    // Consecutive retransmission
    uint64_t _consecutive_retransmiss_count{0};
    // Window size
    uint16_t _window_size{0};
    bool _is_window_zero{false};
    bool _has_syn_sent{false};
    bool _has_fin_sent{false};
    //! our initial sequence number, the number for our SYN.
    WrappingInt32 _isn;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};

    //! retransmission timer for the connection
    unsigned int _initial_retransmission_timeout;

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;

    Timer _timer;

    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};

    //! \brief return true if there is still data to be sent
    bool can_send_more_data() const;

    /**
     * @brief Creates a TCP segment.
     *
     * This function creates a TCP segment with the following properties:
     * - The sequence number is set to the next sequence number.
     * - The SYN flag is set if needed.
     * - The payload is read by amount of the window right edge.
     * - The FIN flag is set if needed.
     *
     * @return The created TCP segment.
     */
    TCPSegment create_segment();

    //! \brief Increase the next sequence number by the size of the segment
    void increase_next_seqno(size_t size) { _next_seqno += size; };

    //! \brief Send a segment
    void send_segment(TCPSegment segment);

    //! \brief Push a segment into the outstanding buffer
    void buffer_push(TCPSegment segment);

    //! \brief set _window_size and _is_window_zero
    void handle_window_size(const uint16_t window_size);

    /**
     * @brief Update the outstanding buffer by removing segments that have been acknowledged.
     *
     * It removes the acknowledged segments from the outstanding buffer and updates the buffer size.
     * Additionally, it resets the timer and consecutive retransmission count.
     *
     * @param abs_ackno The absolute sequence number of the acknowledgment received.
     */
    void update_outstanding_buffer(const uint64_t abs_ackno);

    //! \brief return true if abs_ackno is greater than seqno plus length in the buffer queue
    bool is_ackno_greater_than_seqno_plus_length(const uint64_t abs_ackno, const TCPSegment &seg) const;

    //! \brief set the SYN flag if needed
    void set_syn_flag_if_needed(TCPHeader &header);

    //! \brief return the right edge seqno of the window
    size_t calculate_window_right_edge() const;

    //! \brief read payload from the stream
    string read_payload(const size_t window_right_edge);

    //! \brief set the FIN flag if needed
    void set_fin_flag_if_needed(TCPHeader &header, const TCPSegment &segment, size_t window_right_edge);

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

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    /**
     * @brief Create and send segments to fill as much of the window as possible
     *
     * Fills the sending window with TCP segments until the window is full or there is no more data to send.
     * This function is responsible for creating TCP segments, increasing the sequence number, sending the segments,
     * and pushing them into the buffer. It also fires the timer if it is not already running.
     */
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

    bool has_fin_sent() const { return _has_fin_sent; }
    bool has_sin_sent() const { return _has_syn_sent; }
    bool has_fin_acked() const { return _has_fin_sent && _recent_abs_ackno == _next_seqno; }
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
