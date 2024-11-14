
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND TWOGREAT 
%token AMPERSAND PIPE LESS NEWLINE IF FI THEN LBRACKET RBRACKET SEMI
%token DO DONE WHILE FOR IN EXIT

%{
//#define yylex yylex
#include <stdio.h>
#include <cstdio>
#include "Shell.hh"
#include <stack>
std::stack<IfCommand*> ifCommandStack;
std::stack<ListCommands*> listCommandsStack;

void yyerror(const char * s);
int yylex();
extern int yylex_destroy();

%}

%%

goal: command_list;

arg_list:
        arg_list WORD {
	  //expandWildcardsIfNecessary($1);
          Shell::TheShell->_simpleCommand->insertArgument( $2 ); }
        | /*empty string*/
	;

cmd_and_args:
  	WORD { 
          Shell::TheShell->_simpleCommand = new SimpleCommand(); 
          Shell::TheShell->_simpleCommand->insertArgument( $1 );
	  //expandWildcardsIfNecessary($1);
        } 
        arg_list
	;

pipe_list:
        cmd_and_args 
	    { 
		Shell::TheShell->_pipeCommand->insertSimpleCommand( 
		    Shell::TheShell->_simpleCommand ); 
		Shell::TheShell->_simpleCommand = new SimpleCommand();
	    }
	| pipe_list PIPE cmd_and_args 
	    {
	    	Shell::TheShell->_pipeCommand->insertSimpleCommand(
	    		Shell::TheShell->_simpleCommand);
	  	Shell::TheShell->_simpleCommand = new SimpleCommand();
	    }
	;

io_modifier:
	   GREATGREAT WORD
	   {
	   	Shell::TheShell->_pipeCommand->_outFile = $2;
		Shell::TheShell->_pipeCommand->_append = true;
	   }
	 | GREAT WORD 
	    {
		if (Shell::TheShell->_pipeCommand->_outFile) {
        		fprintf(stderr, "Ambiguous output redirect.\n");
    		} else {
        		Shell::TheShell->_pipeCommand->_outFile = $2;
    		}
	    }
	 | GREATGREATAMPERSAND WORD
	 {
	 	Shell::TheShell->_pipeCommand->_outFile = $2;
		//Shell::TheShell->_pipeCommand->_errFile = $2;
		Shell::TheShell->_pipeCommand->_errFile = new std::string($2->c_str());
		Shell::TheShell->_pipeCommand->_append = true;
	 }
	 | GREATAMPERSAND WORD
	 {
	 	Shell::TheShell->_pipeCommand->_outFile = $2;
		Shell::TheShell->_pipeCommand->_errFile = new std::string($2->c_str());
		//Shell::TheShell->_pipeCommand->_errFile = $2;
	 }
	 | LESS WORD
	 {
	 	Shell::TheShell->_pipeCommand->_inFile = $2;
	 }
	 | TWOGREAT WORD 
	 {
		Shell::TheShell->_pipeCommand->_errFile = $2;
	 }
	;

io_modifier_list:
	io_modifier_list io_modifier
	| /*empty*/
	;

background_optional: 
	AMPERSAND
	{
		Shell::TheShell->_pipeCommand->_background = true;
	}
	| /*empty*/
	;

SEPARATOR:
	NEWLINE
	| SEMI
	;
// if you have nested ifs they wont be in the list right
// cs250 compiler used stack with labels for break and continue. Look at top of stack and go to that label
// Do something similar in command_line for stack of list commands ***
command_line:
	 pipe_list io_modifier_list background_optional SEPARATOR 
         { 
	 // ***
	    Shell::TheShell->_listCommands->
		insertCommand(Shell::TheShell->_pipeCommand);
	    Shell::TheShell->_pipeCommand = new PipeCommand(); 
         }
        | if_command SEPARATOR 
         {
	 // ***
	    //Shell::TheShell->ifCommandStack.push(Shell::TheShell->_ifCommand);
	    if (Shell::TheShell->_level == 0 && !Shell::TheShell->ifCommandStack.empty()) {
		Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->ifCommandStack.top());
		Shell::TheShell->ifCommandStack.pop();
	    } else {
	    	Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->_ifCommand);
	    }
         }
        | while_command SEPARATOR
		{
		    if (Shell::TheShell->_level == 0 && !Shell::TheShell->ifCommandStack.empty()) {
		    	
			Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->ifCommandStack.top());
			Shell::TheShell->ifCommandStack.pop();

	   	    } else {
	    		Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->_ifCommand);
	    	    }
		}
        | for_command SEPARATOR { 
		    if (Shell::TheShell->_level == 0 && !Shell::TheShell->ifCommandStack.empty()) {
		    	
			Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->ifCommandStack.top());
			Shell::TheShell->ifCommandStack.pop();

	   	    } else {
	    		Shell::TheShell->_listCommands->insertCommand(Shell::TheShell->_ifCommand);
	    	    }
	}
	| EXIT { yylex_destroy(); exit(0); }
        | SEPARATOR /*accept empty cmd line*/
        | error SEPARATOR {yyerrok; Shell::TheShell->clear(); }
	;          /*error recovery*/

command_list :
     command_line 
	{ 
	   Shell::TheShell->execute();
	}
     | 
     command_list command_line 
	{
	    Shell::TheShell->execute();
	}
     ;  /* command loop*/

if_command:
	IF LBRACKET
	   {
        	Shell::TheShell->_level++;
		if (Shell::TheShell->_level > 1) {
        		Shell::TheShell->ifCommandStack.push(Shell::TheShell->_ifCommand);
		}
        	Shell::TheShell->_ifCommand = new IfCommand();

		
    	  }

	arg_list RBRACKET SEMI THEN
    	  { 	

        	Shell::TheShell->_ifCommand->insertCondition(
                	Shell::TheShell->_simpleCommand);
        	Shell::TheShell->_simpleCommand = new SimpleCommand();
    	  }

	command_list FI
    	    {

        	Shell::TheShell->_level--;
		if (Shell::TheShell->_level == 0 && !Shell::TheShell->ifCommandStack.empty()) {
			Shell::TheShell->ifCommandStack.top()->insertListCommands(Shell::TheShell->_listCommands);
		} else {
			Shell::TheShell->_ifCommand->insertListCommands(Shell::TheShell->_listCommands);
		}




       		Shell::TheShell->_listCommands = new ListCommands();

    	   }
	;

while_command:
	WHILE LBRACKET {
		Shell::TheShell->_level++;
		if (Shell::TheShell->_level > 1) {
			Shell::TheShell->ifCommandStack.push(Shell::TheShell->_ifCommand);
			ListCommands *saveList = new ListCommands();
			saveList->insertCommand(Shell::TheShell->_listCommands);
			Shell::TheShell->listCommandsStack.push(saveList);
			Shell::TheShell->_listCommands = new ListCommands();
		}
		Shell::TheShell->_ifCommand = new IfCommand();
	}
	arg_list RBRACKET SEMI DO {
		Shell::TheShell->_ifCommand->insertCondition(
			Shell::TheShell->_simpleCommand);
		Shell::TheShell->_simpleCommand = new SimpleCommand();
	}
	command_list DONE 
	{
		Shell::TheShell->_level--;
		if (Shell::TheShell->_level == 0 && !Shell::TheShell->listCommandsStack.empty()) {
			ListCommands *final = new ListCommands();
			final->insertCommand(Shell::TheShell->listCommandsStack.top());
			Shell::TheShell->listCommandsStack.pop();
			final->insertCommand(Shell::TheShell->_listCommands);
			Shell::TheShell->ifCommandStack.top()->insertListCommands(final);
			Shell::TheShell->ifCommandStack.top()->isWhileLoop = true;
		} else {
			Shell::TheShell->_ifCommand->insertListCommands(Shell::TheShell->_listCommands);
		}
	    	Shell::TheShell->_listCommands = new ListCommands();
		Shell::TheShell->_ifCommand->isWhileLoop = true;
	}
    ;

for_command:
    FOR WORD
	{
		Shell::TheShell->_level++;
		if (Shell::TheShell->_level > 1) {
			Shell::TheShell->ifCommandStack.push(Shell::TheShell->_ifCommand);
			ListCommands *saveList = new ListCommands();
			saveList->insertCommand(Shell::TheShell->_listCommands);
			Shell::TheShell->listCommandsStack.push(saveList);
			Shell::TheShell->_listCommands = new ListCommands();
		}
		Shell::TheShell->_ifCommand = new IfCommand();
		Shell::TheShell->_ifCommand->forArg = *$2;
	}	
	IN arg_list SEMI DO 
	{
		Shell::TheShell->_ifCommand->insertCondition(
			Shell::TheShell->_simpleCommand);
		//Shell::TheShell->_simpleCommand->print();
		Shell::TheShell->_simpleCommand = new SimpleCommand();

	}
	command_list DONE 
	{
		Shell::TheShell->_level--;
		if (Shell::TheShell->_level == 0 && !Shell::TheShell->listCommandsStack.empty()) {
			ListCommands *final = new ListCommands();
			final->insertCommand(Shell::TheShell->listCommandsStack.top());
			Shell::TheShell->listCommandsStack.pop();
			final->insertCommand(Shell::TheShell->_listCommands);
			Shell::TheShell->ifCommandStack.top()->insertListCommands(final);
			Shell::TheShell->ifCommandStack.top()->isForLoop = true;
		} else {
			Shell::TheShell->_ifCommand->insertListCommands(Shell::TheShell->_listCommands);
		}
	    	Shell::TheShell->_listCommands = new ListCommands();
		Shell::TheShell->_ifCommand->isForLoop = true;


	}
    ;

%%

void
yyerror(const char * s)
{
  fprintf(stderr,"\n");
}

#if 0
main()
{
  yyparse();
}
#endif
