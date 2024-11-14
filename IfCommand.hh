#ifndef ifcommand_hh
#define ifcommand_hh


#include <stack>
#include "Command.hh"
#include "SimpleCommand.hh"
#include "ListCommands.hh"

// Command Data Structure

class IfCommand : public Command {
public:
  SimpleCommand * _condition;
  ListCommands * _listCommands;
  bool isWhileLoop;
  bool isForLoop;
  std::string forArg;
  //bool dontClear;
  //static std::stack<IfCommand*> ifStack;
  int maEntries;
  int nEntrie;
  char **entrie;
  IfCommand();
  void insertCondition( SimpleCommand * condition );
  void insertListCommands( ListCommands * listCommands);
  static int runTest(SimpleCommand * condition);
  SimpleCommand* expandSimple(SimpleCommand * cond);
  char ** wildcardIfNecessary(SimpleCommand * s);
  void expandWildcards(char * prefix, char * arg);
  void clear();
  void print();
  void execute();

};

#endif
