#pragma once
#include <fstream>
#include <streambuf>
#include <map>
#include <string>
#include <iostream>
//#include <thread>
#include <string>
#include <cstring>
#include <unistd.h>
#include <regex>
#include <vector>
#include <SDL2/SDL.h>
#include <iostream>
#include <future>
#include <unistd.h>
#ifdef _WIN32
const std::string pathSeparator = "\\";
#else
const std::string pathSeparator = "/";
#endif

const Uint32 SDL_UPDATE_CONSOLE_OUTPUT = SDL_USEREVENT + 1;

enum class Platform
{
    Linux,
    Windows,
    Android,
    Web    
};




std::vector<std::string> get_result(std::string &input);
std::string run_command(const std::string &command);
std::string run_command_async(const std::string& command);
int run_process(const std::string &command, const std::string &path, bool use_pipes);
bool is_gcc_or_gpp_error(std::string &input);
std::string get_compiler_error(std::string &input);

bool isOutputMessage(const std::string &input);
std::string extractOutputErrorMessage(const std::string &input);

void replace_character(std::string &str, const std::string &target, const std::string &replacement);


std::string GetDirectoryPath(const std::string &filePath);
std::string GetFileExtension(const std::string &filePath);

bool IsFileExtension(const std::string &filePath, const std::string &extension);
bool HasFileExtension(const std::string &filePath);

std::string GetFileNameFromPath(const std::string &filePath);

std::string GetFileNameWithoutExtension(const std::string &filePath);
bool fileExists(const std::string &filename);
