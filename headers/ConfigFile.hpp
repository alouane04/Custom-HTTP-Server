#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include "Server.hpp"
#include <exception>
#include <algorithm>
#include "Client.hpp"
#include "ServerUtils.hpp"
#include <dirent.h>
#include <random>

using namespace std;

class ConfigFile{
    private:
        size_t size;
        vector<Server*> servers;
    public:
        ConfigFile();
        ConfigFile(const ConfigFile& other);
        ~ConfigFile();

        ConfigFile& operator=(const ConfigFile& rhs);

        Server *getServer(size_t index);
        void setServer();
        void run_servers();
        size_t getSize() const;
};

void            parseRequest(t_client& client, string buffer);
string          lineToParse(string key, string buffer);
void            makeResponse(t_client& client, Server server);
int             GetMethod(t_client& client, Server server);
void            sendResponse(const string& content, const string& content_type, const int error_code, const int client_fd);
const string    getErrorByCode(const int error_code);
string          getContentType(const string& path);
//size_t          getLocationIndex(string& req_path, Server& server);
string          generateRandomString(int length);
void            fillBody(t_client& client,string buffer);
void            chunkedToNormal(t_client& client, string buffer);
int             getRightLocation(string req_path, Server server);
Server          getRightServer(vector<Server *> Servers, t_client client);
string          getRightContent(int fd);
string          getRightRoot(Server server, int loc_pos);

#endif