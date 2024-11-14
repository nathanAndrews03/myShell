#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <signal.h>
#include <sys/wait.h>

#include "Command.hh"
#include "Shell.hh"


int yyparse(void);
char *path;
int is_current_buffer_null();

Shell * Shell::TheShell;

Shell::Shell() {
    this->_level = 0;
    this->_enablePrompt = true;
    this->_listCommands = new ListCommands(); 
    this->_simpleCommand = new SimpleCommand();
    this->_pipeCommand = new PipeCommand();
    this->_currentCommand = this->_pipeCommand;
    this->dontClear = false;
    //this->_ifCommand = new IfCommand();
    if ( !isatty(0)) {
	this->_enablePrompt = false;

    }
}
Shell::~Shell() {
    delete this->_listCommands;
    delete this->_simpleCommand;
    delete this->_pipeCommand;
    delete this->_currentCommand;
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
    //this->_ifCommand->print();
    if (this->_ifCommand == NULL || !this->dontClear) {
    	this->_listCommands->clear();
    	this->_simpleCommand->clear();
    }
    this->_pipeCommand->clear();
    this->_currentCommand->clear();
    //this->_ifCommand->clear();
    this->_level = 0;
}

void Shell::execute() {
  if (this->_level == 0) {
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
			//if(isatty(0)) printf("[%d] exited\n", pid);
        	} else {
            		break;
        	}
    	}	
	Shell::TheShell->prompt();
}

int main(int argc, char **argv) {


  for (int i = 1; i < argc; ++i) {
	char var_name[10];
        sprintf(var_name, "%d", i - 1);  // Create a variable name based on the argument index
        setenv(var_name, argv[i], 1);
  }

    // Set the '*' environment variable to contain all arguments
  std::string allArgs;
  for (int i = 1; i < argc; i++) { // Start from 1 to skip the program's name
    if (i > 1) allArgs += " ";
    allArgs += argv[i];
  }
  setenv("*", allArgs.c_str(), 1);
  
  // Setting individual arguments (similar to ${1}, ${2}, ..., ${n})
  for (int i = 2; i < argc; i++) {
    char envName[20];
    snprintf(envName, sizeof(envName), "%d", i-1);
    setenv(envName, argv[i], 1);
  }

  path = argv[0];
  if (argc > 1) {
  	setenv("0", argv[1], 1);
  }


  char argc_str[10];
  sprintf(argc_str, "%d", argc - 2);  
  setenv("#", argc_str, 1);

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
  //while (true) {
        yyparse();
	

        // If yyparse returns because of EOF and there's no more buffer to pop 
        //if (feof(stdin) && is_current_buffer_null()) {
          //  break;
        //}

        // If there's no file or we've returned to terminal input, show the prompt
        //if (!input_file || is_current_buffer_null()) {
            Shell::TheShell->prompt();

	Shell::TheShell->~Shell();
	//delete Shell::TheShell;

        //}
  //}
}

	
