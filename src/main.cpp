#include "utils.hpp"
#include "process.hpp"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "template.hpp"
#include "TextEditor.h"

#include "Async.hpp"
#include <SDL2/SDL.h>
#include "portable-file-dialogs.h"

#if defined(_WIN32)
#include <direct.h>    // Required for: _getch(), _chdir()
#define GETCWD _getcwd // NOTE: MSDN recommends not to use getcwd(), chdir()
#define CHDIR _chdir
#include <io.h> // Required for: _access() [Used in FileExists()]
#else
#include <unistd.h> // Required for: getch(), chdir() (POSIX), access()
#define GETCWD getcwd
#define CHDIR chdir
#endif

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

#include "nlohmann/json.hpp"
using json = nlohmann::json;

bool ChangeDirectory(const char *dir)
{
    bool result = CHDIR(dir);

    return (result == 0);
}
#define MAX_FILEPATH_LENGTH 256
const char *GetWorkingDirectory(void)
{
    static char currentDir[MAX_FILEPATH_LENGTH] = {0};
    memset(currentDir, 0, MAX_FILEPATH_LENGTH);

    char *path = GETCWD(currentDir, MAX_FILEPATH_LENGTH - 1);

    return path;
}

const char *GetApplicationDirectory(void)
{
    static char appDir[MAX_FILEPATH_LENGTH] = {0};
    memset(appDir, 0, MAX_FILEPATH_LENGTH);

#if defined(_WIN32)
    int len = 0;
#if defined(UNICODE)
    unsigned short widePath[MAX_PATH];
    len = GetModuleFileNameW(NULL, widePath, MAX_PATH);
    len = WideCharToMultiByte(0, 0, widePath, len, appDir, MAX_PATH, NULL, NULL);
#else
    len = GetModuleFileNameA(NULL, appDir, MAX_PATH);
#endif
    if (len > 0)
    {
        for (int i = len; i >= 0; --i)
        {
            if (appDir[i] == '\\')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
    }
    else
    {
        appDir[0] = '.';
        appDir[1] = '\\';
    }

#elif defined(__linux__)
    unsigned int size = sizeof(appDir);
    ssize_t len = readlink("/proc/self/exe", appDir, size);

    if (len > 0)
    {
        for (int i = len; i >= 0; --i)
        {
            if (appDir[i] == '/')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
    }
    else
    {
        appDir[0] = '.';
        appDir[1] = '/';
    }
#elif defined(__APPLE__)
    uint32_t size = sizeof(appDir);

    if (_NSGetExecutablePath(appDir, &size) == 0)
    {
        int len = strlen(appDir);
        for (int i = len; i >= 0; --i)
        {
            if (appDir[i] == '/')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
        else
        {
            appDir[0] = '.';
            appDir[1] = '/';
        }
#endif

    return appDir;
}


void setFontWithSize(const char *fontPath, float fontSize)
{
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF(fontPath, fontSize);
    io.FontDefault = io.Fonts->AddFontFromFileTTF(fontPath, fontSize);
}

std::string fileToEdit = ""; // "src/TextEditor.cpp";
std::string lastPath = "";
std::string modulesPath = "/media/djoker/data/code/projectos/SDLBuilder/modules";

std::string lastFileName = "";
std::string lastFileExtension = "";
std::string lastFile = "";
std::string lastFileOpen = "";

std::string compilerFlags = "";
std::string linkingFlags = "";

std::string compilerLinuxFlags = "";
std::string linkingLinuxFlags = "";

std::string compilerAndroidFlags = "";
std::string linkingAndroidFlags = "";

std::string compilerWebFlags = "";
std::string linkingWebFlags = "";
std::string shellWeb = "";
bool useShellWeb = false;

std::string emskPath = "/media/djoker/data/emsdk/upstream/emscripten";
std::string androidSdkPath = "";
std::string androidNdkPath = "";
std::string javaPath = "";
std::string rootPath = "";

static char compileBuffer[256] = "";
static char buildBuffer[256] = "";

static char compileAndroidBuffer[256] = "";
static char buildAndroidBuffer[256] = "";

static char compileWebBuffer[256] = "";
static char buildWebBuffer[256] = "";
static char shellWebBuffer[256] = "";
static char compileLinuxBuffer[256] = "";
static char buildLinuxBuffer[256] = "";

bool useRaylib = false;
bool useSDL = false;
bool useGLFW = false;

bool isClang = false;
bool isDebug = true;
bool isCpp = false;
bool isSave = true;
bool canSave = true;
bool canRun = false;
bool canBuild = true;
bool autoSave = false;
bool showConsole = true;
bool showOptions = false;
bool noKeys = true; // para testar se as teclas foram todas libertadas, dummy :(
Uint32 lastSave = 0;
Uint32 saveTimer = 5000;
int Window_Width = 1280;
int Window_Height = 920;
Platform current_platform = Platform::Linux;
static std::string consoleText = "@DjokerSoft 2023";

void setBuildFlags(const std::string &string)
{
    linkingFlags += string;
}

void setCompileFlags(const std::string &string)
{
    compilerFlags += string;
}

std::mutex consoleMutex;

void sendToConsole(const std::string &text)
{
    std::lock_guard<std::mutex> lock(consoleMutex);
    SDL_Event updateEvent;
    updateEvent.type = SDL_UPDATE_CONSOLE_OUTPUT;
    updateEvent.user.data1 = new std::string(text);
    SDL_PushEvent(&updateEvent);
}
const char *platformName(Platform p)
{
    switch (p)
    {
    case Platform::Linux:
        return "Desktop";
        break;
    case Platform::Windows:
        return "Desktop";
        break;
    case Platform::Android:
        return "Android";
        break;
    case Platform::Web:
        return "Web";
        break;
    }
    return "";
}

void writeConfiJsonFile()
{
    try
    {
        json data;
        data["autoSave"] = autoSave;
        data["showConsole"] = showConsole;

        data["compilerFlags"] = compilerFlags;
        data["linkingFlags"] = linkingFlags;

        data["compilerLinuxFlags"] = compilerLinuxFlags;
        data["linkingLinuxFlags"] = linkingLinuxFlags;

        data["compilerAndroidFlags"] = compilerAndroidFlags;
        data["linkingAndroidFlags"] = linkingAndroidFlags;

        data["compilerWebFlags"] = compilerWebFlags;
        data["linkingWebFlags"] = linkingWebFlags;

        data["modulesPath"] = modulesPath;
        data["lastFile"] = lastFileOpen;
        data["windowWidth"] = Window_Width;
        data["windowHeight"] = Window_Height;
        data["clang"] = isClang;
        data["emsdk"] = emskPath;
        data["androidSdk"] = androidSdkPath;
        data["androidNdk"] = androidNdkPath;
        data["javaPath"] = javaPath;
        data["lastPlatform"] = (int)current_platform;

        std::ofstream file("config.json");
        if (file.is_open())
        {
            file << std::setw(4) << data; // Formatação com 4 tabs

            file.close();
        }
        else
        {
            std::cout << "Error savinf JSON." << std::endl;
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        sendToConsole("Error: " + std::string(ex.what()));
    }
}

void readConfigJsonFile()
{
    try
    {
        std::ifstream file("config.json");
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open config.json file.");
        }
        json data;
        file >> data;

        autoSave = data["autoSave"].get<bool>();
        showConsole = data["showConsole"].get<bool>();
        compilerFlags = data["compilerFlags"].get<std::string>();
        linkingFlags = data["linkingFlags"].get<std::string>();

        compilerLinuxFlags = data["compilerLinuxFlags"].get<std::string>();
        linkingLinuxFlags = data["linkingLinuxFlags"].get<std::string>();

        compilerAndroidFlags = data["compilerAndroidFlags"].get<std::string>();
        linkingAndroidFlags = data["linkingAndroidFlags"].get<std::string>();

        compilerWebFlags = data["compilerWebFlags"].get<std::string>();
        linkingWebFlags = data["linkingWebFlags"].get<std::string>();

        strcpy(compileBuffer, compilerFlags.c_str());
        strcpy(buildBuffer, linkingFlags.c_str());

        strcpy(compileLinuxBuffer, compilerLinuxFlags.c_str());
        strcpy(buildLinuxBuffer, linkingLinuxFlags.c_str());

        strcpy(compileAndroidBuffer, compilerAndroidFlags.c_str());
        strcpy(buildAndroidBuffer, linkingAndroidFlags.c_str());

        strcpy(compileWebBuffer, compilerWebFlags.c_str());
        strcpy(buildWebBuffer, linkingWebFlags.c_str());

        modulesPath = data["modulesPath"].get<std::string>();

        lastFileOpen = data["lastFile"].get<std::string>();
        Window_Width = data["windowWidth"].get<int>();
        Window_Height = data["windowHeight"].get<int>();
        isClang = data["clang"].get<bool>();
        emskPath = data["emsdk"].get<std::string>();
        androidSdkPath = data["androidSdk"].get<std::string>();
        androidNdkPath = data["androidNdk"].get<std::string>();
        javaPath = data["javaPath"].get<std::string>();
        current_platform = (Platform)data["lastPlatform"].get<int>();

        file.close();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        sendToConsole("Error: " + std::string(ex.what()));
    }
}

void writeTemapleJsonFile(const std::string &f)
{
    try
    {
        json data;
        data["name"] = "";
        data["CMP"] = compilerFlags;
        data["LINK"] = linkingFlags;

        data["Linux"]["CMP"] = compilerLinuxFlags;
        data["Linux"]["LINK"] = linkingLinuxFlags;

        data["Android"]["CMP"] = compilerAndroidFlags;
        data["Android"]["LINK"] = linkingAndroidFlags;

        data["Web"]["CMP"] = compilerWebFlags;
        data["Web"]["LINK"] = linkingWebFlags;
        data["Web"]["shell"] = shellWeb;
        data["Web"]["useShell"] = useShellWeb;

        std::ofstream file(f);
        if (file.is_open())
        {
            file << std::setw(4) << data;

            file.close();
        }
        else
        {
            std::cout << "Erro ao gravar o arquivo JSON." << std::endl;
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        sendToConsole("Error: " + std::string(ex.what()));
    }
}

void readTemapleJsonFile(const std::string &f)
{
    try
    {
        std::ifstream file(f);
        if (file.is_open())
        {
            json data;
            file >> data;

            std::string name = data["name"].get<std::string>();
            compilerFlags = data["CMP"].get<std::string>();
            linkingFlags = data["LINK"].get<std::string>();

            compilerLinuxFlags = data["Linux"]["CMP"].get<std::string>();
            linkingLinuxFlags = data["Linux"]["LINK"].get<std::string>();

            compilerAndroidFlags = data["Android"]["CMP"].get<std::string>();
            linkingAndroidFlags = data["Android"]["LINK"].get<std::string>();

            compilerWebFlags = data["Web"]["CMP"].get<std::string>();
            linkingWebFlags = data["Web"]["LINK"].get<std::string>();
            shellWeb = data["Web"]["shell"].get<std::string>();
            useShellWeb = data["Web"]["useShell"].get<bool>();

            strcpy(compileBuffer, compilerFlags.c_str());
            strcpy(buildBuffer, linkingFlags.c_str());

            strcpy(compileLinuxBuffer, compilerLinuxFlags.c_str());
            strcpy(buildLinuxBuffer, linkingLinuxFlags.c_str());

            strcpy(compileAndroidBuffer, compilerAndroidFlags.c_str());
            strcpy(buildAndroidBuffer, linkingAndroidFlags.c_str());

            strcpy(compileWebBuffer, compilerWebFlags.c_str());
            strcpy(buildWebBuffer, linkingWebFlags.c_str());

            strcpy(shellWebBuffer, shellWeb.c_str());

            file.close();
        }
        else
        {
            std::cout << "Erro ao ler o arquivo JSON." << std::endl;
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        sendToConsole("Error: " + std::string(ex.what()));
    }
}

void LoadFile(const std::string &filePath, TextEditor &editor)
{
    std::ifstream t(filePath);
    if (t.good())
    {
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        editor.SetText(str);

        lastFileName = GetFileNameFromPath(filePath);
        lastFileExtension = GetFileExtension(filePath);
        lastFile = GetFileNameWithoutExtension(filePath);
        isCpp = IsFileExtension(filePath, "cpp");
        canRun = fileExists(lastPath + pathSeparator + lastFile);
        lastFileOpen = filePath;

        std::cout << "lastFileName: " << lastFileName << std::endl;
        std::cout << "lastFileExtension: " << lastFileExtension << std::endl;
        std::cout << "lastFile: " << lastFile << std::endl;
        std::cout << "isCpp: " << isCpp << std::endl;
        std::cout << "Path: " << lastPath << std::endl;
    }
    else
    {
        editor.SetText("");
    }
    t.close();
}

void SaveFile(const std::string &filePath, TextEditor &editor)
{
    std::ofstream t(filePath);
    if (t.good())
    {
        std::string str = editor.GetText();
        t << str;
        lastFileName = GetFileNameFromPath(filePath);
        lastFileExtension = GetFileExtension(filePath);
        lastFile = GetFileNameWithoutExtension(filePath);
        isCpp = IsFileExtension(filePath, "cpp");
        canRun = fileExists(lastPath + pathSeparator + lastFile);
        lastFileOpen = filePath;
    }
    t.close();
}

int async_process(const std::string &command)
{

    TinyProcessLib::Process process(
        command, "",
        [&](const char *bytes, size_t n)
        {
            //   std::cout << "Output from stdout: " << std::string(bytes, n);
            std::string output = std::string(bytes, n);
            if (bytes[n - 1] != '\n')
                output += "\n";
            // consoleText += output;
            sendToConsole(output);
        },
        [&](const char *bytes, size_t n)
        {
            //   std::cout << "Output from stdout: " << std::string(bytes, n);
            std::string output = std::string(bytes, n);
            if (bytes[n - 1] != '\n')
                output += "\n";
            // consoleText += output;
            sendToConsole(output);
        },
        false);

    return process.get_exit_status();
}

std::string getCompiler()
{
    std::string compiler;
    switch (current_platform)
    {
    case Platform::Linux:
    {

        if (isClang)
            if (isCpp)
                compiler = "clang++";
            else
                compiler = "clang";
        else if (isCpp)
            compiler = "g++";
        else
            compiler = "gcc";
    }
    case Platform::Windows:
    {
        return compiler;
        break;
    }
    case Platform::Android:
    {
        if (isCpp)
            compiler = androidNdkPath + "bin" + pathSeparator + "clang++";
        else
            compiler = androidNdkPath + "bin" + pathSeparator + "clang";
        return compiler;
    }
    case Platform::Web:
    {
        if (isCpp)
            compiler = emskPath + pathSeparator + "em++ ";
        else
            compiler = emskPath + pathSeparator + "emcc ";
        return compiler;
    }
    }
    return compiler;
}

std::string linuxCompileCommand()
{
    std::string command = getCompiler();
    if (isDebug)
        command += " -g ";
    else
        command += " -DNDEBUG ";
    std::string includePath = "-I" + modulesPath + "/include -I" + modulesPath + "/include/Linux ";

    if (mkdir(std::string(lastPath + pathSeparator + "OBJ" + pathSeparator).c_str(), 0777) == 0)
    {
        // std::cout << "Folder " << outputPath << " Created." << std::endl;
    }

    command += includePath;
    command += " " + compilerFlags + " " + compilerLinuxFlags + " -c ";
    command += fileToEdit;
    command += " -o ";
    command += lastPath + pathSeparator + "OBJ" + pathSeparator + lastFile + ".o";
    return command;
}

std::string linuxBuildCommand()
{
    std::string command;
    command = getCompiler();
    command += " -o ";
    command += lastPath + pathSeparator + lastFile;
    command += " ";
    command += lastPath + pathSeparator + "OBJ" + pathSeparator + lastFile + ".o ";

    std::string libPath = "-L" + modulesPath + "/libs/Linux ";

    command += libPath;

    command += linkingFlags + " " + linkingLinuxFlags;
    return command;
}

std::string webCompileCommand()
{
    std::string command = getCompiler();
    if (isDebug)
        command += " -g ";
    else
        command += " -DNDEBUG ";

    std::string includePath = "-I" + modulesPath + "/include -I" + modulesPath + "/include/Web ";
    if (mkdir(std::string(lastPath + pathSeparator + "OBJ" + pathSeparator).c_str(), 0777) == 0)
    {
        // std::cout << "Folder " << outputPath << " Created." << std::endl;
    }

    command += includePath;
    command += " " + compilerFlags + " " + compilerWebFlags + " -c ";
    command += fileToEdit;
    command += " -o ";
    command += lastPath + pathSeparator + "OBJ" + pathSeparator + lastFile + ".o";
    return command;
}

std::string webBuildCommand()
{
    std::string command;

    command = getCompiler();

    std::string outputPath = lastPath + pathSeparator + "web_" + lastFile;

    command += " -o ";
    command += lastPath + pathSeparator + "web_" + lastFile + pathSeparator + lastFile + ".html";
    command += " ";
    command += lastPath + pathSeparator + "OBJ" + pathSeparator + lastFile + ".o ";
    if (useShellWeb)
    {
        command += " --shell-file " + shellWeb + " ";
    }

    /*

    std::string shellWeb = "";
bool        useShellWeb = false;*/

    //--shell-file path/to/custom_shell.html

    std::string libPath = "-L" + modulesPath + "/libs/Web ";
    command += libPath;

    command += linkingFlags + " " + linkingWebFlags;

    if (mkdir(outputPath.c_str(), 0777) == 0)
    {
        // std::cout << "Folder " << outputPath << " Created." << std::endl;
    }

    return command;
}

std::string CompileCommand()
{
    switch (current_platform)
    {
    case Platform::Linux:
        return linuxCompileCommand();
        break;
    case Platform::Web:
        return webCompileCommand();
        break;
    }
    return "";
}

std::string BuildCommand()
{
    switch (current_platform)
    {
    case Platform::Linux:
        return linuxBuildCommand();
        break;
    case Platform::Web:
        return webBuildCommand();
        break;
    }
    return "";
}

void readOutputFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    std::string line;

    while (std::getline(file, line))
    {
        std::cout << line << std::endl;
    }

    file.close();
}

void printOutput(const char *bytes, size_t n)
{
    std::string output(bytes, n);
    sendToConsole(output);
}

void executeProcess(Async::Semaphore &semaphore, const std::string &command)
{

    switch (current_platform)
    {
    case Platform::Linux:
    {
        TinyProcessLib::Process process(command, "", printOutput, printOutput);
        int exit_status = process.get_exit_status();
        sendToConsole("Exit with status: " + std::to_string(exit_status));
        ChangeDirectory(rootPath.c_str());
    }
    break;
    case Platform::Web:
    {
        TinyProcessLib::Process process(command, "", printOutput, printOutput);
        int exit_status = process.get_exit_status();
        sendToConsole("Exit with status: " + std::to_string(exit_status));

        break;
    }
    }

    semaphore.signal();
}

void runExecutable(const std::string &command)
{
    // sendToConsole("Running: " + lastFile + "\n");
    Async::Semaphore semaphore(1);
    semaphore.wait();
    std::thread t(executeProcess, std::ref(semaphore), command);
    t.join();
}

bool doRun()
{
    if (!canRun)
        return false;

    std::string command;

    switch (current_platform)
    {
    case Platform::Linux:
    {
        ChangeDirectory(lastPath.c_str());
        command = lastPath + pathSeparator + lastFile;
        break;
    }
    case Platform::Web:
    {
        //   std::string outputPath = lastPath + pathSeparator + "Web" + lastFile;
        std::string outputPath = lastPath + pathSeparator + "web_" + lastFile + pathSeparator + lastFile + ".html";

        command = "emrun --kill_start --kill_exit " + outputPath;
        break;
    }
    }

    std::thread executionThread(runExecutable, command);
    executionThread.detach();
    return true;
}

bool doCompile(TextEditor &editor)
{
    bool result = true;
    std::string command;
    std::string output;

    command = CompileCommand();

    std::cout << "Compile CMD: " << command << std::endl;

    output = run_command(command);

    consoleText = "";
    consoleText += output;

    if (!output.empty())
    {
        std::cout << output << std::endl;
        size_t position = output.find("error:");
        if (position != std::string::npos)
        {
            sendToConsole(output);
            result = false;
        }
        else
        {
            result = true;
        }
    }

    // consoleText += command + "\n";
    // sendToConsole(command + "\n");

    //  std::cout << output << std::endl;

    // if (output != "")
    // {
    //     replace_character(output, "‘", "'");
    //     replace_character(output, "’", "'");

    //     std::vector<std::string> lines = get_result(output);
    //     if (lines.size() == 6)
    //     {
    //         //  consoleText = output;
    //         consoleText += lines[4] + "\n";
    //         consoleText += lines[0] + "\n";
    //         consoleText += "At Function (" + lines[1] + ") Line: " + lines[2] + " Column: " + lines[3] + "\n";
    //         std::string msg = lines[5];

    //         TextEditor::Coordinates coord;
    //         coord.mLine = std::stoi(lines[2]) - 1;
    //         coord.mColumn = std::stoi(lines[3]);
    //         editor.SetCursorPosition(coord);

    //         consoleText += msg + ".\n";
    //         consoleText += output;
    //     }
    //     else
    //     {
    //         consoleText = output;
    //     }

    //     result = false;
    // }
    // else
    // {
    //     result = true;
    //     // consoleText += "Compiled successfully.\n";
    //   //  sendToConsole("Compiled successfully.\n");
    //   std::cout<<"Compiled successfully.\n";
    // }
    canBuild = result;
    return result;
}

void buildProcess(Async::Semaphore &semaphore, const std::string &command, bool run)
{
    std::string output = run_command(command);
    std::cout << output << std::endl;

    if (output != "")
    {
        replace_character(output, "‘", "'");
        replace_character(output, "’", "'");
        sendToConsole("Build faill.\n");
        sendToConsole(output);
    }
    else
    {
        // sendToConsole("Build successfully.\n");
        std::cout << "Build successfully.\n";
        std::string obj = lastPath + pathSeparator + lastFile + ".o";
        /*
        if (std::remove(obj.c_str()) == 0)
        {
            // std::cout("Arquivo removido com sucesso.\n");
        }*/
        if (run)
        {
            switch (current_platform)
            {
            case Platform::Linux:
            {
                doRun();
            }
            break;
            case Platform::Web:
            {
                doRun();
            }
            break;
            }
        }
    }
    semaphore.signal();
}

void runBuild(const std::string &command, bool run)
{
    // sendToConsole("Build: " + lastFile + "\n");
    std::cout << "Build: " << lastFile << "\n";

    Async::Semaphore semaphore(1);
    semaphore.wait();
    std::thread t(buildProcess, std::ref(semaphore), command, run);
    t.join();
}

bool doBuild(bool run = false)
{
    if (!canBuild)
        return false;
    bool result = true;
    std::string command;
    std::string output;

    command = BuildCommand();

    std::cout << "Build CMD: " << command << std::endl;

    // sendToConsole(command + "\n");

    std::thread executionThread(runBuild, command, run);
    executionThread.detach();

    return result;
}

const int Rows = 3;
const int Cols = 8;
bool checkboxes[Rows][Cols];

void renderGUI()
{
    for (int row = 0; row < Rows; row++)
    {
        ImGui::Text("Row %d:", row);
        ImGui::Indent();

        for (int col = 0; col < Cols; col++)
        {
            ImGui::PushID(row * Cols + col);
            ImGui::Checkbox("raylib", &checkboxes[row][col]);
            ImGui::PopID();

            ImGui::SameLine();
        }

        ImGui::Unindent();
    }
}

int main(int, char **)
{

    if (fileExists("config.json"))
    {
        readConfigJsonFile();
    }
    else
        writeConfiJsonFile();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Cross Builder by DjokerSoft v.0.01", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Window_Width, Window_Height, window_flags);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return false;
    }

    consoleText += "\n";
    consoleText += "SDL2 Version: " + std::string(SDL_GetRevision()) + "\n";

    char currentPath[FILENAME_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) != nullptr)
    {
        rootPath = currentPath;
        consoleText += "Work path : " + rootPath + "\n";
        //  std::cout << "Caminho atual: " << currentPath << std::endl;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);

    consoleText += "Current SDL_Renderer: " + std::string(info.name) + "\n";

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    // ImGui::StyleColorsVSModernDark(&style);
    ImGui::StyleColorsXP(&style);

    // ImGui::StyleColorsClassic();

    ImGui_ImplSDL2_InitForSDLRenderer(window);
    ImGui_ImplSDLRenderer_Init(renderer);

    setFontWithSize("fonts/Consolas.ttf", 23);

    style.ScaleAllSizes(1.2);

    TextEditor editor;
    auto lang = TextEditor::LanguageDefinition::CPlusPlus();

    Uint32 startTime = SDL_GetTicks();
    Uint32 elapsedTime = 0;

    editor.SetLanguageDefinition(lang);
    editor.SetPalette(TextEditor::GetVSCodePalette());

    if (fileExists(lastFileOpen))
    {
        fileToEdit = lastFileOpen;
        lastPath = GetDirectoryPath(fileToEdit);
        LoadFile(fileToEdit, editor);
    }

    // error markers
    // TextEditor::ErrorMarkers markers;
    // markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
    // markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
    // editor.SetErrorMarkers(markers);

    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    int display_w, display_h;
    SDL_GetWindowSize(window, &display_w, &display_h);

    bool done = false;
    while (!done)
    {
        bool enableCompiler = (editor.GetTotalLines() > 0 && !fileToEdit.empty());

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {

            switch (event.type)
            {
            case SDL_QUIT:
            {
                done = true;
                break;
            }
            case SDL_WINDOWEVENT:
            {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    done = true;
                else if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    SDL_GetWindowSize(window, &display_w, &display_h);
                    Window_Width = display_w;
                    Window_Height = display_h;
                }
                break;
            }
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    done = true;
                isSave = false;
                noKeys = false;
                break;
            }
            case SDL_KEYUP:
            {
                canSave = true;
                noKeys = true;

                if (enableCompiler)
                {

                    if (event.key.keysym.sym == SDLK_F6)
                    {
                        SaveFile(fileToEdit, editor);
                        isSave = true;
                        if (doCompile(editor))
                            doBuild(false);

                        break;
                    }
                    else if (event.key.keysym.sym == SDLK_F8)
                    {
                        showOptions = !showOptions;
                    }
                    else if (event.key.keysym.sym == SDLK_F5)
                    {
                        SaveFile(fileToEdit, editor);
                        isSave = true;
                        if (doCompile(editor))
                            doBuild(true);
                        break;
                    }
                    else if (event.key.keysym.sym == SDLK_F4)
                    {
                        doRun();
                        break;
                    }
                }
                if (event.key.keysym.sym == SDLK_F2)
                {
                    showConsole = !showConsole;
                    break;
                }
                if (event.key.keysym.sym == SDLK_F3)
                {
                    isDebug = !isDebug;
                    consoleText = "";
                    if (isDebug)
                    {
                        consoleText += "Debug Mode\n";
                    }
                    else
                    {
                        consoleText += "Release Mode\n";
                    }

                    break;
                }

                break;
            }
            case SDL_UPDATE_CONSOLE_OUTPUT:
            {
                std::string *output = static_cast<std::string *>(event.user.data1);
                consoleText += *output;
                //   std::cout << *output << std::endl;
                delete output;
                break;
            }
            }
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);
        if (keyboardState[SDL_SCANCODE_LCTRL] || keyboardState[SDL_SCANCODE_RCTRL])
        {
            if (keyboardState[SDL_SCANCODE_S] && canSave)
            {

                if (editor.GetTotalLines() > 0 && !fileToEdit.empty())
                {
                    SaveFile(fileToEdit, editor);
                    isSave = true;
                }
                canSave = false;
            }
            if (keyboardState[SDL_SCANCODE_F12] && !noKeys)
            {
                isClang = !isClang;
                consoleText = "";
                if (isClang)
                {
                    consoleText += "Clang\n";
                }
                else
                {
                    consoleText += "GCC\n";
                }
                noKeys = true;
            }
            if (keyboardState[SDL_SCANCODE_W] && !noKeys)
            {
                current_platform = Platform::Web;
                noKeys = true;
            }
            if (keyboardState[SDL_SCANCODE_D] && !noKeys)
            {
                current_platform = Platform::Linux;
                noKeys = true;
            }
            if (keyboardState[SDL_SCANCODE_A] && !noKeys)
            {
                current_platform = Platform::Android;
                noKeys = true;
            }
        }

        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (autoSave)
        {
            elapsedTime = SDL_GetTicks() - startTime;
            // Verificar se o tempo decorrido é igual ou superior a 5 segundos (5000 milissegundos)
            if (elapsedTime >= saveTimer)
            {
                startTime = SDL_GetTicks();
                if (editor.GetTotalLines() > 0 && !fileToEdit.empty() && !isSave)
                {
                    SaveFile(fileToEdit, editor);
                    isSave = true;
                }
            }
        }
        int consoleSize = 200;
        auto cpos = editor.GetCursorPosition();
        int size = (showConsole) ? consoleSize + 45 : 45;
        int editorSize = display_h - size;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(display_w, editorSize));
        if (!showOptions)
        {
            ImGui::Begin("Text Editor Demo", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New"))
                    {
                        editor.SetText("");
                        fileToEdit = "";
                        isCpp = false;
                        isSave = false;
                        canSave = true;
                        canRun = false;
                        lastFileName = "";
                        lastFileExtension = "";
                        lastFile = "";

                        //   autoSave = false;
                        //   showConsole = true;
                    }

                    if (ImGui::MenuItem("Load"))
                    {
                        auto textToSave = editor.GetText();
                        std::vector<std::string> filters = {
                            "All (*.*)", "*.*",
                            "Archive C++ (*.cpp)", "*.cpp",
                            "Archive C (*.c)", "*.c"};
                        auto result = pfd::open_file("Load File", lastPath, filters, false).result();
                        if (!result.empty())
                        {

                            fileToEdit = result[0];

                            lastPath = GetDirectoryPath(fileToEdit);

                            std::cout << fileToEdit << "" << lastPath << std::endl;
                            LoadFile(fileToEdit, editor);
                        }
                    }
                    if (editor.GetTotalLines() > 0 && !fileToEdit.empty())
                    {
                        if (ImGui::MenuItem("Save"))
                        {
                            auto textToSave = editor.GetText();
                            if (!fileToEdit.empty())
                            {
                                SaveFile(fileToEdit, editor);
                            }
                            isSave = true;
                        }
                    }
                    if (ImGui::MenuItem("Save As"))
                    {
                        auto textToSave = editor.GetText();
                        std::vector<std::string> filters = {
                            "All (*.*)", "*.*",
                            "Archive C++ (*.cpp)", "*.cpp",
                            "Archive C (*.c)", "*.c"};

                        pfd::save_file result = pfd::save_file("Save File", lastPath, filters, true);

                        std::string fResult = result.result();

                        // auto result = pfd::save_file("Save File", lastPath, filters, false).result();
                        if (!fResult.empty())
                        {
                            auto textToSave = editor.GetText();

                            fileToEdit = fResult;
                            lastPath = GetDirectoryPath(fileToEdit);
                            if (GetFileExtension(fileToEdit) == "")
                            {
                                fileToEdit += ".cpp";
                                SaveFile(fileToEdit, editor);
                                isSave = true;
                            }

                            std::cout << fileToEdit << "" << lastPath << std::endl;
                            SaveFile(fileToEdit, editor);
                            isSave = true;
                        }
                    }
                    ImGui::Separator();

                    if (ImGui::BeginMenu("New from Template"))
                    {

                        if (ImGui::MenuItem("Raylib"))
                        {

                            editor.SetText(genRayLib());
                        }
                        if (ImGui::BeginMenu("SDL2"))
                        {
                            if (ImGui::MenuItem("Simples"))
                            {
                                editor.SetText(getSDL());
                            }
                            if (ImGui::MenuItem("OpenGL"))
                            {
                                editor.SetText(getSDLOpengGl());
                            }
                            if (ImGui::MenuItem("Render"))
                            {
                                editor.SetText(getSDLRender());
                            }

                            ImGui::EndMenu();
                        }

                        if (ImGui::BeginMenu("OpenGL"))
                        {
                            if (ImGui::MenuItem("GLFW"))
                            {
                                editor.SetText(getGLFW());
                            }

                            ImGui::EndMenu();
                        }

                        if (ImGui::MenuItem("gtk3"))
                        {
                            editor.SetText(getGTK3());
                        }
                        if (ImGui::MenuItem("gtkmm3"))
                        {
                            editor.SetText(getGTKmm());
                        }
                        if (ImGui::MenuItem("QT5"))
                        {
                            editor.SetText(getQT5());
                        }

                        ImGui::EndMenu();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Auto Save", nullptr, &autoSave))
                    {
                        startTime = SDL_GetTicks();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Quit", "Alt-F4"))
                        break;
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    bool ro = editor.IsReadOnly();
                    if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                        editor.SetReadOnly(ro);
                    ImGui::Separator();

                    if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
                        editor.Undo();
                    if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
                        editor.Redo();

                    ImGui::Separator();

                    if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
                        editor.Copy();
                    if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
                        editor.Cut();
                    if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
                        editor.Delete();
                    if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                        editor.Paste();

                    ImGui::Separator();

                    if (ImGui::MenuItem("Select all", nullptr, nullptr))
                        editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View"))
                {
                    if (ImGui::MenuItem("Dark palette"))
                        editor.SetPalette(TextEditor::GetDarkPalette());
                    if (ImGui::MenuItem("Light palette"))
                        editor.SetPalette(TextEditor::GetLightPalette());
                    if (ImGui::MenuItem("Retro blue palette"))
                        editor.SetPalette(TextEditor::GetRetroBluePalette());
                    if (ImGui::MenuItem("Vs Code palette"))
                        editor.SetPalette(TextEditor::GetVSCodePalette());
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Compiler"))
                {
                    if (ImGui::MenuItem("Build", "F6", nullptr, enableCompiler))
                    {
                        SaveFile(fileToEdit, editor);
                        isSave = true;
                        if (doCompile(editor))
                            doBuild(false);
                    }
                    // if (ImGui::MenuItem("Build", "F8", nullptr, enableCompiler))
                    // {
                    //     SDL_Event keyEvent;
                    //     keyEvent.type = SDL_KEYDOWN;
                    //     keyEvent.key.keysym.sym = SDLK_F8;
                    //     SDL_PushEvent(&keyEvent);
                    // }

                    if (ImGui::MenuItem("Build Run", "F5", nullptr, enableCompiler))
                    {
                        SaveFile(fileToEdit, editor);
                        isSave = true;
                        if (doCompile(editor))
                            doBuild(true);
                    }

                    if (ImGui::MenuItem("Run", "F4", nullptr, enableCompiler && canRun))
                    {
                        doRun();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Options", "F8", nullptr))
                    {
                        showOptions = true;
                    }

                    ImGui::Separator();
                    if (current_platform == Platform::Linux)
                    {
                        if (ImGui::MenuItem("Clang", "CTRL-F12", &isClang))
                        {
                            consoleText = "";
                            if (isClang)
                            {
                                consoleText += "Clang\n";
                            }
                            else
                            {
                                consoleText += "GCC\n";
                            }
                        }
                    }
                    if (ImGui::MenuItem("Debug", "F3", &isDebug))
                    {
                        consoleText = "";
                        if (isDebug)
                        {
                            consoleText += "Debug Mode\n";
                        }
                        else
                        {
                            consoleText += "Release Mode\n";
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Console"))
                {
                    if (ImGui::MenuItem("Show", "F2", &showConsole, true))
                    {
                        // SDL_Event keyEvent;
                        // keyEvent.type = SDL_KEYDOWN;
                        // keyEvent.key.keysym.sym = SDLK_F2;
                        // SDL_PushEvent(&keyEvent);
                    }
                    if (ImGui::MenuItem("Clear"))
                    {
                        consoleText = "@DjokerSoft 2023";
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Plataform"))
                {
                    if (ImGui::MenuItem("Desktop", "CTRL-D", current_platform == Platform::Linux))
                    {
                        current_platform = Platform::Linux;
                    };
                    if (ImGui::MenuItem("Web", "CTRL-W", current_platform == Platform::Web))
                    {
                        current_platform = Platform::Web;
                    };
                    if (ImGui::MenuItem("Android", "CTRL-A", current_platform == Platform::Android))
                    {
                        current_platform = Platform::Android;
                    };

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            editor.Render("TextEditor");

            ImGui::End();
        }

        float statusBarPosY = editorSize;

        float statusBarHeight = 40;
        ImGui::SetNextWindowPos(ImVec2(0, statusBarPosY));
        ImGui::SetNextWindowSize(ImVec2(display_w, statusBarHeight));
        ImGui::Begin("Status Bar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        const char *fielName = lastFileName.c_str();
        const char *currPlat = platformName(current_platform);

        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s | %s | Compiler [%s] | Build [%s] | FPS: %.2f", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
                    editor.IsOverwrite() ? "Ovr" : "Ins",
                    isSave ? "[ ]" : "[*]",
                    isCpp ? "C++" : "C", fielName,
                    isDebug ? "Debug" : "Release",
                    isClang ? "Clang" : "GCC",
                    currPlat,
                    ImGui::GetIO().Framerate

        );
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(0, statusBarPosY + 40));
        ImGui::SetNextWindowSize(ImVec2(display_w, consoleSize));
        if (showConsole)
        {
            ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine;

            std::vector<char> buffer(consoleText.begin(), consoleText.end());
            buffer.push_back('\0');
            /// ImGui::TextWrapped("Console",consoleText.c_str(),

            if (ImGui::InputTextMultiline("Console", buffer.data(), buffer.size(), ImVec2(-1, -1), flags))
            {
            }

            ImGui::End();
        }
        // ImGui::Begin("Modules");
        // renderGUI();
        // ImGui::End();
        if (showOptions)
        {

            std::vector<char> bufferCompile(compilerFlags.begin(), compilerFlags.end());
            bufferCompile.push_back('\0');
            std::vector<char> bufferLink(linkingFlags.begin(), linkingFlags.end());
            bufferLink.push_back('\0');

            ImGui::Begin("Compiler Options");
            ImGui::Text("Compile flags:");
            ImGui::InputText("##compilerFlags", compileBuffer, sizeof(compileBuffer));
            // ImGui::InputTextMultiline("##compilerFlags", bufferCompile.data(), bufferCompile.size());
            ImGui::Text("Linking flags:");
            // ImGui::InputTextMultiline("##linkingFlags", bufferLink.data(), bufferLink.size());
            ImGui::InputText("##linkingFlags", buildBuffer, sizeof(buildBuffer));
            ImGui::Separator();

            ImGui::Text("Platform: %s", platformName(current_platform));

            ImGui::Separator();

            switch (current_platform)
            {
            case Platform::Android:
            {
                ImGui::Text("Compile flags:");
                ImGui::InputText("##compilerAndroidFlags", compileAndroidBuffer, sizeof(compileAndroidBuffer));
                ImGui::Text("Linking flags:");
                ImGui::InputText("##linkingAndroidFlags", buildAndroidBuffer, sizeof(buildAndroidBuffer));
                break;
            }
            case Platform::Web:
            {
                ImGui::Text("Compile flags:");
                ImGui::InputText("##compilerWebFlags", compileWebBuffer, sizeof(compileWebBuffer));
                ImGui::Text("Linking flags:");
                ImGui::InputText("##linkingWebFlags", buildWebBuffer, sizeof(buildWebBuffer));
                ImGui::InputText("##shellWeb", shellWebBuffer, sizeof(shellWebBuffer));
                ImGui::SameLine();
                ImGui::Checkbox("Use Shell", &useShellWeb);
                break;
            }
            case Platform::Linux:
            {
                ImGui::Text("Compile flags:");
                ImGui::InputText("##compilerLinuxFlags", compileLinuxBuffer, sizeof(compileLinuxBuffer));
                ImGui::Text("Linking flags:");
                ImGui::InputText("##linkingLinuxFlags", buildLinuxBuffer, sizeof(buildLinuxBuffer));

                break;
            }
            }
            ImGui::Separator();

            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("  Load  ").x) / 2.0f - 100);
            if (ImGui::Button("  Load  "))
            {
                std::string path = rootPath + pathSeparator + "templates" + pathSeparator;
                std::vector<std::string> filters = {"Json (*.json)", "*.json"};
                auto result = pfd::open_file("Load File", path, filters, false).result();
                if (!result.empty())
                {

                    std::string loadFile = result[0];

                    readTemapleJsonFile(loadFile);
                }
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("  Save  ").x) / 2.0f + 100);
            if (ImGui::Button("  Save  "))
            {
                std::string path = rootPath + pathSeparator + "templates" + pathSeparator;
                std::vector<std::string> filters = {"Json (*.json)", "*.json"};

                pfd::save_file result = pfd::save_file("Save File", path, filters, true);
                std::string fResult = result.result();
                if (!fResult.empty())
                {
                    std::string saveFile = fResult;
                    if (GetFileExtension(saveFile) == "")
                    {
                        saveFile += ".json";
                    }
                    writeTemapleJsonFile(saveFile);
                }
            }
            ImGui::Separator();
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("   Ok   ").x) / 2.0f - 100);

            if (ImGui::Button("   Ok   "))
            {

                compilerFlags = std::string(compileBuffer);
                linkingFlags = std::string(buildBuffer);

                compilerAndroidFlags = std::string(compileAndroidBuffer);
                linkingAndroidFlags = std::string(buildAndroidBuffer);

                compilerWebFlags = std::string(compileWebBuffer);
                linkingWebFlags = std::string(buildWebBuffer);

                compilerLinuxFlags = std::string(compileLinuxBuffer);
                linkingLinuxFlags = std::string(buildLinuxBuffer);

                shellWeb = std::string(shellWebBuffer);

                std::cout << "Compiler flags: " << compilerFlags << std::endl;
                std::cout << "Linking flags: " << linkingFlags << std::endl;

                showOptions = false;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Cancelar").x) / 2.0f + 100);
            if (ImGui::Button("Cancelar"))
            {
                showOptions = false;
            }
            ImGui::End();
            if (keyboardState[SDL_SCANCODE_RETURN])
            {
                compilerFlags = std::string(compileBuffer);
                linkingFlags = std::string(buildBuffer);

                compilerAndroidFlags = std::string(compileAndroidBuffer);
                linkingAndroidFlags = std::string(buildAndroidBuffer);

                compilerWebFlags = std::string(compileWebBuffer);
                linkingWebFlags = std::string(buildWebBuffer);

                compilerLinuxFlags = std::string(compileLinuxBuffer);
                linkingLinuxFlags = std::string(buildLinuxBuffer);

                std::cout << "Compiler flags: " << compilerFlags << std::endl;
                std::cout << "Linking flags: " << linkingFlags << std::endl;

                showOptions = false;
            }
        }

        // Rendering
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
        SDL_PumpEvents();
        // std::cout << "FPS: " << ImGui::GetIO().Framerate << std::endl;
    }

    // Cleanup
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    writeConfiJsonFile();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
