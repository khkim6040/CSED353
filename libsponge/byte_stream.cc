#include "byte_stream.hh"

#include <iostream>
// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _buffer(), _capacity(capacity) {}

// Write a string of bytes into the stream. Write as many
// as will fit, and return the number of bytes written.
size_t ByteStream::write(const string &data) {
    // adjusted length
    const size_t adj_len = min(data.length(), remaining_capacity());
    std::cout << data << " " << data.length() << ' ' << remaining_capacity() << '\n';
    for (size_t i = 0; i < adj_len; i++) {
        _buffer.push(data[i]);
    }
    _total_written += adj_len;
    return adj_len;
}

// Peek at next "len" bytes of the stream
//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    std::string str = concatenate_buffer();
    return str.substr(0, len);
}

// Remove ``len'' bytes from the buffer
//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    const size_t adj_len = min(len, buffer_size());
    for (size_t i = 0; i < adj_len; i++) {
        _buffer.pop();
    }
    _total_read += adj_len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    const size_t adj_len = min(len, buffer_size());
    std::string str = peek_output(adj_len);
    pop_output(adj_len);
    return str;
}

// Signal that the byte stream has reached its ending
void ByteStream::end_input() {
    _is_ended = true;
}

// `true` if the stream input has ended
bool ByteStream::input_ended() const {
    return _is_ended;
}

// the maximum amount that can currently be peeked/read
size_t ByteStream::buffer_size() const {
    return _buffer.size();
}

// `true` if the buffer is empty
bool ByteStream::buffer_empty() const {
    return buffer_size() == 0;
}

// `true` if the output has reached the ending
bool ByteStream::eof() const {
    // must get end signal and buffer(queue) is empty
    return input_ended() && buffer_empty();
}

// Total number of bytes written
size_t ByteStream::bytes_written() const {
    return _total_written;
}

// Total number of bytes popped
size_t ByteStream::bytes_read() const {
    return _total_read;
}

// Returns the number of additional bytes that the stream has space for
size_t ByteStream::remaining_capacity() const {
    return _capacity - buffer_size();
}

string ByteStream::concatenate_buffer() const {
    // Create a temporary queue and copy elements from the original buffer
    std::queue<char> temp_buffer = _buffer;
    std::string str;
    size_t size = buffer_size();
    // Read elements from the temporary queue
    for (size_t i = 0; i < size; i++) {
        str.push_back(temp_buffer.front());
        temp_buffer.pop();
    }
    return str;
}
