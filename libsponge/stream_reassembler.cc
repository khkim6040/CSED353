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
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // Deal with input data
    size_t stream_limit_index = _output.bytes_read() + _capacity - 1;
    size_t data_length = data.length();
    if (index <= stream_limit_index && stream_size() <= index + data_length) {
        // resize _stream, _is_allocated
        size_t new_size = _stream.size() + _capacity + data_length;
        _stream.resize(new_size);
        _is_allocated.resize(new_size);
    }
    // Push data into stream
    for (size_t i = 0; i < data_length; i++) {
        // Out of stream memory
        if (index + i > stream_limit_index) {
            // Ingore it
            _ignore_flag = true;
            // Stop considering additional data
            break;
        }
        // Check any data is already located in the position
        if (!_is_allocated[index + i]) {
            _is_allocated[index + i] = true;
            _stream[index + i] = data[i];
            _unassembled_count++;
            if (!_is_first_unread_point_set || index + i < _first_unread_point) {
                _first_unread_point = index + i;
                _is_first_unread_point_set = true;
            }
        }
    }
    // Read phase
    if (_first_unread_point == _next_read_point) {
        string str;
        while (_is_allocated[_next_read_point]) {
            str.push_back(_stream[_next_read_point++]);
            _unassembled_count--;
        }
        _output.write(str);
        _is_first_unread_point_set = false;
    }
    // Handle eof flag
    if (eof && empty() && !_ignore_flag) {
        _output.end_input();
    }
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