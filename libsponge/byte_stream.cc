#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) {
    _capacity = capacity;
}

// Write a string of bytes into the stream. Write as many
// as will fit, and return the number of bytes written.
size_t ByteStream::write(const string &data) {
    size_t len = min(data.size(), remaining_capacity());
    for (int i = 0; i < len; i++) {
        buffer.push(data[i]);
    }
    return len;
}

// Peek at next "len" bytes of the stream
//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string str;
    // pop whole chars in queue
    while (!buffer.empty()) {
        str.push_back(buffer.front());
        buffer.pop();
    }
    // retrieve it
    for (char c : str) {
        str.push_back(c);
    }
    return str.substr(0, len);
}

// Remove ``len'' bytes from the buffer
//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    // for 0~len -> q.pop()
    for (int i = 0; i < len; i++) {
        buffer.pop();
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string str;
    // pop whole chars in queue
    for (int i = 0; i < len; i++) {
        str.push_back(buffer.front());
        buffer.pop();
    }
    return str;
}

// Signal that the byte stream has reached its ending
void ByteStream::end_input() {
    // set _is_ended
}

// `true` if the stream input has ended
bool ByteStream::input_ended() const {
    // ret _is_ended
}

// the maximum amount that can currently be peeked/read
size_t ByteStream::buffer_size() const {
    // return q.size()
}

// `true` if the buffer is empty
bool ByteStream::buffer_empty() const {
    // return size == 0
}

// `true` if the output has reached the ending
bool ByteStream::eof() const {
    // must get end signal and buffer(queue) is empty
    // input_ended() && buffer_empty()
}

// Total number of bytes written
size_t ByteStream::bytes_written() const {
    // total_written
}

// Total number of bytes popped
size_t ByteStream::bytes_read() const {
    // total_read
}

// Returns the number of additional bytes that the stream has space for
size_t ByteStream::remaining_capacity() const {
    return _capacity - buffer_size();
}
