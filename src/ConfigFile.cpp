#include "../headers/ConfigFile.hpp"

ConfigFile::ConfigFile(){
    cout << "ConfigFile default constructor called" << endl;
    size = 0;
}

ConfigFile::~ConfigFile(){
    cout << "ConfigFile destructor called" << endl;
    servers.clear();
}

Server ConfigFile::getServer(size_t index) const{
    if (index > size)
        throw out_of_range("ConfigFile::getServer");
    return(servers[index]);
}

size_t ConfigFile::getSize() const{
    return (size);
}

void ConfigFile::setServer(Server &srv){
    servers.push_back(srv);
    size++;
}