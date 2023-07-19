#pragma once


#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <iostream>


std::string read_file_into_string(const char* filename);
void write_string_into_file(const std::string& contents, const char* filename);

