#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _buffer(), _capacity(capacity) {}

size_t ByteStream::write(const string &data) {
    // adjusted length
    const size_t adj_len = min(data.length(), remaining_capacity());
    for (size_t i = 0; i < adj_len; i++) {
        _buffer.push_back(data[i]);
    }
    _total_written += adj_len;
    return adj_len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t adj_len = min(len, buffer_size());
    return string(_buffer.begin(), _buffer.begin() + adj_len);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    const size_t adj_len = min(len, buffer_size());
    for (size_t i = 0; i < adj_len; i++) {
        _buffer.pop_front();
    }
    _total_read += adj_len;
}

//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string str = peek_output(len);
    pop_output(len);
    return str;
}

void ByteStream::end_input() { _is_ended = true; }

bool ByteStream::input_ended() const { return _is_ended; }

size_t ByteStream::buffer_size() const { return _buffer.size(); }

bool ByteStream::buffer_empty() const { return buffer_size() == 0; }

bool ByteStream::eof() const {
    // Must both getting end signal and buffer is empty
    return input_ended() && buffer_empty();
}

size_t ByteStream::bytes_written() const { return _total_written; }

size_t ByteStream::bytes_read() const { return _total_read; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_size(); }