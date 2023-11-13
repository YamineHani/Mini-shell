
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "command.h"
#include <glob.h>
#include <errno.h>
#include <ctime>

char **argv;

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void 
Command::changeDirectory(int i){
	if (_simpleCommands[i]->_numberOfArguments > 2) {
        		fprintf(stderr, "cd: Too many arguments\n");
    		}
			else{
				if (!(_simpleCommands[i]->_arguments[1])) {
        			chdir("/home/"); // cd command if no dir specified
    			} 
			else {
        		if ((chdir(_simpleCommands[i]->_arguments[1]) == -1)){
            			perror("cd error");
        			}
    			}
			}
}

void Command::Wildcarding(int i,int j){
	glob_t glob_result;

	char* pattern = _simpleCommands[i]->_arguments[j];
	int glob_status = glob(pattern, 0, NULL, &glob_result);
	if (glob_status == 0) {
        // Access the matched filenames through the gl_pathv array
        for (size_t i = 0; i < glob_result.gl_pathc; i++) {
            printf("%s\n", glob_result.gl_pathv[i]);
        }

        // Free the memory allocated by glob
        globfree(&glob_result);
    } else if (glob_status == GLOB_NOMATCH) {
        printf("No match found\n");
    } else {
        fprintf(stderr, "Error during glob: %d\n", glob_status);
    }
}


void removeNewline(char *str, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (str[i] == '\n')
		{
			str[i] = '\0';
			return;
		}
	}
}
void sigchld_handler(int sig_num)
{
	pid_t child_pid;
	int status;
	char path_to_log[64];
	strcpy(path_to_log, argv[1]);
	strcat(path_to_log, "/child-log.txt");
    FILE*logFile = fopen(path_to_log, "a");
    if (logFile == NULL) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }
    	time_t Time = time(NULL);
	tm *time_pointer = localtime((&Time));
	char currentTime[32];
	strcpy(currentTime, asctime(time_pointer));
	 removeNewline(currentTime, 32);
	fprintf(logFile, "%s: Child Terminated\n", currentTime);
     fclose(logFile);

}





void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();
	
	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec

	// Save default input, output, and error because we will
	// change them during redirection and we will need to restore them
	// at the end.
	// The dup() system call creates a copy of a file descriptor.

	int defaultin = dup( 0 ); // Default file Descriptor for stdin
	int defaultout = dup( 1 ); // Default file Descriptor for stdout
	int defaulterr = dup( 2 ); // Default file Descriptor for stderr

    //set initial input
	//If an input file (_inputFile) is specified, it opens the file for reading;
	// otherwise, it duplicates the default input. ---->Default
	int fdin; 
	if(_inputFile){
		fdin = open(_inputFile,O_RDONLY);
	}
	else{
		//use default input
		fdin = dup(defaultin);
	}

	int pid;
	int fdout;
	int fderr;

	for(int i = 0 ; i < _numberOfSimpleCommands ; i++){
		// cd command
		if (strcmp(_simpleCommands[i]->_arguments[0], "cd") == 0) {
			changeDirectory(i);
    		break;
		}

		//redirect input
		//input redirection for a child process.
		// After redirecting the input, the child process will read from the specified file
		// or source as if it were reading from standard input (stdin).
		dup2(fdin, 0);
		close(fdin);
		//setup output
		if(i == _numberOfSimpleCommands - 1){
			//last command
			if(_outFile && !_append){
				fdout = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			}
			else if (_outFile && _append)
			{
				fdout = open( _outFile, O_WRONLY |O_CREAT | O_APPEND, 0666);
				_append = 0 ;
			}
			else{
				//use default output
				fdout = dup(defaultout);
			}
			// Setup error
        	if (_errFile) {
            	fderr = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
        	} else {
            	fderr = dup(defaulterr);
        	}
		}
		else{
			//not last command
			//create pipe
			int fdpipe[2];
			pipe(fdpipe);
			fdout = fdpipe[1];
			fdin = fdpipe[0];

			fderr = dup(fdout); 
		}
		//redirect output
		//This means that anything written to standard output 
		//or standard error by the current command will go through the pipe 
		//and can be read by the next command in the pipeline.
		dup2(fdout,1);
		dup2(fderr,2);
		close(fdout);
		close(fderr);

		//create child process
		int wildcardflag = 0;
		if (strcmp(_simpleCommands[i]->_arguments[0], "echo") == 0){	
			for(int j=0;j<_simpleCommands[i]->_numberOfArguments;j++){
				for(int z=0;z<strlen(_simpleCommands[i]->_arguments[j]);z++){
					if(_simpleCommands[i]->_arguments[j][z] == '*' || _simpleCommands[i]->_arguments[j][z] == '?'){
						Wildcarding(i,j);	
						wildcardflag = 1;
						break;
					}
					if(wildcardflag){
						break;
					}
				}
			}
				if(wildcardflag){
						break;
					}
		}
		pid = fork();
		if(pid ==-1){
			perror("can't fork\n");
		}
		if(pid == 0){
			//child process 
			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
			perror( "error in child\n");
			exit( 2 );
		}
		signal(SIGCHLD,sigchld_handler);
	}
	//restore in out default
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr,2);
	close(fdin);
	close(fdout);
	close(fderr);
	if(!_background){
		//wait for last command
		waitpid(pid,0,0);
	}

	// Clear to prepare for next command
	clear();
	// Print new prompt
	prompt();
}

void signit_handler(int n){
	//signal(SIGINT, signit_handler);
}


// Shell implementation

void
Command::prompt()
{
	signal(SIGINT,signit_handler);
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);



int 
main(int argc, char ** argv)
{
	::argv = argv;
    Command::_currentCommand.prompt();
    yyparse(); 	
	return 0;
}

