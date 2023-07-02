Wrote a Client.cpp and Server.cpp that establishes a TCP connection and sends buffers of data from the client to server.  
The client sends a message to the server which contains the number of iterations it will perform (each iteration sends 1500
bytes of data).  When the server has read the full set of data from the client it will send an acknowledgement message back which
includes the number of socket read() calls performed.

The client sends data over in three possible ways: multiple writes (invokes write() for each buffer), writev (allocates array 
of iovec data structures, each having its *iov_base field point to a different data buffer as well as storing the buffer size in
its iov_len field; calls writev() to send all data buffers at once, single write (allocates an array of data buffers the size of 
the number of data buffers, and calls write() to send the array at once.

Client program also measures how long each of the scenarios take and prints out the information.
Client.cpp takes 6 arguments: serverName, port, repetition, nbufs (number of data buffers), bufsize, type (of transfer scenario)

Main thread in server runs in a dispatch loop with accepts a new connection and creates a thread to service the request.
Server program takes 1 argument: a server TCP port

Build script:
g++ client.cpp -o client
g++ server.cpp -pthread -o server

