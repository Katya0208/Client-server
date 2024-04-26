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

enum class ErrorType {
  CommandUsageRead,
  ChannelEmpty,
  UserNotFound,
  ChannelNotFound,
  UserAlreadyExists,
  CommandUsageSend,
  CommandUsageJoin,
  CommandUsageExit,
  WrongCommand,
  InvalidAddress,
  FailedSocketCreat,
  ConnectingFailed,
  BindingFailed,
  ChannelNameAlreadyExists,
  NicknameAlreadyExists,
  ErrorLenNickname,
  ErrorLenNameChannel,
  EcceptingFailed,
};

static std::map<ErrorType, std::string> errorMessages = {
    {ErrorType::CommandUsageRead, "Error command, usage: read <channel>"},
    {ErrorType::ChannelEmpty, "Channel is empty"},
    {ErrorType::UserNotFound, "This user was not found in the channel"},
    {ErrorType::ChannelNotFound,
     "There is no such channel yet, we are creating it..."},
    {ErrorType::UserAlreadyExists,
     "Error: this user is already in the channel"},
    {ErrorType::CommandUsageSend,
     "Error command, usage: send <channel> <message>"},
    {ErrorType::CommandUsageJoin, "Error command, usage: join <channel>"},
    {ErrorType::CommandUsageExit, "Error command, usage: exit <channel>"},
    {ErrorType::WrongCommand,
     "Error: The wrong command was entered. Use the commands from the list "
     "below:\n"},
    {ErrorType::InvalidAddress, "Error: invalid address"},
    {ErrorType::FailedSocketCreat, "Error: failed to create a socket"},
    {ErrorType::ConnectingFailed, "Error: connecting failed"},
    {ErrorType::BindingFailed, "Error: binding failed"},
    {ErrorType::ChannelNameAlreadyExists,
     "Such a channel already exists, choose another name"},
    {ErrorType::NicknameAlreadyExists,
     "This nickname is already in use, choose another one"},
    {ErrorType::ErrorLenNameChannel,
     "Error: the length of the channel name should not exceed 24 characters"},
    {ErrorType::ErrorLenNickname,
     "Error: nickname must not exceed 24 characters in length"},
    {ErrorType::EcceptingFailed, "Error: eccepting failed"}};

extern struct Sockets sockets;

void CreateClientSocket(int& clientSocket, int& port, char* address);

void CreateServerSocket(int& serverSocket, int& port);
std::string processRequest(std::string request, std::vector<Channel>& channels,
                           std::string nickname);

void handleClient(int clientSocket, std::vector<Channel>& channels);

void signalHandler(int signal);
