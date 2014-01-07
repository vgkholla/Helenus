HELENUS
=======

Helenus is an in-memory distributed key-value store like Cassandra

Features:

->	Uses hash partitioning

->	Maintains membership lists through gossiping (allows joins, leaves and failures to be communicated to everyone)

->	Supports 4 database operations - insert, delete, update and lookup, where keys and values are strings

->	Tolerates upto 2 simultaneous failures (re-replicates and can tolerate 2 simultaneous failures again)

->	Supports 3 consistency levels, ONE, QUORUM and ALL

->	Supported by a logging system (which can be queried with the client available at BuildStages/MP1)

Other details:

->	This system was built in stages as part of the Distributed Systems course at the UIUC in Fall 2013 (CS 425)

->	The BuildStages folder has the 4 stages we built it in. So it has a lot of testing files and results too.

->	The Helenus folder has the final system with a README explaning how to get the system working.

->	The system was designed and implemented by Gopalakrishna Holla (vgkholla) and Alok Tiagi (aloktiagi).

->	The specification was provided by the instructors of CS 425 led by Dr. Indranil Gupta (http://www.cs.uiuc.edu/~indy/).

Implementation details:

Written in C++, uses pthread, boost and socket libraries
