#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include "Server.h"

struct Srv {
  int port;
  std::string hostName;
  std::string serverName;
  std::string errorPage;
  int maxBodySize;
  std::vector<Location> locations;
};

struct Loc {
  std::string url;
  std::string root;
//  std::vector<HttpMethod> allowMethod;
  std::vector<std::string> allowMethod;
  bool autoIndex;
  std::vector<std::string> index;
  std::string uploadPath;
  std::vector<std::string> cgiExt;
  std::string cgiPath;
  std::string errorPage;
};

class ConfigReader {
 public:
  ConfigReader() {
    Server srv;
    this->servers.push_back(srv);
  }

  ConfigReader(std::string const &path) : path(path) {
  }

  //ConfigReader(ConfigReader const &other){};

  //ConfigReader &operator=(ConfigReader const &other){};

  ~ConfigReader() {
    this->servers.clear();
  }

  void readConfig() {

    std::ifstream fileStream(this->path);

    if (fileStream.fail()) {
      throw std::runtime_error("Error reading config file. Exiting...");
    }

    std::string str;
    std::vector<std::string> data;

    while (std::getline(fileStream, str)) {
      if (removeSymbols(str)) {
        data.push_back(str);
      }
    }
    fileStream.close();
    setConfig(data);
  }

  bool removeSymbols(std::string &str) {

    while (str[0] == ' ' || str[0] == '\t')
      str.erase(0, 1);
    while (str[str.length() - 1] == ' ' || str[str.length() - 1] == '\t') // remove tabs and spaces before and after
      str.erase((str.length() - 1), 1);

    size_t doubleSpace = str.find('\t'); // remove tabs and double spaces inside string
    doubleSpace = str.find('\t');
    while (doubleSpace != std::string::npos) {
      str.replace(doubleSpace, 1, " ");
      doubleSpace = str.find('\t');
    }

    doubleSpace = str.find("  ");
    while (doubleSpace != std::string::npos) {
      str.erase(doubleSpace, 1);
      doubleSpace = str.find("  ");
    }

    if (str.length() == 0 || str[0] == '#' || str[0] == '\n')
      return false;
    return true;
  }

  void printData() {
    std::vector<Server>::iterator it = this->servers.begin();
    int i = 1;
    while (it != this->servers.end()) {
      std::cout << "\033[91m";
      int k = 1;
      std::cout << "Server #" << i << " config:" << std::endl;
      Server tmp = *it;
      std::cout << "Port: " << tmp.getPort() << std::endl;
      std::cout << "Hostname: " << tmp.getHostName() << std::endl;
      std::cout << "Server Name: " << tmp.getServerName() << std::endl;
      std::cout << "Error page: " << tmp.getErrorPage() << std::endl;
      std::cout << "Size limit: " << tmp.getBodySize() << std::endl << std::endl;

      std::vector<Location> loc = it->getLocations();
      std::vector<Location>::iterator lit = loc.begin();
      std::vector<std::string> vec;
      std::vector<std::string>::iterator vit;
      while (lit != loc.end()) {
        std::cout << "\033[93m";
        std::cout << "Server #" << i << " location #" << k << ":" << std::endl;
        Location ltmp = *lit;
        std::cout << "Url: " << ltmp.getUrl() << std::endl;
        std::cout << "Root: " << (ltmp.getRoot().length() > 0 ? ltmp.getRoot() : "NONE") << std::endl;
        std::cout << "Allowed methods: ";
        vec = ltmp.getMethods();
        vit = vec.begin();
        while (vit != vec.end()) {
          std::cout << *vit++ << " ";
        }
        if (vec.size() == 0) {
          std::cout << "NONE";
        }
        std::cout << std::endl;

        std::cout << "Autoindex: " << ltmp.getAutoindex() << " (1 = on, 0 = off)" << std::endl;
        std::cout << "Index: ";
        vec = ltmp.getIndex();
        vit = vec.begin();
        while (vit != vec.end()) {
          std::cout << *vit++ << " ";
        }
        if (vec.size() == 0) {
          std::cout << "NONE";
        }
        std::cout << std::endl;

        std::cout << "CGI ext: ";
        vec = ltmp.getCgiExt();
        vit = vec.begin();
        while (vit != vec.end()) {
          std::cout << *vit++ << " ";
        }
        if (vec.size() == 0) {
          std::cout << "NONE";
        }
        std::cout << std::endl;
        std::cout << "CGI path: " << (ltmp.getCgiPath().length() > 0 ? ltmp.getCgiPath() : "NONE") << std::endl;
        std::cout << "Error page: " << (ltmp.getErrorPage().length() > 0 ? ltmp.getErrorPage() : "NONE") << std::endl
                  << std::endl;
        k++;
        lit++;
      }
      std::cout << "\033[0m";
      std::cout << "===========================\n" << std::endl;
      i++;
      it++;

    }
  }

  const std::vector<Server> &getServers() const {
    return servers;
  }

 private:
  std::vector<std::string> strSplit(const std::string &text) {
    std::vector<std::string> res;
    size_t start = 0;
    size_t end = 0;
    while ((end = text.find(' ', start)) != std::string::npos) {
      res.push_back(text.substr(start, end - start));
      start = end + 1;
    }
    res.push_back(text.substr(start));
    return res;
  }

  void addServerData(Srv &srv, std::string &str) {
    std::vector<std::string> spl = strSplit(str);
    if (spl.front().compare("port") == 0) {
      int n = spl.back().length();
      char ch[n + 1];
      strcpy(ch, spl.back().c_str());
      srv.port = atoi(ch);
    } else if (spl.front().compare("limit_size") == 0) {
      int n = spl.back().length();
      char ch[n + 1];
      strcpy(ch, spl.back().c_str());
      srv.maxBodySize = atoi(ch);
    } else if (spl.front().compare("host") == 0) {
      srv.hostName = spl.back();
    } else if (spl.front().compare("server_name") == 0) {
      srv.serverName = spl.back();
    } else if (spl.front().compare("error_page") == 0) {
      srv.errorPage = spl.back();
    } else {
      throw std::runtime_error("Config file error: wrong server option. Exiting...");
    }
  }

  //todo
//  HttpMethod extractMethodFromStr(const std::string &method) {
//	if (method == "GET") {
//	  return GET;
//	} else if (method == "POST") {
//	  return POST;
//	} else if (method == "DELETE") {
//	  return DELETE;
//	}
//	throw std::runtime_error("Config file error: wrong server allowed method: " + method);
//  }

  void addLocationData(Loc &loc, std::string &str) {
    std::vector<std::string> spl = strSplit(str);
    if (spl.front().compare("location") == 0) {
      if (spl[1] == "{") {
        loc.url = "NONE";
      } else {
        loc.url = spl[1];
      }
    } else if (spl.front().compare("root") == 0) {
      loc.root = spl.back();
    } else if (spl.front().compare("allow_method") == 0) {
      while (spl.size() != 1) {
        if (spl.back() != "GET" && spl.back() != "POST" && spl.back() != "DELETE") {
          throw std::runtime_error("Config file error: wrong location option. Exiting...");
        }
//        loc.allowMethod.push_back(extractMethodFromStr(spl.back())); // add check for allowed
        loc.allowMethod.push_back(spl.back()); // add check for allowed
        spl.pop_back();
      }
    } else if (spl.front().compare("autoIndex") == 0) {
      spl.back() == "on" ? loc.autoIndex = true : loc.autoIndex = false;
    } else if (spl.front().compare("index") == 0) {
      while (spl.size() != 1) {
        loc.index.push_back(spl.back());
        spl.pop_back();
      }
    } else if (spl.front().compare("cgi_ext") == 0) {
      while (spl.size() != 1) {
        loc.cgiExt.push_back(spl.back());
        spl.pop_back();
      }
    } else if (spl.front().compare("cgi_path") == 0) {
      loc.cgiPath = spl.back();
    } else if (spl.front().compare("error_page") == 0) {
      loc.errorPage = spl.back();
    } else {
      throw std::runtime_error("Config file error: wrong location option. Exiting...");
    }
  }

  void addServer(int count, std::vector<std::string> data) {

    bool loc_bracket = false;
    std::vector<std::string>::iterator it = data.begin() + 1;
    int i = 0;
    Srv srv;
    srv.maxBodySize = 10000000;

    while (i < count - 1) {
      if (!loc_bracket && (*it).find("location") == std::string::npos && *it != "}") {
        addServerData(srv, *it);
      } else if ((*it).find("location") != std::string::npos) {
        loc_bracket = true;
        Loc loc;
        loc.autoIndex = false;

        addLocationData(loc, *it);
        it++;

        while ((*it).find("}") == std::string::npos) {
          addLocationData(loc, *it);
          i++;
          it++;
        }

        loc_bracket = false;
        srv.locations.push_back(Location(loc.url, loc.root, loc.allowMethod, loc.autoIndex,
                                         loc.index, loc.uploadPath, loc.cgiExt, loc.cgiPath, loc.errorPage));
        loc.allowMethod.clear();
        loc.index.clear();
        loc.cgiExt.clear();
        i++;
      }

      if ((*it == "}" && i == count - 2) || it == data.end() - 1) {
        break;
      }
      i++;
      it++;
    }
    this->servers.push_back(Server(srv.port, srv.hostName, srv.serverName,
                                   srv.errorPage, srv.maxBodySize, srv.locations));
  }

  void setConfig(std::vector<std::string> data) {

    while (data.size() > 0) {
      bool srv_bracket = false;
      bool loc_bracket = false;
      int count = 0;
      std::vector<std::string>::iterator it = data.begin();

      while (it != data.end()) {
        std::string str = *it;

        if (str.find("server {") != std::string::npos) {
          srv_bracket = true;
          count++;
        } else if (str.find("location") != std::string::npos) {
          loc_bracket = true;
          count++;
        } else if (str[0] == '}' && loc_bracket && srv_bracket) {
          loc_bracket = false;
          count++;
        } else if (srv_bracket && !loc_bracket && str[0] != '}') {
          count++;
        } else if (srv_bracket && loc_bracket) {
          count++;
        } else if (str[0] == '}' && !loc_bracket && srv_bracket) {
          srv_bracket = false;
          count++;
        }

        if (!loc_bracket && !srv_bracket) {
          break;
        }
        it++;
      }

      addServer(count, data);
      data.erase(data.begin(), data.begin() + count);
    }

    if (this->servers.size() == 0) {
      throw std::runtime_error("Config file error: no server data found. Exiting...");
    }
  }

 private:
  std::string path;
  std::vector<Server> servers;
};
