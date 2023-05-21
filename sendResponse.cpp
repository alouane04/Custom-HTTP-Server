#include "headers/ConfigFile.hpp"
#include "headers/Client.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <algorithm>

/*
There are many different types of errors that can occur when handling a GET request. Here are some examples:

400 Bad Request: The request is malformed and cannot be understood by the server.
401 Unauthorized: The client is not authenticated and cannot access the requested resource.
403 Forbidden: The client is authenticated, but does not have permission to access the requested resource.
404 Not Found: The requested resource could not be found on the server.
405 Method Not Allowed: The requested resource does not support the HTTP method used in the request (e.g. GET vs POST).
406 Not Acceptable: The client requested a content type that is not supported by the server.
500 Internal Server Error: The server encountered an unexpected error while processing the request.

These are just a few examples of the many possible errors that can occur during a GET request.
In general, it's a good idea to handle errors gracefully by returning an appropriate error status code
and a human-readable error message in the response body.*/

string getContentType(const string& path)
{
    if (path.rfind(".html") != string::npos)
        return ("text/html");

    else if (path.rfind(".css") != string::npos)
        return ("text/css");

    else if (path.rfind(".js") != string::npos)
        return ("text/javascript");

    else if (path.rfind(".png") != string::npos)
        return ("image/png");

    else if (path.rfind(".jpg") != string::npos)
        return ("image/jpg");

    else if (path.rfind(".gif") != string::npos)
        return ("image/gif");

    else if (path.rfind(".svg") != string::npos)
        return ("image/svg+xml");

    else if (path.rfind(".pdf") != string::npos)
        return ("application/pdf");

    else if (path.rfind(".zip") != string::npos)
        return ("application/zip");

    else if (path.rfind(".gz") != string::npos)
        return ("application/gzip");

    else if (path.rfind(".mp3") != string::npos)
        return ("audio/mpeg");

    else if (path.rfind(".mp4") != string::npos)
        return ("video/mp4");

    else if (path.rfind(".mkv") != string::npos)
        return ("video/x-matroska");

    else if (path.rfind(".xml") != string::npos)
        return ("application/xml");

    else if (path.rfind(".json") != string::npos)
        return ("application/json");

    else
        return ("application/octet-stream");
}

const string    getErrorByCode(const int error_code)
{
    switch (error_code)
    {
        case 200:
            return (" OK");
        case 404:
            return (" Page Not Found");
        case 500:
            return (" Internal Server Error");
        case 501:
            return (" Not Implemented");
        case 502:
            return (" Bad Gateway");
        case 503:
            return (" Service Unavailable");
        case 504:
            return (" Gateway Timeout");
        default:
            return (" Unknown Error");
    }
}

void    sendResponse(const string& content, const string& content_type, const int error_code, const int client_fd)
{
    string response;
    stringstream ss;

    ss << "HTTP/1.1 " << error_code << getErrorByCode(error_code) << "\r\n";
    ss << "Content-Type: " << content_type << "\r\n";
    ss << "Content-Length: " << content.size() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << content;
    response = ss.str();
    write(client_fd, response.c_str(), response.size());
}

int  getRightLocation(string req_path, Server server)
{
    string  loc_path;
    int temp = 0;
    int j = -1;
    int count = 0;

    for (size_t i = 0; i < server.getSize(); i++)
    {
        loc_path = server.getLocation(i)->getPath();
        if (loc_path.compare("/"))
            loc_path.append("/");

        for (size_t z = 0; z < req_path.size() && z < loc_path.size() ; z++)
        {
            if (req_path[z] == loc_path[z])
            {
                if (req_path[z] == '/')
                    count++;
            }
            else
            {
                count = 0;
                break;
            }
        }

        if(temp < count)
        {
            temp = count;
            j = i;
            count = 0;
        }
    }

    return (j);
}

string  getRightRoot(Server server, int loc_pos)
{
    if (server.getLocation(loc_pos)->getRoot().size())
       return (server.getLocation(loc_pos)->getRoot());
    else if (server.getValue("root").size())
        return (server.getValue("root"));
    else
        throw runtime_error("403 forbiden");
}

string  getRightContent(int fd)
{
    int r = 1;
    char buffer[1024];
    string content;

    while (r != 0)
    {
        bzero(buffer, 1024);
        r = read(fd, buffer, 1023);
        content.append(buffer, r);
    }
    return (content);
}

int    GetMethod(t_client& client, Server server)
{
    string req_path;
    string path_to_serve;
    string test_file;
    int fd;

    req_path = client.request["path"].substr(0, req_path.find("?"));
    int loc_pos = getRightLocation(req_path, server);
    if (loc_pos == -1)
            throw runtime_error("error 403 forbiden");
    path_to_serve = getRightRoot(server, loc_pos);
    if (req_path.substr(server.getLocation(loc_pos)->getPath().size()).size() == 0)
    {
    //////////////////------ get right index file ------////////////////////
        test_file = path_to_serve;
        for (size_t i = 0; i < server.getLocation(loc_pos)->getIndexSize(); i++)
        {
            fd = open(test_file.append("/").append(server.getLocation(loc_pos)->getIndex(i)).c_str(), O_RDONLY);
            if (fd != -1)
                    return (sendResponse(getRightContent(fd), getContentType(server.getLocation(loc_pos)->getIndex(i)), 200, client.new_sock_fd), 0);
            test_file.substr(0, test_file.find_last_of("/"));
        }
        throw runtime_error("error 404 not found"); 
    }
    else
    {
        ////////////////------  check file   ------//////////////////
        if (server.getLocation(loc_pos)->getPath().compare("/"))
            path_to_serve = path_to_serve.append(req_path.substr(server.getLocation(loc_pos)->getPath().size()));
        else
            path_to_serve = path_to_serve.append(req_path);

        fd = open(path_to_serve.c_str(), O_RDONLY);
        if (fd != -1)
            return (sendResponse(getRightContent(fd), getContentType(req_path), 200, client.new_sock_fd), 0);
        throw runtime_error("error 404 not found");
    }
}

void    PostMethod(t_client& client, Server& server)
{
    string filename;
    std::ofstream postfile;
    int L = 0;

    cout << "WE ARE IN POST METHOD" << endl;
    //check the path of the Request and if post is allowed
    L = getRightLocation(client.request["path"], server);
    cout << L << endl;
    for (size_t i = 0; i < server.getLocation(L)->getAllowedMethod().size(); i++)
        if (server.getLocation(L)->getAllowedMethod().at(i).compare("POST"))
            if (i == server.getLocation(L)->getAllowedMethod().size() - 1)


    if (!   count(server.getLocation(L)->getAllowedMethod().begin(),server.getLocation(L)->getAllowedMethod().end(),"POST"))
    {
            //error METHOD NOT ALLOWED
    }
    //<-------------------------------->
    //get the file name and create it
    filename = client.request["path"].substr(client.request["path"].find_last_of('/', string::npos));
    if(filename.size() == 0)
        filename = generateRandomString(10) + "." +client.request["Content-Type"];
    filename = getRightRoot(server, L) + server.getLocation(L)->get_upload_dir() + filename;
    cout << server.getLocation(L)->get_upload_dir() << endl;
    cout<< filename <<endl;
    //<-------------------------------->
    //open the file and write the body to it
    postfile.open(filename);
    if(postfile)
        postfile << client.body;
    else
        // error failed to open file
        return ;
}

void    DeleteMethod(t_client& client, Server server)
{
    string filename;
    std::ofstream postfile;
    struct dirent *pDirent;
    DIR *dir;
    int L = 0;
    //check the path of the Request and if post is allowed
    L = getRightLocation(client.request["path"], server);
    /*if(!count(server.getLocation(L)->getAllowedMethod().begin(),server.getLocation(L)->getAllowedMethod().end(),"DELETE"))
    {
            //error METHOD NOT ALLOWED
    }*/
    filename = getRightRoot(server, L) + client.request["path"].substr(server.getLocation(L)->getPath().size());
    dir = opendir(filename.c_str());
    if(dir != NULL)
    {
        while((pDirent = readdir(dir)) != NULL)
        {
            remove((filename +'/'+ pDirent->d_name).c_str());
        }
    }
    remove(filename.c_str());
}

void    makeResponse(t_client& client, Server server)
{
    string method;

    method = client.request["method"];
    if (method == "GET")
        GetMethod(client, server);
    else if (method == "POST")
        PostMethod(client, server);
    else if (method == "DELETE")
        DeleteMethod(client, server);
    //else
        // error code 405 method not allowed
}

Server getRightServer(vector<Server *> servers, t_client client)
{
    string host = client.request["host"];
    string port = client.request["port"];

    if (servers.size() == 1)
        return (*servers[0]);

    if (!host.size() || !port.size())
        throw runtime_error("no host or port available to check");
    
    for (size_t i = 0; i < servers.size(); i++)
        if (servers[i]->getValue("host") == host && servers[i]->getValue("listen") == port)
            return (*servers[i]);

    throw runtime_error("no server found");
}
