#pragma once

#include<string>

unsigned int hash( const char* s, unsigned int seed);

int tohashable(int c);

std::string get_game_resource_name(std::string hashString, const std::string &append, const char * resource_root);
