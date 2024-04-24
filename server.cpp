#include <iostream>

#include "client_server.h"

int main(int argc, char* argv[]) {
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
  std::vector<std::shared_ptr<std::thread>> threads;
  while (true) {
    clientSocket =
        accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
    if (clientSocket < 0) {
      std::cerr << "Error: eccepting failed" << std::endl;
      exit(1);
    }
    threads.erase(std::remove_if(threads.begin(), threads.end(),
                                 [](const std::shared_ptr<std::thread>& t) {
                                   return t->joinable() && !t->joinable();
                                 }),
                  threads.end());

    // Создаем новый поток для обработки клиента
    std::shared_ptr<std::thread> clientThread = std::make_shared<std::thread>(
        handleClient, clientSocket, std::ref(channels));

    // Отсоединяем поток
    clientThread->detach();

    // Добавляем поток в список
    threads.push_back(clientThread);
  }

  // Здесь можно ожидать завершения оставшихся потоков, если это необходимо
  for (auto& t : threads) {
    if (t->joinable()) {
      t->join();
    }
    // std::thread clientThread(handleClient, clientSocket, std::ref(channels));
    // clientThread.detach();
  }
  close(serverSocket);
  return 0;
}
