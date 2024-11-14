#ifndef pipecommand_hh
#define pipecommand_hh

#include "Command.hh"
#include "SimpleCommand.hh"

// Command Data Structure

class PipeCommand : public Command {
public:
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool _background;
  bool _append;
  // append flag?
  int maxEntries;
  int nEntries;
  char **entries;


  PipeCommand();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void print();
  int built(const char **args);
  void execute();
  char * processArg(const char * arg);
  SimpleCommand* expandSimple(int i);
  // Expands environment vars and wildcards of a SimpleCommand and
  // returns the arguments to pass to execvp.
  char ** wildcardIfNecessary(SimpleCommand * s);
  void expandWildcards(char * prefix, char * arg);

};

#endif
