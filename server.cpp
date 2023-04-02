// A simple server in the internet domain using TCP
// The port nu1mber is passed as an argument

// Please note this is a C program
// It compiles without warnings with gcc

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#include <queue>
#include <fstream>


class Node
{
    private:
        char data;
        int freq;
        int id;
        std::string code;
        Node *left, *right;
    public:
        Node(char, int);

        int getFreq()
        {
            return freq;
        }

        int setFreq(int new_var)
        {
            return this->freq = new_var;
        }

        char getChar()
        {
            return data;
        }

        char setChar(char new_var)
        {
            return this->data = new_var;
        }

        int getID()
        {
            return id;
        }

        int setID(int new_var)
        {
            return this->id = new_var;
        }

        std::string getCode()
        {
            return code;
        }

        std::string setCode(std::string new_var)
        {
            return this->code = new_var;
        }

        Node* getLNode()
        {
            return left;
        }

        Node* setLNode(Node* new_var)
        {
            return this->left = new_var;
        }

        Node * getRNode()
        {
            return right;
        }

        Node* setRNode(Node* new_var)
        {
            return this->right = new_var;
        }
};



Node::Node(char data, int freq)
{
    this->data = data;
    this->freq = freq;
    left = right = nullptr;
}

class compare
{
    public:
        bool operator()(Node *left, Node *right)
        {
            if (left->getFreq() == right->getFreq())
            {
                if (left->getChar() == right->getChar())
                {
                    return (left->getID() < right->getID());
                }
                return (left->getChar() > right->getChar());
            }
            return (left->getFreq() > right->getFreq());
        }
};

class HFTree
{
    private:
        Node *root;
    public:
        HFTree(std::string);
        void display(Node*, std::string);
        Node* getRoot()
        {
            return root;
        }
};

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

HFTree::HFTree(std::string line)
{
    std::vector<std::pair<char, int>> parsedinfo;
    while(std::getline(std::cin, line))
    {
        char symbol = line[0];
        int freq = line[2]-'0';
        parsedinfo.emplace_back(std::make_pair(symbol, freq));
    }

    std::priority_queue<Node*, std::vector<Node*>, compare> pq;

    for(int i=0; i < parsedinfo.size(); i++)
    {
        pq.push(new Node(parsedinfo.at(i).first, parsedinfo.at(i).second));
    }

    int place = 0;
    while (pq.size() != 1)
    {
        Node *left = pq.top();
        pq.pop();

        Node *right= pq.top();
        pq.pop();

        Node *temp = new Node('\0', left->getFreq() + right->getFreq());
        temp->setLNode(left);
        temp->setRNode(right);
        temp->setID(place);
        place++;

        pq.push(temp);

    }

    this->root = pq.top();

    display(root, "");
}

void HFTree::display(Node *root, std::string str)
{
    if (!root)
    {
        return;
    }

    root->setCode(str);

    if (root->getChar() != '\0')
    {
        std::cout << "Symbol: " << root->getChar() << ", Frequency: " << root->getFreq() << ", Code: " << root->getCode() << std::endl;
    }

    display(root->getLNode(), str + "0");
    display(root->getRNode(), str + "1");
}




void fireman(int)
{
   while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[])
{
    std::string line;
    HFTree root = HFTree(line);
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    signal(SIGCHLD, fireman);
    if (argc < 2)
    {
        std::cerr << "ERROR, no port provided\n";
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket";
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR on binding";
        exit(1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (true)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (fork() == 0)
        {
            if (newsockfd < 0)
            {
                std::cerr << "ERROR on accept";
                exit(1);
            }
            int size;
            n = read(newsockfd, &size, sizeof(int));
            if (n < 0)
            {
                std::cerr << "ERROR reading from socket";
                exit(1);
            }
            char code[size];
            char *buffer = new char[size+1];
            bzero(buffer, size+1);
            n = read(newsockfd, buffer, size);
            if (n < 0)
            {
                std::cerr << "ERROR reading from socket";
                exit(1);
            }
            std::string recieved = buffer;
            Node* searched_symbol = search(recieved, root.getRoot(), 0);
            char symbol = searched_symbol->getChar();
            n = write(newsockfd, &symbol, 1);
            if (n < 0)
            {
                std::cerr << "ERROR writing to socket";
                exit(1);
            }
            close(newsockfd);
            delete[] buffer;
            _exit(0);
        }
    }
    close(sockfd);
    return 0;
}
