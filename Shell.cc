
#include <unistd.h>
#include <cstdio>
#include <signal.h>

#include "Command.hh"
#include "Shell.hh"

int yyparse(void);

Shell * Shell::TheShell;

Shell::Shell() {
    this->_level = 0;
    this->_enablePrompt = true;
    this->_listCommands = new ListCommands(); 
    this->_simpleCommand = new SimpleCommand();
    this->_pipeCommand = new PipeCommand();
    this->_currentCommand = this->_pipeCommand;
    if ( !isatty(0)) {
	this->_enablePrompt = false;
    }
}

void Shell::prompt() {
    	//if (isatty(0) {
	//	printf("myshell>");
	//	fflush
	//}
	if (_enablePrompt) {
		printf("myshell>"); 
		fflush(stdout);
	}
}

void Shell::print() {
    printf("\n--------------- Command Table ---------------\n");
    this->_listCommands->print();
}

void Shell::clear() {
    this->_listCommands->clear();
    this->_simpleCommand->clear();
    this->_pipeCommand->clear();
    this->_currentCommand->clear();
    this->_level = 0;
}

void Shell::execute() {
  if (this->_level == 0 ) {
    //this->print();
    this->_listCommands->execute();
    this->_listCommands->clear();
    this->prompt();
  }
}

void yyset_in (FILE *  in_str );

static int keepRunning = 1;
extern "C" void sigInterupt(int sig) {
	keepRunning = 0;
}
 
int main(int argc, char **argv) {

  char * input_file = NULL;
  if ( argc > 1 ) {
    input_file = argv[1];
    FILE * f = fopen(input_file, "r");
    if (f==NULL) {
	fprintf(stderr, "Cannot open file %s\n", input_file);
        perror("fopen");
        exit(1);
    }
    yyset_in(f);
  }  
/*
  struct sigaction sa;
  sa.sa_handler = sInterupt;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGINT, &sa, NULL)) {
	perror("sigaction");
	exit(2);
  }
*/
  Shell::TheShell = new Shell();

  if (input_file != NULL) {
    // No prompt if running a script
    Shell::TheShell->_enablePrompt = false;
  }
  else {
    Shell::TheShell->prompt();
  }
  signal(SIGINT, sigInterupt);
  while (keepRunning) {
  	yyparse();
  }
}


