#include "utils.hpp"
#include "nlohmann/json.hpp"
#include "process.hpp"
#include <future>
#include <iostream>
#include <regex>
#include <thread>
#include <functional>
#include <tuple>
#include <memory>

std::vector<std::string> get_result(std::string &input)
{
//    std::regex regexPattern(R"((.+):(\d+):(\d+): (error|warning): (.+))");
    std::regex regexPattern(R"((.+):(\d+):(\d+): (fatal error|error|warning): (.+))");

    std::smatch match;
    std::vector<std::string> result;

    if (std::regex_search(input, match, regexPattern))
    {
        if (match.size() == 6)
        {
            std::string fileName = match[1].str();
            std::string lineNumber = match[2].str();
            std::string columnNumber = match[3].str();
            std::string messageType = match[4].str();
            std::string errorMessage = match[5].str();
            std::string functionName = "";

            std::regex regexFunction(R"(In function ‘(.+)’)");
            std::smatch matchFunction;
            if (std::regex_search(input, matchFunction, regexFunction))
            {
                if (matchFunction.size() == 2)
                {
                    functionName = matchFunction[1].str();
                }
            }
            std::regex regexRemove(R"(\[.*?\])");
            errorMessage = std::regex_replace(errorMessage, regexRemove, "");

            // std::cout << "File Name: " << fileName << std::endl;
            // std::cout << "Function Name: " << functionName << std::endl;
            // std::cout << "Line Number: " << lineNumber << std::endl;
            // std::cout << "Column Number: " << columnNumber << std::endl;
            // std::cout << "Type: " << messageType << std::endl;
            // std::cout << "Message: " << errorMessage << std::endl;

            result.push_back(fileName);
            result.push_back(functionName);
            result.push_back(lineNumber);
            result.push_back(columnNumber);
            result.push_back(messageType);
            result.push_back(errorMessage);
        }
    }

    return result;
}

bool is_gcc_or_gpp_error(std::string &input)
{
    std::regex regexPattern(R"(^(gcc|g\+\+): error:)");
    return std::regex_search(input, regexPattern);
}

bool isOutputMessage(const std::string &input)
{
    std::regex regexPattern(R"(^' OUTPUT: .+ collect2: error: ld returned 1 exit status '$)");

    return std::regex_search(input, regexPattern);
}

std::string extractOutputErrorMessage(const std::string &input)
{
    std::regex regexPattern(R"(OUTPUT: .+?:(.+): No such file)");
    std::smatch match;

    if (std::regex_search(input, match, regexPattern) && match.size() == 2)
    {
        return match[1].str();
    }

    return "";
}

std::string get_compiler_error(std::string &input)
{
    std::regex regexPattern(R"((gcc|g\+\+): error: (.+))");
    std::string result;
    std::smatch match;
    if (std::regex_search(input, match, regexPattern))
    {
        if (match.size() == 3)
        {
            result = match[2].str();
        }
    }
    return result;
}

void replace_character(std::string &str, const std::string &target, const std::string &replacement)
{
    size_t pos = 0;
    while ((pos = str.find(target, pos)) != std::string::npos)
    {
        str.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }
}


void async_print(std::string message)
{
    SDL_Event updateEvent;
    updateEvent.type = SDL_UPDATE_CONSOLE_OUTPUT;
    updateEvent.user.data1 = new std::string(message); // Crie uma cópia do texto de saída
    SDL_PushEvent(&updateEvent);
}

std::string run_command_async(const std::string& command)
{
    int pipefd[2];
    pipe(pipefd);
    pid_t pid = fork();

    std::string finalCommand = command;
 

    if (pid == -1)
    {
        std::cout << "fork failed" << std::endl;
        exit(1);
    }
    else if (pid == 0)
    {
        close(pipefd[0]);  // Close the read end in the child
        dup2(pipefd[1], STDOUT_FILENO);
        execl("/bin/sh", "sh", "-c", finalCommand.c_str(), (char*)NULL);
        exit(0);  // Only reached if execl fails
    }
    else
    {
        close(pipefd[1]);  // Close the write end in the parent

            std::future<std::string> future = std::async(std::launch::async, [pipefd]() 
            {
            std::string output;
            char buffer[54 + 1];
            int rd = 0;
            while ((rd = read(pipefd[0], buffer, 54)) > 0)
            {
                buffer[rd] = '\0';
                output += buffer;
                std::cout << buffer << std::flush;
               // async_print(buffer);
            }

      
            return output;
        });

        int status;
        waitpid(pid, &status, 0);  // Aguarda o término do processo filho e obtém o status de saída
        int exut_status = WEXITSTATUS(status);

        std::cout << "run completed" << std::flush;

        return future.get();  // Aguarda e obtém o resultado da execução assíncrona
    }
    return "";
}
std::string run_command(const std::string &command)
{
    int stdout_pipe[2];  // pipe para o stdout
    int stderr_pipe[2];  // pipe para o stderr

    pipe(stdout_pipe);
    pipe(stderr_pipe);

    pid_t pid = fork();

    if (pid == -1)
    {
        std::cout << "fork failed" << std::endl;
        exit(1);
    }
    else if (pid == 0)
    {
        // Filho
        close(stdout_pipe[0]);  // Fecha a extremidade de leitura do stdout no filho
        close(stderr_pipe[0]);  // Fecha a extremidade de leitura do stderr no filho

        // Redireciona o stdout para o pipe do stdout
        dup2(stdout_pipe[1], STDOUT_FILENO);

        // Redireciona o stderr para o pipe do stderr
        dup2(stderr_pipe[1], STDERR_FILENO);

        execl("/bin/sh", "sh", "-c", command.c_str(), (char *)NULL);
        exit(0);  // Só é alcançado se execl falhar
    }
    else
    {
        // Pai
        close(stdout_pipe[1]);  // Fecha a extremidade de escrita do stdout no pai
        close(stderr_pipe[1]);  // Fecha a extremidade de escrita do stderr no pai

        std::string stdout_output;
        std::string stderr_output;

        char buffer[512 + 1];
        int rd = 0;

        // Lê o stdout
        while ((rd = read(stdout_pipe[0], buffer, 512)) > 0)
        {
            buffer[rd] = '\0';
            stdout_output += buffer;
        }

        // Lê o stderr
        while ((rd = read(stderr_pipe[0], buffer, 512)) > 0)
        {
            buffer[rd] = '\0';
            stderr_output += buffer;
        }

        int status;
        waitpid(pid, &status, 0);  // Aguarda o término do processo filho e obtém o status de saída
        int exut_status = WEXITSTATUS(status);
        return stdout_output + stderr_output;
    };

    return "";
}


void executeAyncCommand(const std::string& command)
{
    
    int stdout_pipe[2];  // pipe para o stdout
    int stderr_pipe[2];  // pipe para o stderr

	bool failed=false;
    bool exited;
    int exit_code = 0;
    
    pipe(stdout_pipe);
    pipe(stderr_pipe);

    pid_t pid = vfork();

    if (pid == -1)
    {
        std::cout << "fork failed" << std::endl;
        exit(1);
    }
    else if (pid == 0)
    {
        // Filho

        setsid();  // Cria um novo grupo de processos


        
         close(stdout_pipe[0]);  
         close(stderr_pipe[0]);  

        
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);

        execl("/bin/sh", "sh", "-c", command.c_str(), (char *)NULL);
        failed=true;
        exit(127);  // Só é alcançado se execl falhar
        
    }
    else
    {
        

        // Pai
        close(stdout_pipe[1]);  // Fecha a extremidade de escrita do stdout no pai
        close(stderr_pipe[1]);  // Fecha a extremidade de escrita do stderr no pai

        if( failed)
        {
            
            close( stdout_pipe[0] );
            close( stderr_pipe[0] );
            return ;
        }

    std::thread( [&]()
    {
			int status;
			waitpid( pid, &status,0 );

			if( WIFEXITED( status ) )
            {
				exit_code=WEXITSTATUS( status );
			}else
            {
				exit_code=-1;
			}
            close( stdout_pipe[0] );
            close( stderr_pipe[0] );
          //  sendToConsole("Process end with exit code: " + std::to_string(exit_code));
			exited=true;
	} ).detach();


	std::thread( [&]()
    {
       auto buffer = std::unique_ptr<char[]>( new char[4096] );
      ssize_t n;
      while ((n=read(stdout_pipe[0], buffer.get(), 4096)) > 0)
      {
         //   printf("%s %d \n ",buffer.get(), static_cast<size_t>(n));
      }
    } ).join();


	std::thread( [&]()
    {
      auto buffer = std::unique_ptr<char[]>( new char[4096] );
      ssize_t n;
      while ((n=read(stderr_pipe[0], buffer.get(), 4096)) > 0)
      {
         //   printf("%s %d \n ",buffer.get(), static_cast<size_t>(n));
      }
    } ).join();

    

    };

}

void writer(const char *message, int count, FILE *stream)
{
    for (; count > 0; --count)
    {
        //* Write the message to the stream, and send it off immediately.
        fprintf(stream, "%s\n", message);
        fflush(stream);
        // Snooze a while.
        sleep(1);
    }
}

void reader(FILE *stream)
{
    char buffer[1024];
    /* Read until we hit the end of the stream. fgets reads until
    either a newline or the end-of-file. */
    while (!feof(stream) && !ferror(stream) && fgets(buffer, sizeof(buffer), stream) != NULL)
        fputs(buffer, stdout);
}

std::string run_command_stream(const std::string &command, bool redirect)
{
    int pipefd[2];
    pipe(pipefd);
    pid_t pid = fork();

    std::string finalCommand = command;
    if (redirect)
    {
        finalCommand += " 2>&1";
    }

    if (pid == -1)
    {
        std::cout << "fork failed" << std::endl;
        exit(1);
    }
    else if (pid == 0)
    {
        FILE *stream;
        close(pipefd[0]); // Close the read end in the child
        dup2(pipefd[1], STDOUT_FILENO);
        stream = fdopen(pipefd[0], "r");
        execl("/bin/sh", "sh", "-c", finalCommand.c_str(), (char *)NULL);

        exit(0); // Only reached if execl fails
    }
    else
    {
        close(pipefd[1]); // Close the write end in the parent
        std::string output;
        char buffer[512 + 1];
        int rd = 0;
        while ((rd = read(pipefd[0], buffer, 512)) > 0)
        {
            buffer[rd] = '\0';
            output += buffer;
        }

        return output;
    }
    return "";
}

std::string GetDirectoryPath(const std::string &filePath)
{
    size_t found = filePath.find_last_of("/\\");
    return filePath.substr(0, found);
}

std::string GetFileExtension(const std::string &filePath)
{
    size_t found = filePath.find_last_of(".");
    if (found != std::string::npos)
    {
        return filePath.substr(found + 1);
    }
    return "";
}

bool IsFileExtension(const std::string &filePath, const std::string &extension)
{
    size_t found = filePath.find_last_of(".");
    if (found != std::string::npos)
    {
        std::string fileExtension = filePath.substr(found + 1);
        return fileExtension == extension;
    }
    return false;
}

bool HasFileExtension(const std::string &filePath)
{
    size_t found = filePath.find_last_of(".");
    return found != std::string::npos;
}
std::string GetFileNameFromPath(const std::string &filePath)
{
    size_t found = filePath.find_last_of("/\\");
    if (found != std::string::npos)
    {
        return filePath.substr(found + 1);
    }
    return filePath;
}

std::string GetFileNameWithoutExtension(const std::string &filePath)
{
    // Encontra a posição do último separador de diretório
    size_t lastSeparatorPos = filePath.find_last_of("/\\");

    // Encontra a posição do último ponto na extensão do arquivo
    size_t lastDotPos = filePath.find_last_of(".");

    // Verifica se o último ponto está após o último separador
    if (lastDotPos != std::string::npos && lastDotPos > lastSeparatorPos)
    {
        // Retorna o nome do arquivo sem a extensão
        return filePath.substr(lastSeparatorPos + 1, lastDotPos - lastSeparatorPos - 1);
    }

    // Retorna o nome do arquivo completo, se não houver extensão
    return filePath.substr(lastSeparatorPos + 1);
}

bool fileExists(const std::string &filename)
{
    std::ifstream file(filename);
    return file.good();
}

int run_process(const std::string &command, const std::string &path, bool use_pipes)
{

       TinyProcessLib::Process process(command, path, 
        [](const char *bytes, size_t n)
        { 
           // std::cout << "Output from stdout: " << std::string(bytes, n); 
            std::string output = std::string(bytes, n);
            if (bytes[n - 1] != '\n')
                output+="\n";

        },
        [](const char *bytes, size_t n)
        {
            std::string output = std::string(bytes, n);
            if (bytes[n - 1] != '\n')
                output+="\n";

            
          
        });
    return  process.get_exit_status();
}


static char **makeargv( const char *cmd ){
    int n,c;
    char *p;
    static char *args,**argv;

    if( args ) free( args );
    if( argv ) free( argv );
    args=(char*)malloc( strlen(cmd)+1 );
    strcpy( args,cmd );

    n=0;
    p=args;
    while( (c=*p++) ){
        if( c==' ' ){
            continue;
        }else if( c=='\"' ){
            while( *p && *p!='\"' ) ++p;
        }else{
            while( *p && *p!=' ' ) ++p;
        }
        if( *p ) ++p;
        ++n;
    }
    argv=(char**)malloc( (n+1)*sizeof(char*) );
    n=0;
    p=args;
    while( (c=*p++) ){
        if( c==' ' ){
            continue;
        }else if( c=='\"' ){
            argv[n]=p;
            while( *p && *p!='\"' ) ++p;
        }else{
            argv[n]=p-1;
            while( *p && *p!=' ' ) ++p;
        }
        if( *p ) *p++=0;
        ++n;
    }
    argv[n]=0;
    return argv;
}


std::vector<std::string> std_makeargv(const std::string& cmd)
{
    std::vector<std::string> argv;
    std::string args = cmd;

    size_t n = 0;
    size_t pos = 0;
    while (pos < args.length())
    {
        if (args[pos] == ' ')
        {
            pos++;
        }
        else if (args[pos] == '\"')
        {
            size_t endQuote = args.find('\"', pos + 1);
            if (endQuote == std::string::npos)
                endQuote = args.length();
            argv.push_back(args.substr(pos + 1, endQuote - pos - 1));
            pos = endQuote + 1;
        }
        else
        {
            size_t endPos = args.find(' ', pos + 1);
            if (endPos == std::string::npos)
                endPos = args.length();
            argv.push_back(args.substr(pos, endPos - pos));
            pos = endPos;
        }
    }

    return argv;
}