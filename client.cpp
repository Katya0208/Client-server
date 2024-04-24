#include <iostream>

#include "client_server.h"

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <ip-adress server> <port> <channel>"
              << std::endl;
    exit(1);
  }

  int port;
  std::string channel;
  std::string nickPlusChannel;
  char buffer[BUFF_SIZE];
  port = std::atoi(argv[2]);
  channel = argv[3];
  if (channel.length() > 24) {
    std::cerr << "Error: длина имени канала не должна превышать 24 символа"
              << std::endl;
    exit(1);
  }
  CreateClientSocket(sockets.clientSocket, port, argv[1]);

  std::string nickname;
  std::cout << "Введите ваш nickname: ";
  getline(std::cin, nickname);

  while (nickname.size() > 24) {
    std::cerr << "Error: длина nickname не должна превышать 24 символа"
              << std::endl;
    getline(std::cin, nickname);
  }
  nickPlusChannel = nickname;
  nickPlusChannel += ' ';
  nickPlusChannel += argv[3];
  const char* charNickPlusChannel = nickPlusChannel.c_str();
  send(sockets.clientSocket, (const void*)charNickPlusChannel, BUFF_SIZE, 0);
  recv(sockets.clientSocket, buffer, BUFF_SIZE, 0);
  std::string strBuffer1(buffer);
  if (strBuffer1 == "OK") {
    while (true) {
      std::string message;
      getline(std::cin, message);
      const char* charMessage = message.c_str();
      send(sockets.clientSocket, (const void*)charMessage, BUFF_SIZE, 0);
      recv(sockets.clientSocket, buffer, BUFF_SIZE, 0);
      std::string strBuffer(buffer);
      std::cout << strBuffer << std::endl;
      std::cout << std::endl;
    }
    close(sockets.clientSocket);
  } else {
    std::cout << buffer << std::endl;
  }

  close(sockets.clientSocket);
  return 0;
}