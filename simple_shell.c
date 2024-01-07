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
void appendToHiddenFile(const char *command);
#define HISTORY_FILE ".421sh"
#define HISTORY_SIZE 10
#define MAX_COMMAND_LENGTH 100

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
        (void) argv;
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
        //user prompt
        printf("$ ");
        //store input from get_user_command
        char * input = get_user_command();
	int inputLen = strlen(input);
        //make sure input is not history to avoid adding it to the file
	if (inputLen != 0){
		appendToHiddenFile(input);
	}
        //int inputLen = strlen(input);
        //check if empty
        if (inputLen == 0) {
            printf("no user input\n");
            free(input); //free if empty
        }else{
            char ** inputArray = parse_command(input, stderr);
            //print array to check if correctly tokenized
            int k;
            for (k = 0; inputArray[k] != NULL; k++) {
                printf("[%d] %s\n", k, inputArray[k]);
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

            else if (strcmp(inputArray[0], "/proc") == 0){
		char *joined = NULL;
    		int size = 0;
    		// Calculate the total length of the joined string
    		for (int i = 0; inputArray[i] != NULL; i++) {
        		size += strlen(inputArray[i]) + 1; // +1 for space between words
    		}
    		// Allocate memory for the joined string
    		joined = (char *)malloc(size);
    		if (joined == NULL) {
        		perror("Memory allocation failed");
        		exit(EXIT_FAILURE);
    		}
    		// Initialize the joined string
    		joined[0] = '\0';
    		for (int i = 0; inputArray[i] != NULL; i++) {
       			strcat(joined, inputArray[i]);

        		if (inputArray[i + 1] != NULL) {
            			strcat(joined, "/");
        		}

    		}
    		printf("%s\n", joined);
		//open file and display
    		char s;
    		FILE *fp = fopen(joined, "r");
    		if (fp == NULL) {
        		perror("File open failed");
        		free(joined);
        		exit(EXIT_FAILURE);
    		}

    		while ((s = fgetc(fp)) != EOF) {
        		printf("%c", s);
    		}
    		printf("\n");

    		fclose(fp);
    		free(joined);
		//free original input array
		if (inputArray != NULL){
                    freeArray(inputArray);
                }
            }
            else if (strcmp(inputArray[0], "history") == 0){
		//open the history file
		FILE *histFile = fopen(HISTORY_FILE, "r");
                if (histFile == NULL) {
                    perror("Error opening history file");
                    return;
                }
                char *commands[HISTORY_SIZE];
                int temp;
                for (temp = 0; temp < HISTORY_SIZE; temp++) {
                    //allocate memory for command
                    commands[temp] = malloc(MAX_COMMAND_LENGTH * sizeof(char));
                    if (commands[temp] == NULL) {
                        perror("Memory allocation failed");
                        fclose(histFile);
			int n;
                        for (n = 0; n < HISTORY_SIZE; n++) {
                            free(commands[n]);
                        }
                        return;
                    }
                }
                int counter = 0;
                // Read and store the last 10 lines in a buffer
                while (fgets(commands[counter], MAX_COMMAND_LENGTH, histFile) != NULL) {
			commands[counter][MAX_COMMAND_LENGTH - 1] = '\0';
                    	counter = (counter + 1) % HISTORY_SIZE;
                }
                fclose(histFile);
                // Print the last 10 commands in the file
                int i, j;
                for (i = counter, j = 0; j < HISTORY_SIZE; j++) {
                    	printf("%s", commands[i]);
                    	free(commands[i]);
			i = (i + 1) % HISTORY_SIZE;
                }
		if (inputArray != NULL){
                    freeArray(inputArray);
                }
            }
            else{
                //call execute command
                execute_command(inputArray);
                if (inputArray != NULL){
                    freeArray(inputArray);
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
    // Unescape the entered line and store it in a new line
    char *newStr = unescape(str, stderr);
    free(str);  // Free the original string
    int strBuffer = 128; // Set initial buffer
    int max = strBuffer;
    int index = 0;

    char **tempArr = malloc(sizeof(char *) * max); // Allocate memory to the array
    if (tempArr == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Tokenize the unescaped string
    char *strToken = strtok(newStr, " ");
    while (strToken != NULL) {
        // If size becomes equal to the memory allocated, increase size
        if (index == max - 1) {
            max += strBuffer;
            tempArr = realloc(tempArr, sizeof(char *) * max);
            if (tempArr == NULL) {
                fprintf(stderr, "Memory allocation failed.\n");
                exit(EXIT_FAILURE);
            }
        }

        // Allocate memory for the token and store it in the array
        tempArr[index] = malloc(strlen(strToken) + 1);
        if (tempArr[index] == NULL) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        strcpy(tempArr[index], strToken);

        index++;
        strToken = strtok(NULL, " ");
    }
    free(newStr);
    // Set the last element to null
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
    pid_t pid = fork();
    if (pid < 0){
        printf("Fork Failed\n");
        exit(EXIT_FAILURE);
    }
    if (pid == 0){
        if (execvp(array[0], array) == -1) {
            printf("Error: invalid command, child process execvp failed\n");
		freeArray(array);
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0){
        int currStatus;
        if (waitpid(pid, &currStatus, 0) == -1) {
            printf("Error: parent process waitpid failed\n");
            exit(EXIT_FAILURE);
        }
    }

}

//HELPER FUNCTIONS
int counter(char ** arr){
    //loop over the array and count number of elements
    int count = 0;
    int i = 0;
    while(arr[i] != NULL){
        count++;
        i++;
    }
    return count;

}

void freeArray(char ** arr){
    //check to make sure not null
    if (arr == NULL) {
        return; // Nothing to free
    }
    int i;
    //loop over the array and free
    for  (i = 0; arr[i] != NULL; i++){
        free(arr[i]);
        arr[i] = NULL;
        //i++;
    }
    free(arr);

}

int convertToInt(char * str){
    // Loop over each character of the string
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        // Check if the character is not a digit using
        if (!isdigit(str[i])) {
            return -1;
        }
    }
    // Convert the string to int
    int result = atoi(str);
    return result;
}

void appendToHiddenFile(const char *command) {
    //open the file and write to it
    FILE *histFile = fopen(HISTORY_FILE, "a");
    if (histFile == NULL) {
        perror("Error opening history file");
        return;
    }
    fprintf(histFile, "%s\n", command);
    fclose(histFile);
}
