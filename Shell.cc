#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <signal.h>
#include <sys/wait.h>

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
		printf("\rmyshell>"); 
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

void cInterrupt(int sig) {
	std::cout << "\n";
	Shell::TheShell->clear();
	Shell::TheShell->prompt();
}

void zInterrupt(int sig) {
	int pid;
	while (1) {
        	pid = waitpid(-1, NULL, WNOHANG);
        	if (pid > 0) {
			if(isatty(0)) printf("[%d] exited\n", pid);
        	} else {
            		break;
        	}
    	}	
	Shell::TheShell->prompt();
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

  struct sigaction sa_c;
  sa_c.sa_handler = cInterrupt;
  sigemptyset(&sa_c.sa_mask);
  sa_c.sa_flags = SA_RESTART;
  int error = sigaction(SIGINT, &sa_c, NULL);
  if (error) {
	perror("sigaction");
	exit(1);
  }

  struct sigaction sa_z;
  sa_z.sa_handler = zInterrupt;
  sigemptyset(&sa_z.sa_mask);
  sa_z.sa_flags = SA_RESTART;
  error = sigaction(SIGCHLD, &sa_z, NULL);
  if (error) {
	perror("sigaction");
	exit(-1);
  }



  Shell::TheShell = new Shell();

  if (input_file != NULL) {
    // No prompt if running a script
    Shell::TheShell->_enablePrompt = false;
  }
  else {
    Shell::TheShell->prompt();
  }
  yyparse();
}

	
