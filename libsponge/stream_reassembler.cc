#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _stream(capacity), _is_allocated(capacity), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string& data, const size_t index, const bool eof) {
    size_t stream_limit_index = _output.bytes_read() + _capacity - 1;  // The last index of the stream
    size_t data_length = data.length();
    // if data position exceeds stream size during not exceeding stream limit, resize vectors to obtain data
    bool resize_flag = index <= stream_limit_index && stream_size() <= index + data_length;
    if (resize_flag) {
        resize_vectors(data_length);
    }
    // Push data into stream
    handle_data(data, index, stream_limit_index, data_length);
    // Assemble data
    assemble_data();
    // Handle eof argument
    if (eof)
        set_eof();
    handle_stream_eof();
}

size_t StreamReassembler::unassembled_bytes() const {
    return _unassembled_count;
}

bool StreamReassembler::empty() const {
    return _unassembled_count == 0;
}

size_t StreamReassembler::stream_size() const {
    return _stream.size();
}

void StreamReassembler::set_eof() {
    _is_eof = true;
}

void StreamReassembler::handle_stream_eof() {
    if (_is_eof && empty() && !_ignore_flag) {
        _output.end_input();
    }
}

void StreamReassembler::resize_vectors(const size_t& data_length) {
    size_t new_size = _stream.size() + _capacity + data_length;
    _stream.resize(new_size);
    _is_allocated.resize(new_size);
}

void StreamReassembler::handle_data(const string& data, const size_t& index, const size_t& stream_limit_index, const size_t& data_length) {
    for (size_t i = index; i < index + data_length; i++) {
        // Out of stream memory case
        if (i > stream_limit_index) {
            // Ingore it
            _ignore_flag = true;
            // Stop considering additional data
            break;
        }
        // Check any data is already located in the position
        if (!_is_allocated[i]) {
            // If not, push it into the stream
            _is_allocated[i] = true;
            _stream[i] = data[i - index];
            _unassembled_count++;
            // Set the first unread point if it is not set or the current point is smaller than the first unread point
            if (!_is_first_unread_point_set || i < _first_unread_point) {
                _first_unread_point = i;
                _is_first_unread_point_set = true;
            }
        }
    }
}

void StreamReassembler::assemble_data() {
    // if element at the first unread point is allocated, assemble data
    if (_first_unread_point == _next_read_point) {
        string str;
        while (_is_allocated[_next_read_point]) {
            str += _stream[_next_read_point++];
            _unassembled_count--;
        }
        // Write assembled data into the output stream
        _output.write(str);
        // After assembling data, set _is_first_unread_point_set to false to set _first_unread_point each time data comes in
        _is_first_unread_point_set = false;
    }
}