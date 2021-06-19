#pragma once
#include "Logger.h"
#include "Client.h"
#include "Location.h"
#include "StringBuilder.h"
#include "ServerStruct.h"

#include "SelectException.h"
#include "BadListenerFdException.h"
#include "NonBlockException.h"
#include "BindException.h"
#include "ListenException.h"
#include "AcceptException.h"
#include "ReadException.h"
#include "SendException.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/fcntl.h>
#include <errno.h>

#include <vector>
#include <string>

class Server {
 private:
  // tools
  static Logger LOGGER;
  // constants
  static const int TCP = 0;
  static const int BACKLOG = 5;
  // vars
  std::vector<Client> clients;
  int port;
  std::string hostName;
  std::string serverName;
  std::string errorPage;
  int maxBodySize;
  std::vector<Location> locations;
  int listenerFd;

 public:
  // todo?
  Server(int port = 8080,
		 const std::string &hostName = "localhost",
		 const std::string &serverName = "champions_server",
		 const std::string &errorPage = "html/404.html",
		 int maxBodySize = 100000000,
		 const std::vector<Location> &locations = std::vector<Location>())
      :
      port(port),
      hostName(hostName),
      serverName(serverName),
      errorPage(errorPage),
      maxBodySize(maxBodySize),
      locations(locations),
      listenerFd(-1) {

    if (locations.empty()) {
      Location loc = Location(1);
      this->locations.push_back(loc);
    }
  }

  virtual ~Server() {
  }

  Server(const Server &server) {
    operator=(server);
  }

  Server &operator=(const Server &server) {
    this->clients = server.clients;
    this->maxBodySize = server.maxBodySize;
    this->listenerFd = server.listenerFd;
    this->port = server.port;
    this->hostName = server.hostName;
    this->serverName = server.serverName;
    this->errorPage = server.errorPage;
    this->locations = server.locations;
    return *this;
  }

 private:
  static void setNonBlock(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
      throw NonBlockException(StringBuilder()
                                      .append(WebServException::FCNTL_ERROR)
                                      .append(" on fd: ")
                                      .append(fd)
                                      .toString());
    }
  }

  void createSocket() {
    listenerFd = socket(AF_INET, SOCK_STREAM, TCP);
    if (listenerFd == -1) {
      throw BadListenerFdException();
    }
    try {
      setNonBlock(listenerFd);
    } catch (const NonBlockException &e) {
      throw FatalWebServException("Non block exception failure on listener fd");
    }
  }

  void bindAddress() const {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (0 != bind(listenerFd, (struct sockaddr *) &addr, sizeof(addr))) {
      throw BindException();
    }
  }

  void startListening() {
    if (-1 == listen(listenerFd, BACKLOG)) {
      throw ListenException();
    }
  }

 public:
  int acceptConnection(int maxFd) {
    struct sockaddr addr;
    socklen_t socklen;
    int newClientFd;

    // if no connection, accept is blocking process and start waiting
    if ((newClientFd = accept(listenerFd, (struct sockaddr *) &addr, &socklen)) == -1) {
      throw AcceptException();
    }
    // set nonblock
    setNonBlock(newClientFd);

    Client newClient(newClientFd); //todo maybe on heap?
    clients.push_back(newClient);
    return std::max(maxFd, newClientFd);
  }

  int setFdSets(fd_set *readFds, fd_set *writeFds) {
    FD_ZERO(readFds);
    FD_ZERO(writeFds);
    FD_SET(listenerFd, readFds);

    int maxFd = listenerFd;
    for (std::vector<Client>::iterator it = clients.begin();
         it != clients.end(); ++it) {
      int fd = it->getFd();
      FD_SET(fd, readFds);
//      LOGGER.debug("SET READ FD: " + Logger::toString(fd));
      if (it->isReadyToWrite()) {
//        LOGGER.debug("SET WRITE FD: " + Logger::toString(fd));
        FD_SET(fd, writeFds);
      }
      maxFd = std::max(maxFd, fd);
    }

    return maxFd;
  }

  void printFds(fd_set *fds, const std::string &which) {
    std::stringstream ss;
    ss << which << " ";
    for (std::vector<Client>::iterator it = clients.begin();
         it != clients.end(); ++it) {
      int fd = it->getFd();
      if (FD_ISSET(fd, fds)) {
        ss << fd << " ";
      }
    }
    LOGGER.debug(ss.str());
  }

  void printReadFds(fd_set *readFds) {
    printFds(readFds, "Read fd set:");
  }

  void printWriteFds(fd_set *writeFds) {
    printFds(writeFds, "Write fd set:");
  }

  // todo maybe fd_set writeFds & timeout
  void processSelect() {
    fd_set readFds;
    fd_set writeFds;
    int maxFd = setFdSets(&readFds, &writeFds);
    printFds(&readFds, "Read fd set: ");
    printFds(&writeFds, "Write fd set: ");

	struct timeval time;
	time.tv_sec = 0;
	time.tv_usec = 10000;

	int selectRes;
    if ((selectRes = select(maxFd + 1, &readFds, &writeFds, NULL, &time)) == -1) {
      LOGGER.error("ERRNO: " + Logger::toString(errno));
      LOGGER.error(WebServException::SELECT_ERROR);
      std::cin >> selectRes;
      throw SelectException();
    }
    printFds(&readFds, "Read fds after select: ");
    printFds(&writeFds, "Write fds after select: ");
    if (selectRes == 0) {
      LOGGER.debug("SELECT RESULT: 0");
	  return;
    }

	// new connection
    if (FD_ISSET(listenerFd, &readFds)) {
      try {
        maxFd = acceptConnection(maxFd);
		LOGGER.info("Client connected, fd: " + Logger::toString(maxFd));
      } catch (const RuntimeWebServException &e) {
        LOGGER.error(e.what());
      }
    }

    for (std::vector<Client>::iterator client = clients.begin();
         client != clients.end(); ++client) {
      if (FD_ISSET(client->getFd(), &writeFds)) {
        for (std::vector<Request>::iterator request = client->getRequests().begin();
             request != client->getRequests().end();) {
          LOGGER.info("Start sending to client: " + Logger::toString(maxFd));

          Response response(*request, createServerStruct());
          std::string responseBody = response.generateResponse();
          if (send(client->getFd(), responseBody.c_str(), responseBody.length(), 0) == -1) {
			LOGGER.error(StringBuilder()
			                          .append(WebServException::SEND_ERROR)
			                          .append(" fd: ")
			                          .append(client->getFd())
			                          .append(", response body: ")
			                          .append(responseBody)
			                          .toString());
			throw SendException();
          }
          if (response.getStatus() == INTERNAL_SERVER_ERROR) {
            client->setClientStatus(CLOSED);
            break;
          }
          request = client->getRequests().erase(request);
        }
      }

      if (client->getClientStatus() == CLOSED) {
        client = clients.erase(client);
      }

      if (FD_ISSET(client->getFd(), &readFds)) {
        try {
          client->processReading();
        } catch (const RuntimeWebServException &e) {
          LOGGER.error(e.what());
        }
      }
    }
  }

  void run() {
    // 1. create listenerFd
    createSocket();
    // 2. make port not busy for the next use
    int YES = 1;
    setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &YES, sizeof(int));
    // 3. bind
    bindAddress();
    // 4. listen
    // check that port is listening:
    // netstat -a -n | grep LISTEN
    startListening();
  }

  int getSocketFd() const {
    return listenerFd;
  }

  int getPort() const {
    return this->port;
  }

  std::string getHostName() const {
    return this->hostName;
  }

  std::string getServerName() const {
    return this->serverName;
  }

  std::string getErrorPage() const {
    return this->errorPage;
  }

  long int getBodySize() const {
    return this->maxBodySize;
  }

  std::vector<Location> getLocations() const {
    return this->locations;
  }

 private:
  ServerStruct createServerStruct() const {
    return ServerStruct(port, hostName, serverName, maxBodySize, locations);
  }
};

Logger Server::LOGGER(Logger::DEBUG);
