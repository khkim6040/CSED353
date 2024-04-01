Assignment 3 Writeup
=============

My name: Gwanho Kim

My POVIS ID: khkim6040

My student ID (numeric): 20190650

This assignment took me about 7 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
- I exploited queue as a buffer to store the data that needs to be sent.
- Also, I introduced Timer class which is responsible to handle the timeout event.

Implementation Challenges:
- How to set check_point to unwrap ackno and seqno. I used recent absolute ackno(=_recent_abs_ackno) as a check_point.
- Tracking outstanding buffer size. I manually tracked the size of the buffer by introducing correspoinding size variable.
- When to start&reset&stop the timer.
- Considering SYN, FIN flags in the segment.

Remaining Bugs:


- Optional: I had unexpected difficulty with: When fill_window() is called. "Do I have to call fill_window() in every method?"

- Optional: I think you could make this assignment better by: I think it might be better to move consecutive_retransmission_count to the Timer class. This way, the Timer class can handle the retransmission count and the timeout event at the same time.

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: I'm not sure that setting recent abs ackno as a checkpoint is correct.  
