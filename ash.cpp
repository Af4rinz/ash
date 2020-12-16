/**************************************************
 * ash shell program                           *
 * Course: Operating Systems - Fall 2020          *
 * Date: 16/12/2020                               *
 *                                                *
 * Objectives: a simple bash shell to execute     *
 * commands, pass messages, and pipeline multiple *
 * commands together. Executable in script mode.  *
 *                                                *
***************************************************/
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <locale>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>

#include <readline/history.h>
#include <readline/readline.h>

// maximum size of buffer
#define BUFFER_SIZE 512

// prompt message
#define PMPT " "

#define EXITSH "quit"
// clear shell
#define CLEAR() cout << "\033[H\033[J"

// ansi colours
#define DEFAULT "\x1B[0m"
#define WHITE "\x1B[37m"
#define RED "\x1B[31m"
#define YELLOW "\x1B[33m"
#define GREEN "\x1B[32m"
#define CYAN "\x1B[36m"
#define BLUE "\x1B[34m"
// for bold prompt text
#define BOLD "\e[1m"
#define REVERSE "\e[7m"
#define WHITEBG "\x1B[47m"

using namespace std;

// basics
char currDir[BUFFER_SIZE];
char *input;
string homeDir;
string sinput;
int shid;
bool stop = false;

// IPC stuff
string inbox = "/tmp/ash-inbox-";
string outbox = "/tmp/ash-inbox-";
vector<string> pmessages;

// builtin commands and aliases
map<string, void (*)(vector<string>)> biCmd;
map<string, void (*)(vector<string>)>::iterator biItr;
map<string, string> aliasTable;

// replace beginning string with sth else (~ <-> $HOME)
string strReplace(string str, string init, string fin) {

  size_t str_res = str.find(init);
  if (str_res == string::npos) {
    // not found
    return str;
  } else if (str_res != 0) {
    // not at the beggining
    return str;
  }

  string res;

  res = fin;
  res = fin + str.substr(init.length());

  return res;
}

// change directory
void run_cd(vector<string> args) {
  string dir = args[1];
  // invalid directory
  if (dir == "")
    return;

  string slashedHomeDir = homeDir + "/";
  string cdPath = strReplace(dir, "~", homeDir);

  if (chdir(cdPath.c_str()) != 0) {
    perror("cannot change directory");
  }
  // update working directory
  getcwd(currDir, BUFFER_SIZE);
}

// show shell help
void run_help(vector<string> args) {
  // some ascii art :))))
  cout << CYAN;
  cout << " ________  ________  ___  ___     "
       << "\n";
  cout << "|\\   __  \\|\\   ____\\|\\  \\|\\  \\    "
       << "\n";
  cout << "\\ \\  \\|\\  \\ \\  \\___|\\ \\  \\\\   \\   "
       << "\n";
  cout << " \\ \\   __  \\ \\_____  \\ \\   __  \\  "
       << "\n";
  cout << "  \\ \\  \\ \\  \\|____|\\  \\ \\  \\ \\  \\ "
       << "\n";
  cout << "   \\ \\__\\ \\__\\____\\_\\  \\ \\__\\ \\__\\ "
       << "\n";
  cout << "    \\|__|\\|__|\\_________\\|__|\\|__|"
       << "\n";
  cout << "             \\|_________|         "
       << "\n\n\n";
  // basic info
  cout << "ash shell was developed for Operating Systems - Fall 2020 course.\n";
  cout << "For help with external commands use \'man\'.\n";

  cout << BLUE << "\n\nBuilt-in commands:\n\n";

  for (biItr = biCmd.begin(); biItr != biCmd.end(); ++biItr) {
    cout << CYAN
         << "\n-------------------------×××-------------------------\n\n";
    cout << YELLOW << biItr->first << ":\n";

    if (biItr->first == "cd") {
      cout << GREEN BOLD << "\tcd [path]\n" << DEFAULT;
      cout << YELLOW << "Changes the current working directory.\n";
    }

    else if (biItr->first == "help") {
      cout << GREEN BOLD << "\thelp\n" << DEFAULT;
      cout << YELLOW << "Shows this help message.\n";
    }

    else if (biItr->first == "quit") {
      cout << GREEN BOLD << "\tquit\n" << DEFAULT;
      cout << YELLOW << "Exits the shell with return code 0.\n";
    }

    else if (biItr->first == "send") {
      cout << GREEN BOLD << "\tsend [shell_id] [message]\n" << DEFAULT;
      cout << YELLOW << "Send a message to another shell.\n";
    }

    else if (biItr->first == "receive") {
      cout << GREEN BOLD << "\treceive\n" << DEFAULT;
      cout << YELLOW << "Receive all unread messages from the other shell.\n";
    }

    else if (biItr->first == "clear") {
      cout << GREEN BOLD << "\tclear\n" << DEFAULT;
      cout << YELLOW << "Clears terminal.\n";
    }

    else if (biItr->first == "history") {
      cout << GREEN BOLD << "\thistory\n" << DEFAULT;
      cout << YELLOW << "Shows the most recent commands.\n";
    }

    else if (biItr->first == "pwd") {
      cout << GREEN BOLD << "\tpwd\n" << DEFAULT;
      cout << YELLOW << "Prints the current working directory.\n";
    }

    else if (biItr->first == "shid") {
      cout << GREEN BOLD << "\tshid\n" << DEFAULT;
      cout << YELLOW << "Prints the shell id given by user.\n";
    }
  }
  cout << DEFAULT << endl;
}

// exit ash
void run_exit(vector<string> args) { exit(0); }

// clear input
void run_clear(vector<string> args) { CLEAR(); }

// get shell id
void run_shid(vector<string> args) {
  cout << shid << endl;
  return;
}

// print command history
void run_history(vector<string> args) {
  HIST_ENTRY **hlist = history_list();
  for (int i = 0; i < history_length; i++) {
    cout << hlist[i]->line << "\n";
  }
}

// print current working directory
void run_pwd(vector<string> args) {
  string result = strReplace(currDir, homeDir, "~");

  if (result == "~") {
    result = "~/";
  }
  cout << BLUE << result << DEFAULT << endl;
  return;
}
// print terminal prompt
void prompt() {
  string result = strReplace(currDir, homeDir, "~");

  if (result == "~") {
    result = "~/";
  }
  cout << BOLD BLUE WHITEBG REVERSE << " " << result << PMPT DEFAULT << " ";
}

// make fifo (named pipe) for inbox
void makeFifo() { mkfifo(inbox.c_str(), 0666); }

void getFifo() {
  while (1) {
    int fd = open(inbox.c_str(), O_RDONLY);
    string msg = "";
    char buffer[BUFFER_SIZE] = {};
    read(fd, buffer, BUFFER_SIZE);

    msg += string(buffer);
    pmessages.push_back(msg);
    close(fd);
  }
}

// receive messages from inbox
void receive(vector<string> args) {
  for (string pmsg : pmessages) {
    cout << YELLOW << pmsg << DEFAULT << endl;
  }
  pmessages.clear();
}

// send a message to another shell
void send(vector<string> args) {
  if (args.size() > 2) {
    string wPath = (outbox + args[1]);
    int fd = open(wPath.c_str(), O_WRONLY | O_NDELAY);
    string msg = "";
    msg += "From " + to_string(shid) + ": ";
    // re-glue the arguments
    for (int i = 2; i < args.size(); i++) {
      msg += args[i];
      if (i != args.size()) {
        msg += " ";
      }
    }
    write(fd, msg.c_str(), msg.length());
    close(fd);
  } else
    cout << RED BOLD << "Invalid arguments for send.\n" << DEFAULT;
}

void parseUtil(string in, vector<string> &inArgs) {
  // reset previous values
  inArgs.clear();
  string tok;
  string temp;
  istringstream iss(in);

  getline(iss >> ws, tok, ' ');
  if (aliasTable.count(tok)) {
    string al = aliasTable[tok];
    istringstream aiss(al);

    while (getline(aiss >> ws, temp, ' ')) {
      if (temp.length() == 0 || temp == "")
        continue;
      inArgs.push_back(temp);
    }
  } else if (tok.length() != 0) {
    inArgs.push_back(tok);
  }

  while (getline(iss >> ws, temp, ' ')) {
    if (temp.length() == 0)
      continue;
    inArgs.push_back(temp);
  }
}

void parse(string &in, vector<vector<string>> &inArgs) {
  // reset inArgs
  inArgs.clear();
  istringstream iss(in);
  string temp;

  while (getline(iss >> ws, temp, '|')) {
    if (temp.length() == 0)
      continue;
    vector<string> res;
    parseUtil(temp, res);
    inArgs.push_back(res);
  }
}

// execute and run commands
int execute(vector<string> instr, bool isPipe, int fdInput) {

  int stat;
  int fdIn[2], fdOut[2];
  pipe(fdIn);
  pipe(fdOut);

  pid_t childPid = fork();

  if (childPid < 0) {
    cout << RED << "Error while forking child.\n" << DEFAULT;
  }
  // child
  else if (childPid == 0) {

    char **cstr = new char *[instr.size() + 1];
    for (int i = 0; i < instr.size(); i++) {
      cstr[i] = (char *)instr[i].c_str();
    }
    // null terminate
    cstr[instr.size()] = NULL;

    if (fdInput != 0) {
      while ((dup2(fdIn[0], 0) == -1) && (errno == EINTR))
        ;
      close(fdIn[1]);
      close(fdIn[0]);
    }

    if (isPipe) {
      while ((dup2(fdOut[1], 1) == -1) && (errno == EINTR))
        ;
      close(fdOut[1]);
      close(fdOut[0]);
    }
    // run the command
    if (execvp(cstr[0], cstr) == -1) {
      perror("oops ×_×");
      exit(1);
    }
    // exit child only
    exit(0);
  }

  // parent
  else {
    if (fdInput != 0) {
      // we don need to read from input here
      close(fdIn[0]);
      char buffer[BUFFER_SIZE];
      int resFread;
      while ((resFread = read(fdInput, buffer, BUFFER_SIZE))) {
        write(fdIn[1], buffer, resFread);
      }
      close(fdIn[1]);
    }

    // close the write end of output
    if (isPipe)
      close(fdOut[1]);

    // wait for child
    while (wait(&stat) != childPid)
      ;
    // return read end of output for next pipe
    if (isPipe)
      return fdOut[0];

    return 0;
  }
}

// handle ctrl + C
void handler(int signal){
  if (shid != getpid()){
    exit(0);
  }
  else {
    stop = true;
    return;
  }
}

// ------ MAIN -------
int main(int argc, char **argv) {

  // built-in command setup
  biCmd["send"] = &send;
  biCmd["receive"] = &receive;
  biCmd["cd"] = &run_cd;
  biCmd["help"] = &run_help;
  biCmd["exit"] = &run_exit;
  biCmd["clear"] = &run_clear;
  biCmd["history"] = &run_history;
  biCmd["pwd"] = &run_pwd;
  biCmd["shid"] = &run_shid;

  // get home and current directory
  homeDir = getenv("HOME");
  getcwd(currDir, BUFFER_SIZE);

  // some basic aliasing
  aliasTable["ls"] = "ls --color=auto";
  aliasTable["cp"] = "cp -i";
  aliasTable["df"] = "df -h";
  aliasTable["free"] = "free -m";
  aliasTable["grep"] = "grep --colour=auto";

  switch (argc) {

  // ****INTERACTIVE MODE****
  case 1: {
    
    shid = getpid();
    inbox += to_string(shid);
    makeFifo();
    thread thrd(getFifo);

    // initiate history
    using_history();

    // begin the real work
    vector<vector<string>> procIn;
    // handle ctrl + C input
    signal(SIGINT, handler);

    while (1) {
      bool isPipe = false;
      bool isBi = false;

      // print prompt
      prompt();
      // c string for basic processing
      input = readline("");
      // cpp string for its own functions
      sinput = input;
      // skip line if empty
      if (input == NULL || sinput == EXITSH) {
        cout << BLUE << "leaving..." << DEFAULT << endl;
        // break;
        exit(0);
      }
      sinput = regex_replace(sinput, regex("^ +| +$|( ) +"), "$1");
      if (sinput == "" || stop) {
        stop = false;
        cout << endl;
        continue;
      }

      // adds to history only if it wasn't empty
      add_history(input);

      parse(sinput, procIn);

      int pipedResFd = 0;
      for (biItr = biCmd.begin(); biItr != biCmd.end(); ++biItr) {
        // came out as builtin command
        if (procIn[0][0] == biItr->first) {
          (*biItr->second)(procIn[0]);
          isBi = true;
        }
      }

      if (!isBi) {
        isPipe = (procIn.size() > 1);
        for (int i = 0; i < procIn.size(); i++) {
          if (isPipe) {
            pipedResFd = execute(procIn[i], isPipe, pipedResFd);

            // do for the last pipe, if they exist
            if (i==procIn.size()-1 && i != 0) {

              char buffer[BUFFER_SIZE];
              int resFread;
              while ((resFread = read(pipedResFd, buffer, BUFFER_SIZE))) {
                write(1, buffer, resFread);
              }
            }
          } else
            execute(procIn[i], isPipe, pipedResFd);
        }
      }
    }
    break;
  }

  // ****BATCH MODE****
  case 2: {
    if (fork() == 0) {
      ifstream batchFile(argv[1]);
      cout << CYAN REVERSE << "\t\topened " << argv[1] << " in batch mode" << DEFAULT << endl;
      while (getline(batchFile, sinput)) {
        vector<vector<string>> procIn;

        bool isPipe = false;
        bool isBi = false;
        // skip line if empty
        sinput = regex_replace(sinput, regex("^ +| +$|( ) +"), "$1");
        if (sinput == "") {
          cout << endl;
          continue;
        }
        cout << YELLOW BOLD << sinput << DEFAULT << endl;
        if (sinput == EXITSH) {
          cout << RED BOLD << "leaving..." << DEFAULT << endl;
          break;
        }

        parse(sinput, procIn);

        int pipedResFd = 0;
        for (biItr = biCmd.begin(); biItr != biCmd.end(); ++biItr) {
          // came out as builtin command
          if (procIn[0][0] == biItr->first) {
            (*biItr->second)(procIn[0]);
            isBi = true;
          }
        }

        if (!isBi) {
          isPipe = (procIn.size() > 1);
          for (int i = 0; i < procIn.size(); i++) {
            if (isPipe) {
              pipedResFd = execute(procIn[i], isPipe, pipedResFd);

              // do for the last pipe, if they exist
              if (i != 0) {

                char buffer[10];
                int resFread;
                while ((resFread = read(pipedResFd, buffer, 10))) {
                  write(1, buffer, resFread);
                }
              }
            } else
              execute(procIn[i], isPipe, pipedResFd);
          }
        }
      }
      exit (0);
    }
  break;
  }
  default: {
    cout << RED BOLD << "Invalid arguments, exiting..." << DEFAULT << endl;
    break;
  }
  }
  exit(0);
}
