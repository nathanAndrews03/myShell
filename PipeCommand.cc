/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "PipeCommand.hh"
#include "Shell.hh"


PipeCommand::PipeCommand() {
    // Initialize a new vector of Simple PipeCommands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _append = false;
}

void PipeCommand::insertSimpleCommand( SimpleCommand * simplePipeCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simplePipeCommand);
}

void PipeCommand::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simplePipeCommand : _simpleCommands) {
        delete simplePipeCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
}

void PipeCommand::print() {
    printf("\n\n");
    //printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple PipeCommands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simplePipeCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simplePipeCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void PipeCommand::execute() {
    	// Don't do anything if there are no simple commands
    	if ( _simpleCommands.size() == 0 ) {
        	Shell::TheShell->prompt();
        	return;
    	}

    	// Print contents of PipeCommand data structure
    	//print();

   	// Add execution here
    	// For every simple command fork a new process
    	// Setup i/o redirection
    	// and call exec
    	int ret;

    	//save in/out
	int tmpin=dup(0);
	int tmpout=dup(1);
	int tmperr = dup(2);
	//set the initial input
	int fdin;
	int fdout;
	int fderr;
	if (_inFile) {
		fdin = open(_inFile->c_str(), O_RDONLY);
	} else {
		// Use default input
		fdin = dup(tmpin);
	}

	if (_errFile) {
		if (_append) {
			fderr = open(_errFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0600);
		} else {
			fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
		}
	} else {
		fderr = dup(tmperr);
	}

	dup2(fderr,2);
	close(fderr);
	for (unsigned long i = 0; i < _simpleCommands.size(); i++) {
		SimpleCommand *s = _simpleCommands[i];
		const char ** args = (const char **) 
			malloc((s->_arguments.size()+1)*sizeof(char*));
		for (unsigned long j = 0; j < s->_arguments.size(); j++) {
			args[j] = s->_arguments[j]->c_str();
		}

		dup2(fdin, 0);
		close(fdin);
		//setup output
		if (i == _simpleCommands.size()-1){
		// Last simple command
			if(_outFile){
				// open output file, append if necessary
				if (_append) {
					fdout = open(_outFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0600);
				} else {
					fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
				}
			} else {
				// Use default output
				fdout = dup(tmpout);
			}
		} else {
			// Not last
			//simple command
			//create pipe
			int fdpipe[2];
			pipe(fdpipe);
			fdout=fdpipe[1];
			fdin = fdpipe[0];
		}
		// Redirect output
		dup2(fdout,1);
		close(fdout);

		args[s->_arguments.size()] = NULL;
		ret = fork();
		if (ret == 0) {
			execvp(args[0], (char* const*)args);
			perror("execvp");
			exit(1);
		}
    	}
    	dup2(tmpin, 0);
	dup2(tmpout, 1);
	dup2(tmperr, 2);
	close(tmpin);
	close(tmpout);
	close(tmperr);
   	if(!_background){
		waitpid(ret,NULL,0);
	}
	 // Clear to prepare for next command
    	clear();

    	// Print new prompt
    	Shell::TheShell->prompt();
}	

// Expands environment vars and wildcards of a SimpleCommand and
// returns the arguments to pass to execvp.
char ** 
PipeCommand::expandEnvVarsAndWildcards(SimpleCommand * simpleCommandNumber)
{
    simpleCommandNumber->print();
    return NULL;
}


