#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#define BUFF_SIZE 1024
#define MAX_CLIENTS 10

struct Message {
  std::string nick;
  std::string message;
};

struct Channel {
  std::string name;
  std::vector<Message> msgs;
  std::vector<std::string> clients;
};

struct Sockets {
  int clientSocket;
  int serverSocket;
  int clientSocketOnServer;
};

Sockets sockets;

void CreateClientSocket(int& clientSocket, int& port, char* address) {
  struct sockaddr_in serverAddress;
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket < 0) {
    std::cerr << "Error: Error: failed to create a socket";
    exit(1);
  }
  memset((char*)&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  if (inet_pton(AF_INET, address, &(serverAddress.sin_addr)) != 1) {
    std::cerr << "Error: invalid address" << std::endl;
    exit(1);
  }
  // serverAddress.sin_addr.s_addr = inet_addr(address);
  if (connect(clientSocket, (struct sockaddr*)&serverAddress,
              sizeof(serverAddress)) < 0) {
    std::cerr << "Error: connecting failed" << std::endl;
    exit(1);
  }
}

void CreateServerSocket(int& serverSocket, int& port) {
  struct sockaddr_in serverAddress;
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  socklen_t serverLen = sizeof(serverAddress);
  if (serverSocket < 0) {
    std::cerr << "Error: failed to create a socket" << std::endl;
    exit(1);
  }
  memset((char*)&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  if (bind(serverSocket, (struct sockaddr*)&serverAddress, serverLen) < 0) {
    std::cerr << "Error: binding failed" << std::endl;
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
  if (words[0] == "read") {
    int statusChannel = 0;
    if (words.size() != 2) {
      return "Error command, use: read <channel>";
    }

    for (Channel channel : channels) {
      if (channel.name == words[1]) {
        statusChannel = 1;
        auto it =
            std::find(channel.clients.begin(), channel.clients.end(), nickname);
        if (it != channel.clients.end()) {
          // пользователь есть в данном канале
          if (channel.msgs.empty()) {
            return "Channel is empty";
          }
          //   for (Message pair : channel.msgs) {
          size_t numMessages = channel.msgs.size();
          size_t start = numMessages > 40 ? numMessages - 40 : 0;

          for (size_t i = start; i < numMessages; ++i) {
            const Message& pair = channel.msgs[i];
            answer += pair.nick + ": " + pair.message + '\n';
          }
          // answer += pair.nick + ": " + pair.message + '\n';
          //   }
        } else {
          return "Error: user " + nickname + " not found in the channel " +
                 channel.name;
        }
        break;
      }
    }
    if (statusChannel == 0) {
      answer = "There is no such channel yet, we are creating it...";
      Channel newChannel;
      newChannel.name = words[1];
      newChannel.clients.push_back(nickname);
      channels.push_back(newChannel);
    }
  } else if (words[0] == "send") {
    if (words.size() < 3) {
      return "Error command, use: send <channel> <message>";
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
          return "Error: user " + nickname + " not found in the channel " +
                 it->name;
        }
        break;
      }
    }
    if (statusChannel == 0) {
      answer = "There is no such channel yet, we are creating it...";
      Channel newChannel;
      newChannel.name = words[1];
      newChannel.clients.push_back(nickname);
      channels.push_back(newChannel);
    }
  } else if (words[0] == "join") {
    int statusChannel = 0;
    if (words.size() != 2) {
      return "Error command, use: join <channel>";
    }
    for (auto it = channels.begin(); it != channels.end(); ++it) {
      if (it->name == words[1]) {
        statusChannel = 1;
        std::vector<std::string>::iterator itClient =
            std::find(it->clients.begin(), it->clients.end(), nickname);
        if (itClient != it->clients.end()) {
          return "Error: this user is already in the channel";
        } else {
          it->clients.push_back(nickname);
          answer = "User " + nickname + " successfully added to the channel " +
                   words[1];
        }
      }
    }
    if (statusChannel == 0) {
      answer = "There is no such channel yet, we are creating it...";
      Channel newChannel;
      newChannel.name = words[1];
      newChannel.clients.push_back(nickname);
      channels.push_back(newChannel);
    }
  } else if (words[0] == "exit") {
    int statusChannel = 0;
    if (words.size() != 2) {
      return "Error command, usage: exit <channel>";
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
          return "The user " + nickname + " is not in the channel " + words[1];
        }
      }
    }
    if (statusChannel == 0) {
      answer = "There is no such channel yet, we are creating it...";
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
    answer =
        "Error: The wrong command was entered. Use the commands from the list "
        "below:\n";
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
      answer = "Such a channel already exists, choose another name";
    }
    for (const auto& client : it->clients) {
      if (client == nickname) {
        flag = 1;
        answer = "This nickname is already in use, choose another one";
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
