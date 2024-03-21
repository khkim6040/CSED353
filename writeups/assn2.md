Assignment 2 Writeup
=============

My name: __Gwanho Kim__

My POVIS ID: __khkim6040__

My student ID (numeric): __20190650__

This assignment took me about __6 hours__(2hours to read, 4hours to implement) to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
- The core of the TCPReceiver is translating the wrapped seqno into the correct stream index. So I used SYN, FIN related member variables to handle the wrapped seqno.
- In `wrapping_integers.cc`, I used modulo operation to wrap and unwrap the seqno. Notable thing is that I was able to find appropriate unwrap value by using `left_gap` and `right_gap`, not by using any __loop.__


Implementation Challenges:
- I had a hard time to see the differences between the TCP receiver, the reassembler and the bytestream
- I struggled to find correct unwrap efficiently.
- It was hard to handle seqno, abs_seqno and stream index at the same time.
- When does receiver know the ackno should be abs_seqno of FIN? I solved this by introducing the member variable `_FIN_abs_seqno`.  

Remaining Bugs:
- The seg contains only FIN or SYN. I'm not sure that I handled it correctly as an unavailable case in `segment_received()`. 

- Optional: I had unexpected difficulty with: How to deal with SYN, FIN when translating the seqno into the stream index.

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: Differences between TCP receiver, reassembler and bytestream regarding their roles and responsibilities.
