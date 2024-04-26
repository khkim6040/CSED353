Assignment 4 Writeup
=============

My name: Gwanho Kim

My POVIS ID: khkim6040

My student ID (numeric): 20190650

This assignment took me about 17 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [0.35, 0.36]

Program Structure and Design of the TCPConnection:
- Using skeleton code, I added some local variables to check time elapsed, state activeness and functions to send packet with header flags.
- I carefully set shutdown logic in tick() considering both passive and active close.

Implementation Challenges:
- It is really hard to set prerequisites for connection closing appropriately.
- I had to consider both passive and active close, and also had to consider the case when the connection is already closed.
- I had problem where random tests are failed every time, I finally found the issue and fixed it. The issue was reassembler's ending logic. 

Remaining Bugs:
- I guess the problem is that the first FIN sender doesn't know it's final ack is whether acknowledged or not. And it seems my code doesn't correctly handle the entire case.


- Optional: I had unexpected difficulty with: 
    - Implementing shutdown logic
    - active() state handling
    - Dealing with random test errors

- Optional: I think you could make this assignment better by: 
    - There is a typo in page 10, tomahawk lab4 pdf. If one of these steps is going **awry**

- Optional: I was surprised by: 
    - My code actually run in real network world!

- Optional: I'm not sure about: 
    - Correctness of my shotdown code
