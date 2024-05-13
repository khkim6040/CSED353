Assignment 6 Writeup
=============

My name: Gwanho Kim

My POVIS ID: khkim6040

My student ID (numeric): 20190650

This assignment took me about 3 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router: 
- Initially, I used unordered_map<pair, pair> to store the routing information as I did in the network_interface lab. 
- But I realized that I set hash function for pair, which is difficult and not efficient.
- So I changed the data structure to vector to store everything at once.

Implementation Challenges:
- Considering when longest prefix length is 0, I had to handle it separately.

Remaining Bugs:
- I think my code couldn't work with add_route for the same prefixes but different next hops. 

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: Beauty of abstraction.

- Optional: I'm not sure about: [describe]
