#include <iostream>

#include "client_server.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    exit(1);
  }
  int serverSocket, clientSocket;
  int port;
  std::vector<Channel> channels;
  port = std::atoi(argv[1]);
  struct sockaddr_in clientAddress;
  socklen_t clientLen = sizeof(clientAddress);

  CreateServerSocket(serverSocket, port);

  while (true) {
    clientSocket =
        accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen);
    if (clientSocket < 0) {
      std::cerr << "Error: eccepting failed" << std::endl;
      exit(1);
    }
    std::thread clientThread(handleClient, clientSocket, std::ref(channels));
    clientThread.detach();
  }
  close(clientSocket);
  close(serverSocket);
  return 0;
}
