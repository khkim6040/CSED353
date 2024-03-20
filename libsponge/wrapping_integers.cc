#include "wrapping_integers.hh"

#include <stdlib.h>

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint64_t MOD = 1ull << 32;
    return isn + n % MOD;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t MOD = 1ull << 32;
    int64_t nr = n.raw_value();
    int64_t isnr = isn.raw_value();

    // For modulo operation in C++
    if (nr < isnr)
        nr += MOD;

    // 32-bits
    uint64_t diff = nr - isnr;

    if (diff >= checkpoint) {
        return diff;
    }

    // |diff| .... |diff + k*MOD| ............ |checkpoint| ............. |diff + (k+1)*MOD|
    //                          |<- left_gap ->|          |<- right_gap ->|
    // left_gap + right_gap = MOD(=2^32)
    uint32_t left_gap = (checkpoint - diff) % MOD;
    uint32_t right_gap = MOD - left_gap;
    if (left_gap < right_gap) {
        return checkpoint - left_gap;
    } else {
        return checkpoint + right_gap;
    }
}
