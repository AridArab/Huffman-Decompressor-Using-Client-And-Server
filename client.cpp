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

/*
struct info
{
    HFTree tree;
    std::string line;
    std::string code;
    std::vector<char> *message;
    std::vector<std::pair<int, char>> decoded;
    std::vector<int> pos;
    info(const HFTree& tree, const std::string& line)
        : tree(tree), line(line)
    {}
};


std::vector<info*> retrieve_lines(std::string entered, int &count)
{
    std::ifstream file;
    std::string line;
    file.open(entered);
    while (std::getline(file, line))
    {
        info *entry = new info(tree, line);
        data.push_back(entry);
        count++;
    }

    file.close();

    return data;
}

std::string retrieve_code(std::string line)
{
    std::string returned;
    while(line[0] != ' ')
    {
        returned.push_back(line[0]);
        line.erase(0,1);
    }
    line.erase(0, 1);
    return returned;
}

std::vector<int> retrieve_pos(std::string line, std::vector<int> pos, int num)
{
    std::string curr;
    std::stringstream ss(line);
    while (getline(ss, curr, ' '))
    {
        pos.push_back(std::stoi(curr));
    }
    
    return pos;
}

Node* search(std::string code, Node* root, int count)
{
    if (!root)
    {
        return nullptr;
    }

    if (count == code.length())
    {
        return root;
    }


    switch (code[count])
    {
        case '0':
            return search(code, root->getLNode(), count+1);
        case '1':
            return search(code, root->getRNode(), count+1);
    }
    return nullptr;
}

info *make_pairs(info *&ptr, Node *node)
{
    for (int i = 0; i < ptr->pos.size(); i++)
    {
        ptr->decoded.push_back(std::make_pair(ptr->pos.at(i), node->getChar()));
    }

    return ptr;
}


void *decode(void *void_ptr)
{
    info *pos_ptr = (info*)void_ptr;
    pos_ptr->code = retrieve_code(pos_ptr->line);
    pos_ptr->line.erase(0,pos_ptr->code.length()+1);
    pos_ptr->pos = retrieve_pos(pos_ptr->line, pos_ptr->pos, pos_ptr->code.length());
    int count = 0;
    Node *symbol_node = search(pos_ptr->code, pos_ptr->tree.getRoot(), count);
    make_pairs(pos_ptr, symbol_node);
    for (int i = 0; i < pos_ptr->decoded.size(); i++)
    {
        int position = pos_ptr->decoded.at(i).first;
        pos_ptr->message->at(position) = pos_ptr->decoded.at(i).second;
    };
    
    return nullptr;
}

*/
int main(int argc, char *argv[])
{
    std::string entered;
    std::cin >> entered;
    int threads = 0;

    pthread_t *tid = new pthread_t[threads];
    std::ifstream file;

    /*
    for (int i = 0; i < threads; i++)
    {
        datatodecode.at(i)->message = &originalMSG;
        if(pthread_create(&tid[i], NULL, decode, (void*)datatodecode.at(i)))
        {
            fprintf(stderr, "Error occured\n");
            return 1;
        }
        wait(nullptr);
    }
    */
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3)
    {
        std::cerr << "usage " << argv[0] << "hostname port\n";
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        std::cerr << "ERROR opening socket";
    server = gethostbyname(argv[1]);
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
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR connecting";
        exit(1);
    }
    int sizeofmessage;
    std::cout << "Please input size of message: ";
    std::cin >> sizeofmessage;
    std::cout << "Please input message: ";
    
    char message[sizeofmessage];
    std::cin >> message;
    n = write(sockfd, &sizeofmessage, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR writing to socket";
        exit(1);
    }
    n = write(sockfd, message, sizeofmessage);
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
    return 0;
}
