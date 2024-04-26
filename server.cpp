#include <iostream>

#include "client_server.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    exit(1);
  }
  int port;
  std::vector<Channel> channels;
  port = std::atoi(argv[1]);
  struct sockaddr_in clientAddress;
  socklen_t clientLen = sizeof(clientAddress);
  signal(SIGINT, signalHandler);

  CreateServerSocket(sockets.serverSocket, port);
  std::vector<std::shared_ptr<std::thread>> threads;
  while (true) {
    sockets.clientSocketOnServer = accept(
        sockets.serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
    if (sockets.clientSocketOnServer < 0) {
      std::cerr << errorMessages[ErrorType::EcceptingFailed] << std::endl;
      exit(1);
    }
    threads.erase(std::remove_if(threads.begin(), threads.end(),
                                 [](const std::shared_ptr<std::thread>& t) {
                                   return t->joinable() && !t->joinable();
                                 }),
                  threads.end());

    std::shared_ptr<std::thread> clientThread = std::make_shared<std::thread>(
        handleClient, sockets.clientSocketOnServer, std::ref(channels));

    clientThread->detach();

    threads.push_back(clientThread);
  }

  for (auto& t : threads) {
    if (t->joinable()) {
      t->join();
    }
  }
  close(sockets.serverSocket);
  return 0;
}
