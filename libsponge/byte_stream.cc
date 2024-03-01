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
        _buffer.push(data[i]);
    }
    _total_written += adj_len;
    return adj_len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string str = concatenate_buffer();
    return str.substr(0, len);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    const size_t adj_len = min(len, buffer_size());
    for (size_t i = 0; i < adj_len; i++) {
        _buffer.pop();
    }
    _total_read += adj_len;
}

//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    const size_t adj_len = min(len, buffer_size());
    string str = peek_output(adj_len);
    pop_output(adj_len);
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

string ByteStream::concatenate_buffer() const {
    // Create a temporary queue and copy elements from the original buffer
    queue<char> temp_buffer = _buffer;
    string str;
    size_t size = buffer_size();
    // Read elements from the temporary queue
    for (size_t i = 0; i < size; i++) {
        str.push_back(temp_buffer.front());
        temp_buffer.pop();
    }
    return str;
}
