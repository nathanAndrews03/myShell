
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <cstring>
#include <iterator>
#include <iostream>
#include <cassert>
#include <regex.h>
#include <limits.h>
#include <dirent.h>
#include <algorithm>
#include <string>
#include <sstream>

#include "Command.hh"
#include "SimpleCommand.hh"
#include "IfCommand.hh"
#include "PipeCommand.hh"
#include "Shell.hh"


//std::stack<IfCommand*> IfCommand::ifStack;

IfCommand::IfCommand() {
    _condition = NULL;
    _listCommands =  NULL;
    isWhileLoop = false;
    isForLoop = false;
    forArg = "";
    //dontClear = true;
}


char * processArg(const char *arg) {
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

SimpleCommand* IfCommand::expandSimple(SimpleCommand *cond) {
        maEntries = 20;
        nEntrie = 0;

        entrie = (char **) malloc(maEntries * sizeof(char*));
	//entrie = (char **) calloc(maEntries, sizeof(char*));

        if (!entrie) {
                std::cerr << "failed allocation" << std::endl;
                return NULL;
        }

        SimpleCommand* newCommand = new SimpleCommand();
        SimpleCommand *s = cond;
	for (unsigned long j = 0; j < s->_arguments.size(); j++) {
                char* tempArg = processArg(s->_arguments[j]->c_str());
                std::string arg = tempArg;  
                delete[] tempArg;
		if(strchr(arg.c_str(), '*') || strchr(arg.c_str(), '?')) {
                        char * writeArg = strdup(arg.c_str());
                        expandWildcards(NULL, writeArg);
                        free(writeArg);
                        if (nEntrie == 0) {
                                std::string *newArg = new std::string(arg);
                                newCommand->insertArgument(newArg);
                        }
                        for (int i = 0; i < nEntrie; i++) {
                                std::string* newArg = new std::string(entrie[i]);
                                newCommand->insertArgument(newArg);
                        }
                } else {
                        std::string *newArg = new std::string(arg);
                        newCommand->insertArgument(newArg);
                }
        }
        for (int i = 0; i < nEntrie; i++) {
                free(entrie[i]);
        }
        free(entrie);
        return newCommand;
}



// Run condition with command "test" and return the exit value.
int
IfCommand::runTest(SimpleCommand * s) {
    
	int status;
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		return 1;
	}
    	if (pid == 0) {
        	// Child process
		//SimpleCommand *s = expandSimple(condition);
		std::vector<const char*> args;
        	args.push_back("test"); // Add the command name

        	for (const auto& arg : s->_arguments) {
           		args.push_back(processArg(arg->c_str())); // Add each argument
        	}
        	args.push_back(nullptr); // Null-terminate the argument list

        	// Convert vector of const char* to char**
        	char** argv = new char*[args.size()];
        	for (size_t i = 0; i < args.size(); ++i) {
            		argv[i] = const_cast<char*>(args[i]);
        	}
		execvp(argv[0], argv);
		perror("execvp");
		delete[] argv;
        	exit(EXIT_FAILURE);
   	 } else if (pid > 0) {
        	waitpid(pid, &status, 0);
        	if (WIFEXITED(status)) {
        		return WEXITSTATUS(status);
       		}
	}
    	return 1; 
}


int comp(const void *a, const void *b) {
        return strcmp(*(const char **)a, *(const char **)b);
}


char **
IfCommand::wildcardIfNecessary(SimpleCommand *s) {
        maEntries = 20;
        nEntrie = 0;
        printf("max = %d\n n = %d\n", maEntries, nEntrie);
        entrie = (char **) malloc(maEntries * sizeof(char*));

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
                        printf("max = %d\n n = %d\n", maEntries, nEntrie);
                        if (nEntrie == maEntries) {
                                maEntries *= 2;
                                entrie = (char**) realloc(entrie, maEntries * sizeof(char*));
                                assert(entrie != NULL);
                        }
                        entrie[nEntrie++] = strdup(arg);
                }
                delete[] arg;

        }
        //entries = (const char**) realloc(entries, nEntries * sizeof(char*));
        entrie[nEntrie] = NULL;
        return entrie;
}

void
IfCommand::expandWildcards(char * prefix, char * arg)
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
                int initial = nEntrie;
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
                                        if (nEntrie == maEntries) {
                                                maEntries *= 2;
                                                entrie = (char **)realloc(entrie, maEntries * sizeof(char *));
                                        }
                                        char *argument = (char*)malloc(100);
                                        argument[0] = '\0';
                                        if (prefix) sprintf(argument, "%s/%s", prefix, ent->d_name);

                                        if (ent->d_name[0] == '.') {
                                                if (arg[0] == '.') {
                                                        entrie[nEntrie++] =
                                                                (argument[0] != '\0')?strdup(argument):strdup(ent->d_name);
                                                }
                                        } else {
                                                entrie[nEntrie++] =
                                                        (argument[0] != '\0')?strdup(argument):strdup(ent->d_name);
                                        }
                                        free(argument);
                                        //char *argument = (char *)malloc(strlen(prefix) + strlen(ent->d_name) + 2);
                                        //sprintf(argument, "%s%s%s",
                                        //              prefix, prefix[strlen(prefix)-1] == '/' ? "" : "/", ent->d_name);
                                        //entries[nEntries++] = argument;
                                }
                        }
                }
                closedir(d);
                regfree(&regex);
                free(toOpen);

                if (nEntrie > initial) {
                        qsort(entrie + initial, nEntrie - initial, sizeof(char*), comp);
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


void 
IfCommand::insertCondition( SimpleCommand * condition ) {
    _condition = condition;
}

void 
IfCommand::insertListCommands( ListCommands * listCommands) {
    _listCommands = listCommands;
}

void 
IfCommand::clear() {
	this->_condition->clear();
	this->_listCommands->clear();
}

void 
IfCommand::print() {
    if (isWhileLoop) {
    	printf("While [ \n"); 
    	this->_condition->print();
	printf(" ]; do\n");
	if (this->_listCommands != NULL) {
    		this->_listCommands->print();
	}

    } else {
	printf("list if IF? If [ ");
	this->_condition->print();
    	printf(" ]; then\n");
	if (this->_listCommands != NULL) {
    		this->_listCommands->print();
	}
	
    }
}
  
void 
IfCommand::execute() {
   //int t = runTest(this->_condition); 
    //this->print();
    if (isWhileLoop) {
        // Execute while loop
	//
	Shell::TheShell->dontClear = true;
	ListCommands *list = new ListCommands();
	list = _listCommands;
	SimpleCommand *cond = new SimpleCommand();
	cond = this->_condition;
        while (runTest(cond) == 0) {
	    //if (_listCommands != NULL) {
	    	//this->print();
		//
		//printf("LIST before:\n");
		//list->print();
            	list->execute();
		//printf("LIST after:\n");
		//list->print();
		//printf("COND after:\n");
		//cond->print();
	    //}
        }
    } else if (isForLoop) {
	Shell::TheShell->dontClear = true;
	ListCommands *list = new ListCommands();
	list = _listCommands;
	SimpleCommand *cond = new SimpleCommand();
	cond = 	this->_condition;
	//cond = expandSimple(cond);
	//cond = this->_condition;
	for (auto& str : cond->_arguments) {
	   if (str != nullptr) {
		setenv(this->forArg.c_str(), str->c_str(), 1);
		list->execute();
	   }
	}
    } else {
        // Execute if statement
        if (runTest(this->_condition) == 0) {
	    //if (_listCommands != NULL) {
                _listCommands->execute();
	    //}
        }
    }
    //Shell::TheShell->dontClear = false;
    //this->clear();
    Shell::TheShell->clear();
}

