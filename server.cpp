#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <map>
#include <cmath>
#include <vector>
#include <sstream>
#include <iomanip>


void eq1(std::map<char, int> &freq, int currFreq, double currH/*prev h*/, char selectedTask, int extraFreq, double &H, int &NFreq);
std::string parse(std::string s);

void fireman(int)
{
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

struct task{
  char ch;
  int freq;
  task(char c, int f){
    ch = c; freq = f;
  }
};


int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno, clilen;
  struct sockaddr_in serv_addr, cli_addr;

  // Check the commandline arguments
  //filename, portNo
  if (argc != 2) { 
    std::cerr << "Port not provided" << std::endl;
    exit(0);
  }

  // Create the socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cerr << "Error opening socket" << std::endl;
    exit(0);
  }

  // Populate the sockaddr_in structure
  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  // Bind the socket with the sockaddr_in structure
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cerr << "Error binding" << std::endl;
    exit(0);
  }

  // Set the max number of concurrent connections
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);
  
  signal(SIGCHLD, fireman); 
  // Accept the incoming connection
  while (true) {
    // Accept a new connection
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
    
    if (newsockfd < 0) {
      std::cerr << "Error accepting new connections" << std::endl;
      exit(0);
    }
    
    int pid = fork();
    if (pid == 0) {
      int n, msgSize = 0;
      n = read(newsockfd, &msgSize, sizeof(int));
      if (n < 0) {
        std::cerr << "Error reading from socket" << std::endl;
        exit(0);
      }
      char *tempBuffer = new char[msgSize + 1];
      bzero(tempBuffer, msgSize + 1);
      n = read(newsockfd, tempBuffer, msgSize + 1);
      if (n < 0) {
        std::cerr << "Error reading from socket" << std::endl;
        exit(0);
      }
      
      std::string buffer = tempBuffer;
      delete[] tempBuffer;
      std::string temp = parse(buffer);
      msgSize = temp.size();
      
      n = write(newsockfd, &msgSize, sizeof(int));
      if (n < 0) {
        std::cerr << "Error writing to socket" << std::endl;
        exit(0);
      }
      n = write(newsockfd, temp.c_str(), msgSize);
      if (n < 0) {
        std::cerr << "Error writing to socket" << std::endl;
        exit(0);
      }
      close(newsockfd);
      _exit(0);
    } 
  }
  close(sockfd);
  return 0;
}


void eq1(std::map<char, int> &freq, int currFreq/*time instant=extraFreq added up*/, double currH/*prev entropy/H*/, char selectedTask, int extraFreq/*given*/, double &H, int &NFreq/*currFreq + extraFreq*/) {
  double currentTerm=0;
  NFreq = currFreq + extraFreq;
  if (NFreq == extraFreq) {
    H = 0;
    freq[selectedTask] += extraFreq;
  }
  else {
    if (freq[selectedTask] == 0) {
      currentTerm = 0;
    }
    else{
      currentTerm = freq[selectedTask] * log2(freq[selectedTask]);
    }
    double newTerm = (freq[selectedTask] + extraFreq) * log2(freq[selectedTask] + extraFreq);
    NFreq = static_cast<double>(NFreq);
    H = log2(NFreq) - ((log2(currFreq) - currH) * (currFreq) - currentTerm + newTerm) / NFreq;
    freq[selectedTask] += extraFreq;
  }

}

std::string parse(std::string s){

  std::map<char, int> map; //holds only chars w value 0 (freq array used in algorithm)
  std::vector<task> vect; //holds all user data
        
  //splitting string and storing into vector and map
  std::istringstream iss(s);
  char ch; int freq;
  while(iss >> ch >> freq){
    map[ch]=0;
    task temp(ch, freq); //create task
    vect.push_back(temp); //assing task to vector (of type task)
  }

  //for loop to call algorithm for each task
  std::vector<double> entropy; //vector to hold all entropy values
  double H=0; int nFreq=0;
  double currH = 0;
  int currFreq = 0;

  for(int i=0; i<vect.size(); i++){
    eq1(map, currFreq, currH, vect[i].ch, vect[i].freq, H, nFreq);
    entropy.push_back(H);
    currH = H;
    currFreq = nFreq;
  }

  //turning vector of entropies into a single string
  std::stringstream ss;
  for(size_t i = 0; i < entropy.size(); ++i)
  {
    if(i != 0)
      ss << " ";
    ss << std::fixed << std::showpoint;
    ss << std::setprecision(2);
    ss << entropy[i];
  }
  std::string results = ss.str();

  return results;
  
}
