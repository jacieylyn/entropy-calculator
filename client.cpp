#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <pthread.h>
#include <sstream>


struct threadData{
  std::string inputData;
  std::string entropies;
  int port;
  const char *server;
};

struct task{
  char ch;
  int freq;
  task(char c, int f){
    ch = c; freq = f;
  }
};

void *middleman(void *arg_pointer);
void print(int CPU, std::vector<task> vect, std::string entropy);

int main(int argc, char *argv[]) {

  //filename, IP(localhost), portNo
  if (argc != 3) {
    std::cerr << "usage " << argv[0] << " hostname port" << std::endl;
    exit(0);
  }

  int portno;
  const char *server;
  portno = atoi(argv[2]);
  server = argv[1];
  
  //inputing data
  std::vector<threadData> threadDataVect; //size of vect depends on # strings

  std::string str;
  while(getline(std::cin, str)){
    if(str.empty()) //if cin is empty (user input has ended), stop calling cin
      break;
    threadData thread1; //creating struct
    thread1.inputData = str; //inserting input string into struct
    thread1.server = server;
    thread1.port = portno;
    threadDataVect.push_back(thread1); //inserting struct into vector
  }

  //threading
  const int NUMTHREADS = threadDataVect.size(); 
  pthread_t tid[NUMTHREADS];
  for(int i=0; i< NUMTHREADS; i++){ 
    pthread_create(&tid[i], nullptr, middleman, &threadDataVect[i]); 
  }
  /* wait/join threads */
  for(int i=0; i< NUMTHREADS; i++){ 
      pthread_join(tid[i], nullptr);
  }

  //printing
  for(int i=0; i<threadDataVect.size(); i++){
    std::vector<task> parsedData;
    //splitting string again...
    std::istringstream stream(threadDataVect[i].inputData);
    char ch; int freq;
    while(stream >> ch >> freq){
      task temp(ch, freq);
      parsedData.push_back(temp);
    }

    print(i+1, parsedData, threadDataVect[i].entropies);
  }
  
  return 0;
}

void *middleman(void *arg_pointer){

  //casting arg_pointer
  threadData *data = (threadData *)arg_pointer;
  
  int sockfd, portno, n;
  std::string buffer;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  portno = data->port; //atoi(data->argv[2])
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cerr << "ERROR opening socket" << std::endl;
    exit(0);
  }
  server = gethostbyname(data->server);
  if (server == NULL) {
    std::cerr << "ERROR, no such host" << std::endl;
    exit(0);
  }
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  
  //connecting to server
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cerr << "ERROR connecting" << std::endl;
    exit(0);
  }
  
  buffer = data->inputData;  
  int msgSize = sizeof(buffer);
  
  //writing size of buffer to server
  n = write(sockfd, &msgSize, sizeof(int));
  if (n < 0) {
    std::cerr << "ERROR writing to socket" << std::endl;
    exit(0);
  }
  //writing buffer to server
  n = write(sockfd, buffer.c_str(), msgSize);
  if (n < 0) {
    std::cerr << "ERROR writing to socket" << std::endl;
    exit(0);
  }
  
  n = read(sockfd, &msgSize, sizeof(int));
  if (n < 0) {
    std::cerr << "ERROR reading from socket" << std::endl;
    exit(0);
  }
  char *tempBuffer = new char[msgSize + 1];
  bzero(tempBuffer, msgSize + 1);
  n = read(sockfd, tempBuffer, msgSize);
  if (n < 0) {
    std::cerr << "ERROR reading from socket" << std::endl;
    exit(0);
  }
  
  buffer = tempBuffer;
  delete[] tempBuffer;
  data->entropies = buffer; 
  close(sockfd);

  return nullptr;
}


void print(int CPU, std::vector<task> vect, std::string entropy){
  bool first = true;
  std::cout << "CPU " << CPU << "\n";
  std::cout << "Task scheduling information: ";
  for(auto &a: vect){
    if(!first)
      std::cout << ", ";
    std::cout << a.ch << "(" << a.freq << ")";
    first = false;
  }
  std::cout << std::endl;
  std::cout << "Entropy for CPU " << CPU << std::endl;
  //entropy info
  std::cout << entropy << std::endl;
  std::cout << std::endl;
}