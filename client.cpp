// Please note this is a C program
// It compiles without warnings with gcc
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fstream>
#include <vector>
#include <queue>
#include <sstream>
#include <cstring>

struct package
{
    char* hostname;
    int port;
    std::string line;
    char symbol;
    std::vector<char> *message;

    
};



std::vector<std::string> retrieve_lines(std::string entered, std::vector<package*> &data, int &count, int &msgsize)
{
    std::ifstream file;
    std::string line;
    std::vector<std::string> lines;
    file.open(entered);
    while (std::getline(file, line))
    {
        package *entry = new package();
        data.push_back(entry);
        data.at(count)->line = line;
        size_t pos = line.find_last_of(" ");
        std::string last = line.substr(pos + 1);
        int last_num = atoi(last.c_str());
        if (last_num > msgsize)
        {
            msgsize = last_num;
        }
        count++;
    }

    file.close();

    return lines;
}

void *child_thread(void *void_ptr)
{
    package *pos_ptr = (package*)void_ptr;
    int sockfd, portno, n;
    char sendmsg[pos_ptr->line.size() + 1];
    std::strcpy(sendmsg, pos_ptr->line.c_str());


    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        std::cerr << "ERROR opening socket";
    server = gethostbyname(pos_ptr->hostname);
    if (server == NULL)
    {
        std::cerr << "ERROR, no such host\n";
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(pos_ptr->port);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR connecting";
        exit(1);
    }

    int sizeofmessage = pos_ptr->line.size();
    n = write(sockfd, &sizeofmessage, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR writing to socket";
        exit(1);
    }
    n = write(sockfd, sendmsg, sizeof(sendmsg));
    if (n < 0)
    {
        std::cerr << "ERROR writing to socket";
        exit(1);
    }
    int size;
    n = read(sockfd, &size, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR reading from socket";
        exit(1);
    }
    char *buffer = new char[size + 1];
    bzero(buffer, size + 1);
    n = read(sockfd, buffer, size);
    std::cout << buffer << std::endl;
    delete [] buffer;
    close(sockfd);
    
    return nullptr;
}



int main(int argc, char *argv[])
{
    std::string entered;
    std::cin >> entered;
    std::vector<std::string> inputed;
    std::vector<package*> datatobesent;
    int threads = 0;
    int msgsize = 0;
    inputed = retrieve_lines(entered, datatobesent, threads, msgsize);
    std::vector<char> finalmsg(msgsize);
    std::cout << "Size of final msg: " << finalmsg.size() << std::endl;
    pthread_t *tid = new pthread_t[threads];
    


    for (int i = 0; i < threads; i++)
    {
        datatobesent.at(i)->message = &finalmsg;
        datatobesent.at(i)->port = atoi(argv[2]);
        datatobesent.at(i)->hostname = argv[1];
        if(pthread_create(&tid[i], NULL, child_thread, (void*)datatobesent.at(i)))
        {
            fprintf(stderr, "Error occured\n");
            return 1;
        }
        wait(nullptr);
    }

    for (int i = 0; i < threads; i++)
    {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
