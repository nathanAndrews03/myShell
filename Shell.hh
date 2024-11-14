#ifndef shell_hh
#define shell_hh

#include "ListCommands.hh"
#include "PipeCommand.hh"
#include "IfCommand.hh"

extern char *path;
class Shell {

public:
  int _level; // Only outer level executes.
  bool _enablePrompt;
  ListCommands * _listCommands; 
  SimpleCommand *_simpleCommand;
  PipeCommand * _pipeCommand;
  IfCommand * _ifCommand;
  Command * _currentCommand;
  static Shell * TheShell;
  bool dontClear;
  int is_current_buffer_null();
  std::stack<IfCommand*> ifCommandStack;
  std::stack<ListCommands*> listCommandsStack;

  Shell();
  ~Shell();
  void execute();
  void print();
  void clear();
  void prompt();

};

#endif
