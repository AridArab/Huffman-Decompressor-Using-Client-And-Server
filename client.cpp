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
    std::string code;
    std::vector<char> *message;
    std::vector<int> positions;

    
};

// Gets the positions for the symbol
std::vector<int> retrieve_pos(std::string entered)
{
    std::string curr;
    std::stringstream ss(entered);
    std::vector<int> returned;

    while (getline(ss, curr, ' '))
    {
        returned.push_back(std::stoi(curr));
    }
    
    return returned;
}

// Reads the lines from the input, parses them, and adds them to the package struct.
std::vector<std::string> retrieve_lines(std::vector<package*> &data, int &count, int &msgsize)
{
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(std::cin, line))
    {
        package *entry = new package();
        data.push_back(entry);
        data.at(count)->line = line;
        size_t pos_code = line.find_first_of(" ");
        data.at(count)->code = line.substr(0, pos_code);
        size_t pos_freq = line.find_last_of(" ");
        std::string last = line.substr(pos_freq + 1);
        std::string pos_unparsed = line.substr(pos_code + 1);
        data.at(count)->positions = retrieve_pos(pos_unparsed);
        int last_num = atoi(last.c_str());
        // Reads message size by getting a substring of the last number in each line, then compares it to find the biggest number
        if (last_num > msgsize)
        {
            msgsize = last_num;
        }
        count++;
    }

    return lines;
}

void *child_thread(void *void_ptr)
{
    package *pos_ptr = (package*)void_ptr;
    int sockfd, portno, n;
    // Converts the code string to a c string that can be sent to the server
    char sendmsg[pos_ptr->code.size() + 1];
    std::strcpy(sendmsg, pos_ptr->code.c_str());


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

    int sizeofmessage = pos_ptr->code.size();
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
    char recieved_symbol;
    n = read(sockfd, &recieved_symbol, sizeof(char));
    close(sockfd);

    // Places the character in the positions that were stated in the compressed file
    for (int i = 0; i < (int)pos_ptr->positions.size(); i++)
    {
        int position = pos_ptr->positions.at(i);
        pos_ptr->message->at(position) = recieved_symbol;
    }

    return nullptr;
}



int main(int argc, char *argv[])
{
    std::vector<std::string> inputed;
    std::vector<package*> datatobesent;
    int threads = 0;
    int msgsize = 1;
    inputed = retrieve_lines(datatobesent, threads, msgsize);
    std::vector<char> finalmsg(msgsize + 1);
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


    std::cout << "Original message: ";

    for (int i = 0; i < finalmsg.size(); i++)
    {
        std::cout <<  finalmsg.at(i);
    }
    
    std::cout << std::endl;

    return 0;
}
