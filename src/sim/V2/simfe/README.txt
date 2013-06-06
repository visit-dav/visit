                                  SIMFE
                   Brad Whitlock, Thu Jun  6 14:45:34 PDT 2013
                   ============================================

Simfe stands for "sim front-end" and it is a program that listens on a socket
for inbound simulation connections from VisIt. It acts like an in-between 
socket relay for simulations and might be useful for when you want to connect
to a simulation that is not capable of communicating to the outside world. You
run your simulation and it writes a sim2 file. Then you run simfe and give it 
the sim2 file, which it reads and it writes a new sim2 file that you can open.
The new sim2 file contains information that lets VisIt connect to simfe, which
then forms a socket connection to the original simulation.

This program might not be useful. It's hard to say. I'm checking it in anyway 
so as to not lose it. The program is not currently built.
