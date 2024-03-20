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
    uint64_t divider = 1ul << 32;
    return WrappingInt32(uint32_t((n % divider + isn.raw_value()) % divider));
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
    int64_t nn = n.raw_value();
    int64_t isnn = isn.raw_value();

    // uint64_t diff = n.raw_value() > isn.raw_value() ? n.raw_value() - isn.raw_value() : isn.raw_value() - n.raw_value();
    uint64_t pre;
    if (nn - isnn < 0)
        pre = (1ull << 32) - (isnn - nn) % (1ull << 32);
    else
        pre = (nn - isnn) % (1ull << 32);

    uint64_t pre_gap = pre > checkpoint ? pre - checkpoint : checkpoint - pre;
    for (uint64_t cur = pre + (1ull << 32); cur < UINT64_MAX; cur += (1ull << 32)) {
        uint64_t cur_gap = cur > checkpoint ? cur - checkpoint : checkpoint - cur;
        if (cur_gap >= pre_gap) {
            break;
        } else {
            pre = cur;
            pre_gap = cur_gap;
        }
    }

    // cout << n.raw_value() << " " << isn.raw_value() << " " << checkpoint << " " << pre << endl;
    return pre;
}
