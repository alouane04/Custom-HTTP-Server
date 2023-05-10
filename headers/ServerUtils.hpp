#ifndef SERVERUTILS_HPP
#define SERVERUTILS_HPP
#include <iostream>
#include <exception>
#include "Server.hpp"

unsigned int    ft_inet_addr(std::string str);
void            Accept(vector<pollfd>& fds, t_client& clients, size_t i);


#endif