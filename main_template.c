#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include "utils.h"

/*
In this project, you are going to implement a number of functions to
create a simple linux shell interface to perform basic linux commands
*/

//DEFINE THE FUNCTION PROTOTYPES
void user_prompt_loop();
char * get_user_command();
char ** parse_command(char * , FILE * );
void execute_command(char ** );
//helper functions
int counter(char ** );
void freeArray(char ** arr);
int convertToInt(char * str);

int main(int argc, char **argv)
{
    /*
    Write the main function that checks the number of argument passed to ensure
    no command-line arguments are passed; if the number of argument is greater
    than 1 then exit the shell with a message to stderr and return value of 1
    otherwise call the user_prompt_loop() function to get user input repeatedly
    until the user enters the "exit" command.
    */

    /*
    ENTER YOUR CODE HERE
    */
    if (argc >  1){
        fprintf(stderr, "Error: Too many command-line arguments\n");
        return 1;
    }
    else{
        user_prompt_loop();
    }
    return 0;
}

/*
user_prompt_loop():
Get the user input using a loop until the user exits, prompting the user for a command.
Gets command and sends it to a parser, then compares the first element to the two
different commands ("/proc", and "exit"). If it's none of the commands,
send it to the execute_command() function. If the user decides to exit, then exit 0 or exit
with the user given value.
*/

void user_prompt_loop(){
// initialize variables

/*
loop:
    1. prompt the user to type command by printing >>
    2. get the user input using get_user_command() function
    3. parse the user input using parse_command() function
    Example:
        user input: "ls -la"
        parsed output: ["ls", "-la", NULL]
    4. compare the first element of the parsed output to "/proc"and "exit"
    5. if the first element is "/proc" then you have the open the /proc file system
       to read from it
        i) concat the full command:
            Ex: user input >>/proc /process_id_no/status
                concated output: /proc/process_id_no/status
        ii) read from the file line by line. you may user fopen() and getline() functions
        iii) display the following information according to the user input from /proc
            a) Get the cpu information if the input is /proc/cpuinfo
            - Cpu Mhz
            - Cache size
            - Cpu cores
            - Address sizes
            b) Get the number of currently running processes from /proc/loadavg
            c) Get how many seconds your box has been up, and how many seconds it has been idle from /proc/uptime
            d) Get the following information from /proc/process_id_no/status
            - the vm size of the virtual memory allocated the vbox
            - the most memory used vmpeak
            - the process state
            - the parent pid
            - the number of threads
            - number of voluntary context switches
            - number of involuntary context switches
            e) display the list of environment variables from /proc/process_id_no/environ
            f) display the performance information if the user input is /proc/process_id_no/sched
    6. if the first element is "exit" the use the exit() function to terminate the program
    7. otherwise pass the parsed command to execute_command() function
    8. free the allocated memory using the free() function
*/

/*
Functions you may need:
    get_user_command(), parse_command(), execute_command(), strcmp(), strcat(),
    strlen(), strncmp(), fopen(), fclose(), getline(), isdigit(), atoi(), fgetc(),
    or any other useful functions
*/

	while (1){
        printf(">> ");
        char * input = get_user_command();
        int inputLen = strlen(input);
        //check if empty
        if (inputLen == 0) {
            printf("no input\n");
            free(input); //free if empty
        }else{
            char ** inputArray = parse_command(input, stderr);
            //print array to check if correctly tokenized
            for (int i = 0; inputArray[i] != NULL; i++) {
                printf("[%d] %s\n", i, inputArray[i]);
            }

            if (strcmp(inputArray[0], "exit") == 0 ){
                //if only 1 argument, exit with code 0
                if (counter(inputArray) == 1) {
                    freeArray(inputArray);
                    exit(0);
                }
                    //if more than 2 argument, print error
                else if (counter(inputArray) > 2) {
                    printf("invalid number of arguments after exit \n");
                    freeArray(inputArray);
                }
                else {
                    int return_code = convertToInt(inputArray[1]);
                    // if return_code function returns -1, the second argument was not a number
                    if (return_code == -1) {
                        printf("invalid exit code: return code must be an int\n");
                        freeArray(inputArray);

                    }
                        //else exit with return code
                    else {
                        printf("exiting with code: ");
                        printf("%d", return_code);
                        printf("\n");
                        freeArray(inputArray);
                        exit(return_code);
                    }

                }
            }
        }
    }
}

/*
get_user_command():
Take input of arbitrary size from the user and return to the user_prompt_loop()
*/

char * get_user_command(){
    /*
    Functions you may need:
        malloc(), realloc(), getline(), fgetc(), or any other similar functions
    */

    //create null char and unsigned int
    char* user_input = NULL;
    size_t inputSize = 0;

    //use the getline function to get user input
    ssize_t inputBytes = getline(&user_input, &inputSize, stdin);

    if (inputBytes == -1) {
        fprintf(stderr, "Error reading user input\n");
        free(user_input);
        return NULL;
    }
    // Remove trailing newline character if present
    if (inputBytes > 0 && user_input[inputBytes - 1] == '\n') {
        user_input[inputBytes - 1] = '\0'; //add null to the end
    }
    return user_input;
}

/*
parse_command():
Take command grabbed from the user and parse appropriately.
Example:
    user input: "ls -la"
    parsed output: ["ls", "-la", NULL]
Example:
    user input: "echo     hello                     world  "
    parsed output: ["echo", "hello", "world", NULL]
*/

char ** parse_command(char * str, FILE * stderr){
/*
Functions you may need:
    malloc(), realloc(), free(), strlen(), first_unquoted_space(), unescape()
*/

	//unescape the entered line and store it in a new line
	char * newStr = unescape(str, stderr);
    	free(str);
    	int strBuffer = 128; //set initial buffer
    	char ** tempArr = malloc(sizeof(char * ) * strBuffer); //allocate memory to the array
    	int max = strBuffer;
    	int index = 0;
    	char* strToken;
    	strToken = strtok(newStr, " "); //break string at space

    	//loop over
    	while (strToken != NULL){
        	//if size becomes equal to the memory allocated, increase size
        	if (index == max) {
           		 max = max + strBuffer;
            		tempArr = realloc(tempArr, sizeof(char * ) * max);

        	}
        	//store the string at index in the array
        	tempArr[index] = strToken;
        	index++;

        	strToken = strtok(NULL, " ");
    	}
    	//set last element to null
    	tempArr[index] = NULL;
	return tempArr;
}

/*
execute_command():
Execute the parsed command if the commands are neither /proc nor exit;
fork a process and execute the parsed command inside the child process
*/

void execute_command(char ** array){
/*
Functions you may need:
    fork(), execvp(), waitpid(), and any other useful function
*/

/*
ENTER YOUR CODE HERE
*/

}

//HELPER FUNCTIONS
int counter(char ** arr){
    int counter = 0;
    int i;
    for  (i = 0; arr[counter] != NULL; i++){
        counter++;
        i++;
    }
    return counter;
}

void freeArray(char ** arr){
    int i;
    for  (i = 0; arr[i] != NULL; i++){
        free(arr[i]);
        i++;
    }
    free(arr);
}

int convertToInt(char * str){

    // Loop over each character of the string
    for (int i = 0; str[i] != '\0'; i++) {
        // Check if the character is not a digit using
        if (!isdigit(str[i])) {
            return -1;
        }
    }
    // Convert the string to int
    int result = atoi(str);
    return result;
}
