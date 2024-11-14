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
#include <limits.h>
#include <regex.h>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <cassert>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <sstream>
#include <string>

#include "PipeCommand.hh"
#include "Shell.hh"

extern char **environ;
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
    	_outFile = NULL;
    } 
    if ( _errFile ) {
        delete _errFile;
    	_errFile = NULL;
    } 
    if ( _inFile ) {
        delete _inFile;
    	_inFile = NULL;
    }
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

















int PipeCommand::built(const char **args) {
	if (!strcmp( args[0], "setenv" ) ) { 
		// add your code to set the environment variable 
		//if (_simpleCommands[i]->_arguments.size() != 3) {
		//	perror("setenv");	
		//}
		int err = setenv(args[1], args[2], 1);
		if (err) {
			perror("setenv");
		}

		Shell::TheShell->clear();
		Shell::TheShell->prompt();
		return 1;
	} 

	if (!strcmp(args[0], "unsetenv")) {
		if (args[1] == NULL) {
			perror("unsetenv");
		}
		unsetenv(args[1]);

		Shell::TheShell->clear();
		Shell::TheShell->prompt();
		return 1;
	}	

	if (!strcmp(args[0], "cd")) {
		int tmp;
		if (args[1] == NULL) {
			tmp = chdir(getenv("HOME"));
		} else {
			//const char *dirPath = _simpleCommands[0]->_arguments[1]->c_str();
			char* tempPath = processArg(args[1]);
			tmp = chdir(tempPath);
			delete[] tempPath;
			//const char *path = processArg(_simpleCommands[i]->_arguments[1]->c_str());
		}
		if (tmp < 0) {
			const char *path = args[1];
			char message[1024] = "cd: can't cd to ";
			strcat(message, path);
			perror(message);
		}
		Shell::TheShell->clear();
		Shell::TheShell->prompt();
		return 1;
	}
	return 0;
}

char * PipeCommand::processArg(const char *arg) {
    std::string str = arg;
    size_t startPos = 0;
    char *varValue;

    while ((startPos = str.find("${", startPos)) != std::string::npos) {
        size_t endPos = str.find("}", startPos);
        if (endPos == std::string::npos) {
            break; // No closing brace found, exit the loop
        }

        // Extract the environment variable name
        std::string varName = str.substr(startPos + 2, endPos - startPos - 2);
        
        // Get the environment variable value
	
	if (!strcmp(varName.c_str(), "_")) {
		varName = "_env";	
	}

	varValue = std::getenv(varName.c_str());
        if (varValue) {
            // Replace the placeholder with the environment variable value
            str.replace(startPos, endPos - startPos + 1, varValue);
        } else {
            // Environment variable not found, move past this placeholder
            startPos = endPos + 1;
        }
    }
    char* modifiableStr = new char[str.length() + 1]; // +1 for null terminator
    strcpy(modifiableStr, str.c_str());
    return modifiableStr;
    //return str.c_str();
} 

bool containsSub(std::string &arg) {
	size_t start, end;


	start = arg.find('`');
	if (start != std::string::npos) {
		end = arg.find('`', start+1);
		if (end != std::string::npos) {
			arg = arg.substr(start + 1, end - start - 1);
			return true;
		}	
	}
	
	start = arg.find("$(");
   	if (start != std::string::npos) {
        	end = arg.find(')', start+2);
       		if (end != std::string::npos) {
			arg = arg.substr(start + 2, end - start - 2);
            		return true;
        	}
    	}
	return false;
}

std::string executeSub(const std::string &command) {
	int pin[2], pout[2];
	//int tempin, tempout;
	pid_t pid;
	//std::string output;
	//tempin = dup(0);
	//tempout = dup(1);
	pipe(pin);
	pipe(pout);

	pid = fork();
	if (pid == 0) {
		close(pin[1]);
		close(pout[0]);
		
		dup2(pin[0], 0);
		dup2(pout[1], 1);

		//char **args = new char*[2];
		//args[0] = (char*)"/proc/self/exe";
		//args[1] = NULL;
		//execvp(args[0], args);
		execlp("/proc/self/exe", "/proc/self/exe", nullptr);
		perror("execlp");
		exit(1);
	} else if (pid < 0) {
		perror("fork");
		exit(1);
	}
/*	dup2(tempin, 0);
	dup2(tempout, 1);
	close(tempin);
	closse(tempout); */
	close(pin[0]);
	close(pout[1]);

	write(pin[1], command.c_str(), command.length());
	write(pin[1], "\nexit\n", 6);
	close(pin[1]);

       	std::ostringstream output;
        char buf[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(pout[0], buf, sizeof(buf))) > 0) {
            //output.write(&buffer, bytesRead); // Write to output stringstream
	    buf[bytesRead] = '\0';
	    output << buf;
        }	
	close(pout[0]);
	waitpid(pid, NULL, 0);

	return output.str();
	
}

std::string processSub(const std::string& output) {
    std::string processedOutput;
    for (char c : output) {
        if (c == '\n') {
            processedOutput += ' ';  // Replace newline with space
        } else {
            processedOutput += c;
        }
    }
    return processedOutput;
}













SimpleCommand* PipeCommand::expandSimple(int i) {
	maxEntries = 20;
	nEntries = 0;
	//std::cout << "Before malloc, getenv('a'): " << (getenv("a") ? getenv("a") : "NULL") << std::endl;

	entries = (char **) malloc(maxEntries * sizeof(char*));
	if (!entries) {
		std::cerr << "failed allocation" << std::endl;
		return NULL;
	}
	//std::cout << "After malloc, getenv('a'): " << (getenv("a") ? getenv("a") : "NULL") << std::endl;


	SimpleCommand* newCommand = new SimpleCommand();
	SimpleCommand *s = _simpleCommands[i];
	for (unsigned long j = 0; j < s->_arguments.size(); j++) {
		char* tempArg = processArg(s->_arguments[j]->c_str());
		std::string arg = tempArg;  // Now the std::string 'arg' holds a copy of the content.
		delete[] tempArg;
		//std::string arg = processArg(s->_arguments[j]->c_str());
		if (containsSub(arg)) {
			//execute subshell
			std::string subOut = executeSub(arg);
			subOut = processSub(subOut);
			std::vector<std::string> splitArg;
			std::stringstream ss(subOut);
			std::string word;
			while (ss >> word) {
				splitArg.push_back(word);
			}
			for (const auto &a : splitArg) {
				std::string *newArg = new std::string(a);
				newCommand->insertArgument(newArg);
			}
		} else if(strchr(arg.c_str(), '*') || strchr(arg.c_str(), '?')) {
			char * writeArg = strdup(arg.c_str());
			expandWildcards(NULL, writeArg);
			free(writeArg);
			if (nEntries == 0) {
				std::string *newArg = new std::string(arg);
				newCommand->insertArgument(newArg);
			}
			for (int i = 0; i < nEntries; i++) {
				std::string* newArg = new std::string(entries[i]);
				newCommand->insertArgument(newArg);
			}
		} else {
			std::string *newArg = new std::string(arg);
			newCommand->insertArgument(newArg);
		}
	}
	for (i = 0; i < nEntries; i++) {
		free(entries[i]);
	}
	free(entries);
	return newCommand;
}

void PipeCommand::execute() {
    	// Don't do anything if there are no simple commands
    	if ( _simpleCommands.size() == 0 ) {
        	Shell::TheShell->prompt();
        	return;
    	}
	    	// Print contents of PipeCommand data structure
    	//if (isatty(0)) print();
	char real_path[PATH_MAX];
	realpath(path, real_path);
	setenv("SHELL", real_path, 1);

	pid_t pid = getpid();
	char pidS[20];
	sprintf(pidS, "%d", pid);
	setenv("$", pidS, 1);



   	// Add execution here
    	// For every simple command fork a new process
    	// Setup i/o redirection
    	// and call exec
    	int ret;
    	//save in/out
	int tmpin=dup(0);
	int tmpout=dup(1);
	int tmperr=dup(2);
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
		// open err file, appennd if set to append
		if (_append) {
			fderr = open(_errFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0600);
		} else {
			fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
		}

		if (fderr < 0) {
			perror("open");
			exit(1);
		}
	} else {
		// Use default output
		fderr = dup(tmperr);
	}
	SimpleCommand *s;


	for (unsigned long i = 0; i < _simpleCommands.size(); i++) {
		// set, unset, cd
		
		s = expandSimple(i); //_simpleCommands[i];
		const char ** args = (const char**) malloc((s->_arguments.size()+1)*sizeof(char*));
		unsigned long j;
		for (j = 0; j < s->_arguments.size(); j++) {
			args[j] = s->_arguments[j]->c_str();
		}
		args[j] = NULL;
		if (built(args)) return;
		dup2(fdin, 0);
		close(fdin);
		//setup output
		if (i == _simpleCommands.size()-1) {
			if (_outFile){
				// open output file, append if set to append
				if (_append) {
					fdout = open(_outFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0600);
				} else {
					fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
				}
				if (fdout < 0) {
					perror("open");
					exit(1);
				}

			} else {
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
		dup2(fderr, 2);
		close(fderr);

		//args[nEntries] = NULL;
		ret = fork();
		if (ret == 0) {

			if (!strcmp(args[0], "printenv")) {
				char **env = environ;
				while(*env) {
					printf("%s\n", *env);
					env++;
				}
				exit(0);
			} else {

				execvp(args[0], (char* const*)args);
				perror("execvp");
				exit(1);
			}
		} else if (ret < 0) {
			perror("fork");
			exit(1);
		}
		free(args);
		setenv("_env", _simpleCommands[i]->_arguments[_simpleCommands[i]->_arguments.size() -1]->c_str(), 1);//lastArg = args[i];
	        delete s;
    	}
    	dup2(tmpin, 0);
	dup2(tmpout, 1);
	dup2(tmperr, 2);
	close(tmpin);
	close(tmpout);
	close(tmperr);
	int status;
   	if(!_background) {
		waitpid(ret,&status,0);
		if (WIFEXITED(status)) {
			char exitS[20];
			sprintf(exitS, "%d", WEXITSTATUS(status));
			setenv("?", exitS, 1); //return_code = WEXITSTATUS(status);
		}
	} else {
		char retS[20];
		sprintf(retS, "%d", ret);
		setenv("!", retS, 1); //backG = ret;
	}
    	//clear();
    	// Print new prompt
    	//Shell::TheShell->prompt();
}


int compare(const void *a, const void *b) {
	return strcmp(*(const char **)a, *(const char **)b);
}


char **
PipeCommand::wildcardIfNecessary(SimpleCommand *s) {
	maxEntries = 20;
	nEntries = 0;
	printf("max = %d\n n = %d\n", maxEntries, nEntries);
	entries = (char **) malloc(maxEntries * sizeof(char*));

	for (size_t i=0; i < s->_arguments.size(); i++) {
		pid_t pid = getpid();
		char pidS[20];
		sprintf(pidS, "%d", pid);
		setenv("$", pidS, 1);
		//char * arg = s->_arguments[i]->c_str();
		char *arg = processArg(s->_arguments[i]->c_str());

		if (strchr(arg, '*') || strchr(arg, '?')) {
			expandWildcards(NULL, arg);
		} else {
			printf("max = %d\n n = %d\n", maxEntries, nEntries);
			if (nEntries == maxEntries) {
				maxEntries *= 2;
				entries = (char**) realloc(entries, maxEntries * sizeof(char*));
				assert(entries != NULL);
			}
			entries[nEntries++] = strdup(arg);
		}
		delete[] arg;

	}
	//entries = (const char**) realloc(entries, nEntries * sizeof(char*));
	entries[nEntries] = NULL;
	return entries;
}









// Expands environment vars and wildcards of a SimpleCommand and
// returns the arguments to pass to execvp.
void
PipeCommand::expandWildcards(char * prefix, char * arg)
{
	char *tmp = arg;
	char *save = (char *)malloc(strlen(arg)+10);
	char *dir = save;

	if (tmp[0] == '/') *(save++) =  *(tmp++);
	while (*tmp != '/' && *tmp) *(save++) = *(tmp++);
	*save = '\0';

	if (strchr(dir, '*') || strchr(dir, '?')) {
		if (!prefix && arg[0] == '/') {
			prefix = strdup("/");
			dir++;
		}
		
		char *reg = (char*) malloc (2*strlen(arg)+10);
		char *a = dir;
		//free(dir);
		//dir = NULL;
		//save = NULL;
		char *r = reg;
		*r = '^'; r++;
		while (*a) {
			if (*a == '*') { *(r++)='.'; *(r++)='*'; }
			else if (*a == '?') { *(r++)='.'; }
			else if (*a == '.') { *(r++)='\\'; *(r++)='.'; }
			else { *(r++)=*a;}
			a++;
		}
		*(r++)='$'; *r='\0';

		regex_t regex;
		if (regcomp(&regex, reg, REG_EXTENDED | REG_NOSUB)) {
			perror("compile");
			free(reg);
			exit(1);
		}
		free(reg);
		char *toOpen = strdup((prefix)?prefix:".");
		DIR *d = opendir(toOpen);
		if (d == NULL) {
			perror("opendir");
			//free(reg);
			free(toOpen);
			//free(save);
			exit(1);
		}
		struct dirent *ent;
		int initial = nEntries;
		regmatch_t match;
		while ((ent = readdir(d)) != NULL) {
			if (!regexec(&regex, ent->d_name, 1, &match, 0)) {
				if (*tmp) {
					// process ent as a directory
					// */*
					if (ent->d_type == DT_DIR) {
						char *nPrefix = (char *) malloc (150);
						if (!strcmp(toOpen, ".")) nPrefix = strdup(ent->d_name);
						else if (!strcmp(toOpen, "/")) sprintf(
								nPrefix, "%s%s", toOpen, ent->d_name);
						else sprintf(nPrefix, "%s/%s", toOpen, ent->d_name);
						expandWildcards(nPrefix, (*tmp == '/')?++tmp:tmp);
						free(nPrefix);

					}
					//free(prefix);
				} else {
					if (nEntries == maxEntries) {
						maxEntries *= 2;
						entries = (char **)realloc(entries, maxEntries * sizeof(char *));
					}
					char *argument = (char*)malloc(100);
					argument[0] = '\0';
					if (prefix) sprintf(argument, "%s/%s", prefix, ent->d_name);

					if (ent->d_name[0] == '.') {
						if (arg[0] == '.') {
							entries[nEntries++] = 
								(argument[0] != '\0')?strdup(argument):strdup(ent->d_name);
						}
					} else {
						entries[nEntries++] = 
							(argument[0] != '\0')?strdup(argument):strdup(ent->d_name);
					}
					free(argument);
					//char *argument = (char *)malloc(strlen(prefix) + strlen(ent->d_name) + 2);
					//sprintf(argument, "%s%s%s",
					//		prefix, prefix[strlen(prefix)-1] == '/' ? "" : "/", ent->d_name);
					//entries[nEntries++] = argument;
				}
			}
		}
		closedir(d);
		regfree(&regex);
		free(toOpen);

		if (nEntries > initial) {
			qsort(entries + initial, nEntries - initial, sizeof(char*), compare);
		}
	} else {
		if (*tmp) {
			char *pre = (char*)malloc(100);
			if (prefix) sprintf(pre, "%s/%s", prefix, dir);
			else pre = strdup(dir);
			if (*tmp) expandWildcards(pre, ++tmp);
			free(pre);
		}
	}


}


