#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "byte_stream.hh"

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

//! \brief A class that assembles a series of excerpts from a byte stream (possibly out of order,
//! possibly overlapping) into an in-order byte stream.
class StreamReassembler {
  private:
    // Your code here -- add private members as necessary.
    vector<char> _stream;                     //!< Use STL vector to store and reassemble substring
    vector<bool> _is_allocated;               //!< Tell whether a position of _stream is allocated to avoid overlap
    ByteStream _output;                       //!< The reassembled in-order byte stream
    size_t _capacity = 0;                     //!< The maximum number of bytes
    size_t _unassembled_count = 0;            //!< Number of unassembled bytes
    size_t _next_read_point = 0;              //!< Indicate next to the already read point, which also stands the first unassembled point in the stream
    size_t _first_unread_point = 0;           //!< Indicate first unread point
    bool _ignore_flag = false;                //!< Tell whether any unacceptable byte occurred
    bool _is_first_unread_point_set = false;  //!< Used to set _first_unread_point each time data comes in
    bool _is_eof = false;                     //!< Used to remember whether true value of argument eof was passed

  public:
    //! \brief Construct a `StreamReassembler` that will store up to `capacity` bytes.
    //! \note This capacity limits both the bytes that have been reassembled,
    //! and those that have not yet been reassembled.
    StreamReassembler(const size_t capacity);

    //! \brief Receive a substring and write any newly contiguous bytes into the stream.
    //!
    //! The StreamReassembler will stay within the memory limits of the `capacity`.
    //! Bytes that would exceed the capacity are silently discarded.
    //!
    //! \param data the substring
    //! \param index indicates the index (place in sequence) of the first byte in `data`
    //! \param eof the last byte of `data` will be the last byte in the entire stream
    void push_substring(const std::string &data, const uint64_t index, const bool eof);

    //! \name Access the reassembled byte stream
    //!@{
    const ByteStream &stream_out() const { return _output; }
    ByteStream &stream_out() { return _output; }
    //!@}

    //! The number of bytes in the substrings stored but not yet reassembled
    //!
    //! \note If the byte at a particular index has been pushed more than once, it
    //! should only be counted once for the purpose of this function.
    size_t unassembled_bytes() const;

    //! \brief Is the internal state empty (other than the output stream)?
    //! \returns `true` if no substrings are waiting to be assembled
    bool empty() const;

    //! \brief Return the size of the stream
    size_t stream_size() const;

    //! \brief Set eof flag to true
    //! \details This function is called when eof argument is true
    void set_eof();

    //! \brief Handle stream eof
    //! \details If eof flag is true and current stream is empty, end input of output stream
    //! \note If there was any ignored byte, stream eof will be never set
    void handle_stream_eof();

    //! \brief Resize vectors to obtain data
    //! \details Resize _stream and _is_allocated vectors
    //! \param data_length the length of data
    //! \note This function is called when data position exceeds stream size during not exceeding stream limit
    void resize_vectors(const size_t &data_length);

    //! \brief Push data into stream
    //! \details This function is called when data comes in
    //! \param data same as the data argument of push_substring
    //! \param index same as the index argument of push_substring
    //! \param stream_limit_index the last index of the stream
    //! \param data_length the length of data
    void handle_data(const string &data,
                     const size_t &index,
                     const size_t &stream_limit_index,
                     const size_t &data_length);

    //! \brief Consider the data and assemble it
    //! \details Write any newly contiguous bytes into the _output stream
    void assemble_data();
    size_t get_next_read_point() const { return _next_read_point; }
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
