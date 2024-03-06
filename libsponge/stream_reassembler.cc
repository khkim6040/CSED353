#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _stream(capacity), _output(capacity), _capacity(capacity), _unassembled_count(0), _next_read_point(0), _first_unread_point(-1) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
}

size_t StreamReassembler::unassembled_bytes() const {
    return _unassembled_count;
}

bool StreamReassembler::empty() const {
    return _unassembled_count == 0;
}
