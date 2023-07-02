//Server implementiaton
//
//Assumptions and Implementation
//   -Establishing a TCP connection
//   -Will create a service thread to receive from client how many repetitions
//       it will perform
//   -Each repetition will send 1500 bytes, service thread will read each
//   -Service thread writes back how many read calls it performed
//
//----------------------------Server----------------------------------------------
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
#include <pthread.h>
using namespace std;

const int BUFF_SIZE = 1500;
const int NUM_CONNECTIONS = 5;
pthread_mutex_t mutex;

void *service(void *arg);

int main(int argc, char *argv[])
{

    //argument validation
    if(argc != 2) {
        cerr << "Usage: port" << endl;
        return -1;
    }

    //build address
    int port = atoi(argv[1]);
    sockaddr_in acceptSocketAddress;
    bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    acceptSocketAddress.sin_family = AF_INET;
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    acceptSocketAddress.sin_port = htons(port);

    //open socket and bind
    int server_SD = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    setsockopt(server_SD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    cout << "Socket #: " << server_SD << endl;

    int rc = bind(server_SD, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    if (rc < 0)
    {
        cerr << "Bind Failed" << endl;
    }


    while(1){
        //listen and accept
        listen(server_SD, NUM_CONNECTIONS);
        sockaddr_in newSockAddr;
        socklen_t newSockAddrSize = sizeof(newSockAddr);
        int new_SD = accept(server_SD, (sockaddr *) &newSockAddr, &newSockAddrSize);
        cout << "Accepted Socket #: " << new_SD <<endl;
        //servicing thread
        pthread_t service_thread;
        pthread_create(&service_thread, NULL, service, (void*)&new_SD);
        pthread_join(service_thread, NULL);
    }

    return 0;
}

//Service thread start routine, the only arg passed is new_SD socket
void *service(void *arg) {
        pthread_mutex_lock(&mutex);
        int new_SD = *((int*) arg);
        char data_buf[BUFF_SIZE];
        bzero(data_buf, BUFF_SIZE);

        //reading how many repetitions client will perform
        char rep_client[5] ;
        int rep_client_read = read(new_SD, rep_client, sizeof(rep_client));
        int repetition = atoi(rep_client);

        //reading data from socket
        int read_calls = 0;
        for(int i = 0; i < repetition; i++) {
                int bytes_read = BUFF_SIZE;
                 while(bytes_read > 0) { //making sure we read BUFF_SIZE amount
                         int current  =  read(new_SD, data_buf, bytes_read);
                         bytes_read -= current;
                         read_calls++;
                 }
         }
        //writing to socket how many read calls
         string t = to_string(read_calls);
         char const *read_call_buf = t.c_str();
         int read_calls_bytes = write(new_SD, read_call_buf, sizeof(read_call_buf));

         //closing connection, unlocking mutex, terminating service thread
         close(new_SD);
         pthread_mutex_unlock(&mutex);
         pthread_exit(NULL);
         return nullptr;
}

