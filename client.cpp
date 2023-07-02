//Client implementation
//
//Assumptions and Implementation
//   -Establishing a TCP connection
//   -Client sends server how many repetitions it will perform
//   -Each repetition will send 1500 bytes
//   -Client should receive how many socket read() calls the server performed
//   -Prints out time the test takes (usec), # of server read calls, and
//      throughput (Gbps)

//---------------------------------Client---------------------------------------
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
using namespace std;

const int BUFF_SIZE=1500;

int main(int argc, char *argv[])
{
    //argument validation
    if (argc != 7)
    {
       cerr << "Usage: server_name port repetition n_bufs buf_size type" << endl;
       return -1;
    }

    //assigning arguments into variables
    char* server_name = argv[1];
    char server_port[6];
    char* port = argv[2];
    for(int i = 0; i < 6; i++) {
            server_port[i] = port[i];
    }
    int repetition = atoi(argv[3]);
    int n_bufs = atoi(argv[4]);
    int buf_size = atoi(argv[5]);
    int type = atoi(argv[6]);
    
    
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int client_SD = -1;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;  //allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = 0; // optional Options
    hints.ai_protocol = 0; // allow any protocol
    //get info about server being used
    int rc = getaddrinfo(server_name, server_port, &hints, &result);
    if (rc != 0)
    {
       cerr << "ERROR: " << gai_strerror(rc) << endl;
       exit(EXIT_FAILURE);
    }

    // iterate through addresses and connect
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        client_SD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (client_SD == -1)
                {
            continue;
        }
        //socket has been created
        rc = connect(client_SD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0)
        {
            cerr << "Connection Failed" << endl;
            close(client_SD);
            return -1;
        }
else    //success with connection
        {
            break;
        }
    }

    if (rp == NULL)
    {
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Client Socket: " << client_SD << endl;
    }
    freeaddrinfo(result);

    //sending over # of repetitions to server
    int rep_sent_bytes = write(client_SD, argv[3], sizeof(argv[3]));

    //allocating and filling data_buf
    char** data_buf = new char*[n_bufs];
    for(int i = 0; i < n_bufs; i++) {
            data_buf[i] = new char[buf_size];
    }
    for (int i = 0; i < n_bufs ; i++)
    {
        for(int j = 0; j < buf_size; j++) {
            data_buf[i][j] = 'z';
        }
    }

   auto start = chrono::steady_clock::now();
    for(int k = 0; k < repetition; k++) {
         if(type == 1) { //multiple writes
                for(int i = 0; i < n_bufs; i++) {
                      int written = write(client_SD, data_buf[i], buf_size);
                }
         } else if(type == 2) { //writev
                struct iovec vector[n_bufs];
                for (int i = 0; i < n_bufs; i++) {
                vector[i].iov_base = data_buf[i];
                vector[i].iov_len = buf_size;
        }
                int written = writev(client_SD, vector, n_bufs);
         } else { //type 3 single write
                int written1 = write(client_SD, data_buf, n_bufs * buf_size);
         }
    }
    auto end = chrono::steady_clock::now();
    int time = chrono::duration_cast<chrono::microseconds>(end-start).count();
    //divided by 1000 for throughput because divided with microseconds not seconds
    double throughput = ((double)(repetition * n_bufs * buf_size * 8)/(double)time)/1000.0;
    //reading number of server read calls
    char server_read[10];
    int bytes_read = read(client_SD, server_read, 10);
    cout << "Test " << type << ": " << "time = " << time  << " usec, #reads = " << server_read << ", throughput = " << throughput << " Gbps" << endl;

    for(int i = 0; i < n_bufs; i++) {
            delete[] data_buf[i];
    }
    delete[] data_buf;
    close(client_SD);

    return 0;
}
