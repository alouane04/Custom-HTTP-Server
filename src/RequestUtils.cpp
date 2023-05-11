#include "../headers/ConfigFile.hpp"

string lineToParse(string key, string buffer){
    int pos;
    string res;

    pos = 0;
    if (!key.compare("first"))
    {
        pos = buffer.find("\r\n");
        res = buffer.substr(0,pos);
        return res;
    }

    while (buffer.at(0) != '\0')
    {
        pos = buffer.find("\r\n");
        res = buffer.substr(0,pos);
        if(res.find(key) != string::npos)
            return res;
        buffer.erase(0,pos+2);
    }
    return buffer;
}