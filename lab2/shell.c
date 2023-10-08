// Additional libraries imported for various functionalities
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>


//+
// File:	shell.c
//
// Pupose:	This program implements a simple shell program. It does not start
//		processes at this point in time. However, it will change directory
//		and list the contents of the current directory.
//
//		The commands are:
//		   cd name -> change to directory name, print an error if the directory doesn't exist.
//		              If there is no parameter, then change to the home directory.
//		   ls -> list the entries in the current directory.
//			      If no arguments, then ignores entries starting with .
//			      If -a then all entries
//		   pwd -> print the current directory.
//		   exit -> exit the shell (default exit value 0)
//				any argument must be numeric and is the exit value
//
//		if the command is not recognized an error is printed.
//-

// Maximum buffer size for command and maximum arguments allowed
#define CMD_BUFFSIZE 1024
#define MAXARGS 10

// Function prototypes
int splitCommandLine(char * commandBuffer, char* args[], int maxargs);
int doInternalCommand(char * args[], int nargs);
int doProgram(char * args[], int nargs);

//+
// Function:	main
//
// Purpose:	The main function. Contains the read
//		eval print loop for the shell.
//
// Parameters:	(none)
//
// Returns:	integer (exit status of shell)
//-

int main() {
    // Initialize variables
    char commandBuffer[CMD_BUFFSIZE];
    // note the plus one, allows for an extra null
    char *args[MAXARGS+1];
    int nargs;

    // print prompt.. fflush is needed because
    // stdout is line buffered, and won't
    // write to terminal until newline

    // Main command loop
    printf("%%> ");
    fflush(stdout);
    while(fgets(commandBuffer,CMD_BUFFSIZE,stdin) != NULL){
        //printf("%s",commandBuffer);

	// remove newline at end of buffer
	int cmdLen = strlen(commandBuffer);
	if (commandBuffer[cmdLen-1] == '\n'){
	    commandBuffer[cmdLen-1] = '\0';
	    cmdLen--;
            //printf("<%s>\n",commandBuffer);
	}

    // Split the command line into arguments
    nargs = splitCommandLine(commandBuffer, args, MAXARGS);

    // Check if command is internal or external
    if (nargs > 0) {
             int internalCommandStatus = doInternalCommand(args, nargs); // Step 3
        if (internalCommandStatus == 0) {
            int programStatus = doProgram(args, nargs);
        if (programStatus == 0) {
            printf("Command not found.\n");
        }
    }
        }

	// debugging
	//printf("%d\n", nargs);
	//int i;
	//for (i = 0; i < nargs; i++){
	//   printf("%d: %s\n",i,args[i]);
	//}
	// element just past nargs
	//printf("%d: %x\n",i, args[i]);

	// print prompt
	printf("%%> ");
	fflush(stdout);
    }
    return 0;
}

////////////////////////////// String Handling (Step 1) ///////////////////////////////////

//+
// Function:	skipChar
//
// Purpose:	This function skips over a given char in a string
//		For security, will not skip null chars.
//
// Parameters:
//    charPtr	Pointer to string
//    skip	character to skip
//
// Returns:	Pointer to first character after skipped chars
//		ID function if the string doesn't start with skip,
//		or skip is the null character
//-

char * skipChar(char * charPtr, char skip){
    while(*charPtr == skip && *charPtr != '\0') {
        charPtr++;
    }
    return charPtr;
}

//+
// Function: splitCommandLine
//
// Purpose:	This function takes a command line input buffer and splits it
//              into arguments. It then stores these arguments in an array for
//              further processing.
//
// Parameters:
//      commandBuffer - Pointer to the command line input buffer.
//      args          - Array to store the individual arguments.
//      maxargs       - Maximum number of arguments that can be parsed.
//
// Returns:	Number of arguments parsed and stored in the array.
//-

// Function to split the command line into arguments
int splitCommandLine(char * commandBuffer, char* args[], int maxargs){
   int nargs = 0;

   commandBuffer = skipChar(commandBuffer, ' ');

    // Loop to break the command into tokens (arguments)
   while (*commandBuffer != '\0' && nargs < maxargs) {
    args[nargs] = commandBuffer;
    
   
    char *nextSpace = strchr(commandBuffer, ' ');
    if (nextSpace != NULL) {
        *nextSpace = '\0';
        commandBuffer = nextSpace + 1;
    } else {
        commandBuffer = commandBuffer + strlen(commandBuffer);
    }
    
    commandBuffer = skipChar(commandBuffer, ' ');
    
    nargs++;
}

    args[nargs] = NULL;
    return nargs;
}


////////////////////////////// External Program  (Note this is step 4, complete doeInternalCommand first!!) ///////////////////////////////////

// list of directorys to check for command.
// terminated by null value
char * path[] = {
    ".",
    "/usr/bin",
    NULL
};

//+
// Function: doProgram
//
// Purpose:	This function takes an array of arguments and tries to execute
//              the command specified in args[0]. It searches through the directories
//              listed in the path array to find the executable.
//
// Parameters:
//      args  - Array containing the command and its arguments.
//      nargs - Number of arguments present in the array.
//
// Returns:	int
//		1 = found and executed the file
//		0 = could not find and execute the file
//-

// Function to execute external programs
int doProgram(char * args[], int nargs){

  char *cmd_path = NULL;
  struct stat statbuf;
  
  // Extract just the command name from args[0]
  char *command_name = strrchr(args[0], '/');
  if (command_name) {
    command_name++;  // Skip the '/'
  } else {
    command_name = args[0];
  }

    // Loop to check each directory in PATH for the command
  for (int i = 0; path[i] != NULL; i++) {
      
    cmd_path = malloc(strlen(path[i]) + strlen(command_name) + 2);
    sprintf(cmd_path, "%s/%s", path[i], command_name);
        
    printf("Checking path: %s\n", cmd_path);  // Debug print

    if (stat(cmd_path, &statbuf) == 0) {
      if (S_ISREG(statbuf.st_mode) && (statbuf.st_mode & S_IXUSR)) {
        break;
      }
    }
    free(cmd_path);
    cmd_path = NULL;
  }
  
  if (cmd_path == NULL) {
    return 0;
  }
  
  // Fork and execute the program
  pid_t pid = fork();
  if (pid == 0) {
    execv(cmd_path, args);
    exit(1);  
  } else if (pid > 0) {
    wait(NULL);  
    free(cmd_path);
  } else {
    printf("Fork failed.\n");
    return 0;
  }
  return 1;
}


////////////////////////////// Internal Command Handling (Step 3) ///////////////////////////////////

// define command handling function pointer type
typedef void(*commandFunc)(char * args[], int nargs);

// associate a command name with a command handling function
struct cmdStruct{
   char 	* cmdName;
   commandFunc 	cmdFunc;
};

// prototypes for command handling functions
void exitFunc(char * args[], int nargs);
void pwdFunc(char * args[], int nargs);
void cdFunc(char * args[], int nargs);
void lsFunc(char * args[], int nargs);

// list commands and functions
// must be terminated by {NULL, NULL} 
// in a real shell, this would be a hashtable.
struct cmdStruct commands[] = {
   {"exit", exitFunc},
   {"pwd", pwdFunc},
   {"cd", cdFunc},
   {"ls", lsFunc},
   { NULL, NULL}		// terminator
};

//+
// Function:	doInternalCommand
//
// Purpose:	This function takes an array of arguments and determines if
//              the command specified in args[0] is an internal shell command.
//              If it is, the function executes the corresponding command.
//
// Parameters:
//      args  - Array containing the command and its arguments.
//      nargs - Number of arguments present in the array.
//
// Returns:	int
//		1 = args[0] is an internal command and was executed
//		0 = args[0] is not an internal command
//-

// Function to check and execute internal commands
int doInternalCommand(char * args[], int nargs){
    int i = 0;
    while (commands[i].cmdName != NULL) {
        if (strcmp(commands[i].cmdName, args[0]) == 0) {
            commands[i].cmdFunc(args, nargs);
            return 1; 
        }
        i++;
    }
    return 0;
}

///////////////////////////////
// comand Handling Functions //
///////////////////////////////

// Function to handle 'exit' command
void exitFunc(char * args[], int nargs) {
    exit(0);
}

// Function to handle 'pwd' command
void pwdFunc(char * args[], int nargs) {
    char *cwd = getcwd(NULL, 0);
    printf("%s\n", cwd);
    free(cwd);
}

// Function to handle 'cd' command
void cdFunc(char * args[], int nargs) {
    if (nargs == 1) {
        struct passwd *pw = getpwuid(getuid());
        chdir(pw->pw_dir);
    } else if (nargs == 2) {
        if (chdir(args[1]) != 0) {
            perror("chdir");
        }
    }
}

// Function to handle 'ls' command
void lsFunc(char * args[], int nargs) {
    struct dirent **namelist;
    int n;

    n = scandir(".", &namelist, NULL, NULL);
    if (n < 0) {
        perror("scandir");
    } else {
        while (n--) {
            printf("%s\n", namelist[n]->d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
}

