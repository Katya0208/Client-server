#include "client_server.hpp"

#include <iostream>

Sockets sockets;
void CreateClientSocket(int& clientSocket, int& port, char* address) {
  struct sockaddr_in serverAddress;
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket < 0) {
    std::cerr << errorMessages[ErrorType::FailedSocketCreat];
    exit(1);
  }
  memset((char*)&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  if (inet_pton(AF_INET, address, &(serverAddress.sin_addr)) != 1) {
    std::cerr << errorMessages[ErrorType::InvalidAddress] << std::endl;
    exit(1);
  }
  // serverAddress.sin_addr.s_addr = inet_addr(address);
  if (connect(clientSocket, (struct sockaddr*)&serverAddress,
              sizeof(serverAddress)) < 0) {
    std::cerr << errorMessages[ErrorType::ConnectingFailed] << std::endl;
    exit(1);
  }
}

void CreateServerSocket(int& serverSocket, int& port) {
  struct sockaddr_in serverAddress;
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  socklen_t serverLen = sizeof(serverAddress);
  if (serverSocket < 0) {
    std::cerr << errorMessages[ErrorType::FailedSocketCreat] << std::endl;
    exit(1);
  }
  memset((char*)&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  if (bind(serverSocket, (struct sockaddr*)&serverAddress, serverLen) < 0) {
    std::cerr << errorMessages[ErrorType::BindingFailed] << std::endl;
    exit(1);
  }
  listen(serverSocket, MAX_CLIENTS);
}

std::string processRequest(std::string request, std::vector<Channel>& channels,
                           std::string nickname) {
  std::string answer;
  std::istringstream iss(request);
  std::vector<std::string> words{std::istream_iterator<std::string>{iss},
                                 std::istream_iterator<std::string>{}};
  ErrorType error;
  if (words[0] == "read") {
    int statusChannel = 0;
    if (words.size() != 2) {
      return errorMessages[ErrorType::CommandUsageRead];
      // return "Error command, use: read <channel>";
    }

    for (Channel channel : channels) {
      if (channel.name == words[1]) {
        statusChannel = 1;
        auto it =
            std::find(channel.clients.begin(), channel.clients.end(), nickname);
        if (it != channel.clients.end()) {
          // пользователь есть в данном канале
          if (channel.msgs.empty()) {
            return errorMessages[ErrorType::ChannelEmpty];
          }

          size_t numMessages = channel.msgs.size();
          size_t start = numMessages > 40 ? numMessages - 40 : 0;

          for (size_t i = start; i < numMessages; ++i) {
            const Message& pair = channel.msgs[i];
            answer += pair.nick + ": " + pair.message + '\n';
          }

        } else {
          return errorMessages[ErrorType::UserNotFound];
          // return "Error: user " + nickname + " not found in the channel " +
          channel.name;
        }
        break;
      }
    }
    if (statusChannel == 0) {
      answer = errorMessages[ErrorType::ChannelNotFound];
      ;
      Channel newChannel;
      newChannel.name = words[1];
      newChannel.clients.push_back(nickname);
      channels.push_back(newChannel);
    }
  } else if (words[0] == "send") {
    if (words.size() < 3) {
      return errorMessages[ErrorType::CommandUsageSend];
      // return "Error command, use: send <channel> <message>";
    }
    int statusChannel = 0;
    std::string msg = "";
    for (auto it = channels.begin(); it != channels.end(); ++it) {
      if (it->name == words[1]) {
        statusChannel = 1;
        std::vector<std::string>::iterator itClient =
            std::find(it->clients.begin(), it->clients.end(), nickname);
        if (itClient != it->clients.end()) {
          // юзер есть в этом канале
          for (int i = 2; i < words.size(); i++) {
            msg += words[i];
            msg += " ";
          }
          Message mssg;
          mssg.nick = nickname;
          mssg.message = msg;
          it->msgs.push_back({mssg.nick, mssg.message});
          answer = "Message " + msg + " sent successfully";
        } else {
          return errorMessages[ErrorType::UserNotFound];
        }
        break;
      }
    }
    if (statusChannel == 0) {
      answer = errorMessages[ErrorType::ChannelNotFound];
      ;
      Channel newChannel;
      newChannel.name = words[1];
      newChannel.clients.push_back(nickname);
      channels.push_back(newChannel);
    }
  } else if (words[0] == "join") {
    int statusChannel = 0;
    if (words.size() != 2) {
      return errorMessages[ErrorType::CommandUsageJoin];
      // return "Error command, use: join <channel>";
    }
    for (auto it = channels.begin(); it != channels.end(); ++it) {
      if (it->name == words[1]) {
        statusChannel = 1;
        std::vector<std::string>::iterator itClient =
            std::find(it->clients.begin(), it->clients.end(), nickname);
        if (itClient != it->clients.end()) {
          return errorMessages[ErrorType::UserAlreadyExists];
          // return "Error: this user is already in the channel";
        } else {
          it->clients.push_back(nickname);
          answer = "User " + nickname + " successfully added to the channel " +
                   words[1];
        }
      }
    }
    if (statusChannel == 0) {
      answer = errorMessages[ErrorType::ChannelNotFound];
      ;
      Channel newChannel;
      newChannel.name = words[1];
      newChannel.clients.push_back(nickname);
      channels.push_back(newChannel);
    }
  } else if (words[0] == "exit") {
    int statusChannel = 0;
    if (words.size() != 2) {
      return errorMessages[ErrorType::CommandUsageExit];
      // return "Error command, usage: exit <channel>";
    }
    for (auto it = channels.begin(); it != channels.end(); ++it) {
      if (it->name == words[1]) {
        statusChannel = 1;
        std::vector<std::string>::iterator itClient =
            std::find(it->clients.begin(), it->clients.end(), nickname);
        if (itClient != it->clients.end()) {
          it->clients.erase(itClient);
          answer = "User " + nickname +
                   " successfully deleted from the channel " + words[1];
        } else {
          return errorMessages[ErrorType::UserNotFound];
          // return "The user " + nickname + " is not in the channel " +
          // words[1];
        }
      }
    }
    if (statusChannel == 0) {
      answer = errorMessages[ErrorType::ChannelNotFound];
      Channel newChannel;
      newChannel.name = words[1];
      newChannel.clients.push_back(nickname);
      channels.push_back(newChannel);
    }
  } else if (words[0] == "commands") {
    answer = "A list of commands that you can enter:\n";
    answer += "\n";
    answer += "send <channel> <message>\n";
    answer += "read <channel>\n";
    answer += "join <channel>\n";
    answer += "exit <channel>\n";
    answer += "commands";
  } else {
    answer = errorMessages[ErrorType::WrongCommand];
    answer += "\n";
    answer += "send <channel> <message>\n";
    answer += "read <channel>\n";
    answer += "join <channel>\n";
    answer += "exit <channel>\n";
    answer += "commands";
  }
  return answer;
}

void handleClient(int clientSocket, std::vector<Channel>& channels) {
  int flag = 0;
  Channel ch;
  char buffer[BUFF_SIZE];
  char nickname[24], channel[24];
  int msg_read = recv(clientSocket, buffer, BUFF_SIZE, 0);
  sscanf(buffer, "%s %s", nickname, channel);
  std::string answer;
  answer = "OK";
  for (auto it = channels.begin(); it != channels.end(); ++it) {
    if (it->name == channel) {
      flag = 1;
      answer = errorMessages[ErrorType::ChannelNameAlreadyExists];
    }
    for (const auto& client : it->clients) {
      if (client == nickname) {
        flag = 1;
        answer = errorMessages[ErrorType::NicknameAlreadyExists];
      }
    }
  }
  const char* charCheckOk = answer.c_str();

  send(clientSocket, (const void*)charCheckOk, BUFF_SIZE, 0);
  if (flag == 0) {
    std::cout << "Присоединился: " << nickname << " на канале " << channel
              << std::endl;
    ch.name = channel;
    ch.clients.push_back(nickname);
    channels.push_back({ch.name, ch.msgs, ch.clients});
    while (true) {
      int msg_read = recv(clientSocket, buffer, BUFF_SIZE, 0);
      if (msg_read <= 0) {
        exit(1);
      }
      std::string request(buffer);
      std::string answer = processRequest(request, channels, nickname);
      const char* charAnswer = answer.c_str();
      send(clientSocket, (const void*)charAnswer, BUFF_SIZE, 0);
      buffer[msg_read] = '\0';
    }
    close(clientSocket);
  }
}

void signalHandler(int signal) {
  if (signal == SIGINT) {
    close(sockets.clientSocket);
    close(sockets.clientSocketOnServer);
    close(sockets.serverSocket);
    std::cout << "exiting..." << std::endl;
    exit(EXIT_SUCCESS);
  }
}