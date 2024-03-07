Assignment 1 Writeup
=============

My name: [Gwanho Kim]

My POVIS ID: [khkim6040]

My student ID (numeric): [20190650]

This assignment took me about [6.5] hours to do(including the time on studying(1h), designing(2h), and writing the code(3.5h)).

Program Structure and Design of the StreamReassembler:
[Consider not to use the queue, deque, or list. Because the substring can be inserted in the middle of the stream.  
Instead, I used vector to implement the stream reassembler despite of its passive resizing panalty]

Implementation Challenges:
[Initialize the stream vector with -1 or MAX initailly to simply avoid overlapping. But I checked that tests gave randomly generated data. Thus, I decided to use another vector(which is _is_allocated) to check allocation.]

Remaining Bugs:
[vector resizing problem. It is only valid when the length of whole data is less than 2^32 - 1 (=4GB).]

- Optional: I had unexpected difficulty with: [Make appropriate data handle logic, espaecially for the edge cases(cap.cc, holes.cc) inserting overlapping substring]

- Optional: I think you could make this assignment better by: [Introduce more edge test cases]

- Optional: I was surprised by: [Tests even consider randomly generated data]

- Optional: I'm not sure about: [Whether the vector is the best data structure to implement the stream reassembler. Whether I handled all of the edge cases]
