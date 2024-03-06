#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int board[9][9];
int solution = 1;

// Function to check if a number is present in a column
int numberIsInCol(int check, int col) {
    for(int i = 0; i < 9; ++i) {
        if(board[i][col] == check) {
            return 1;
        }
    }
    return -1;
}

// Function to check if a number is present in a row
int numberIsInRow(int check, int row) {
    for(int j = 0; j < 9; ++j) {
        if(board[row][j] == check) {
            return 1;
        }
    }
    return -1;
}

// Function to check if a number is present in a subgrid
int numberIsInSubgrid(int check, int startRow, int startCol){
    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            if(board[startRow + i][startCol + j] == check){
                return 1;
            }
        }
    }
    return 0;
}

// Worker thread function to check column validity
void* columnWorker(void* param) {
    for(int i = 0; i < 9; ++i) {
        for(int j = 1; j < 10; ++j) {
            if(numberIsInCol(j, i) != 1) {
                solution = 0; //no solution
                printf("Column didn't find %d\n", j);
                pthread_exit(0);
            } 
        }
    }
    pthread_exit(0);
}

// Worker thread function to check row validity
void* rowWorker(void* param) {
   for(int i = 0; i < 9; ++i) {
        for(int j = 1; j < 10; ++j) {
            if(numberIsInRow(j, i) != 1) {
                solution = 0; //no solution
                printf("Row didn't find %d\n", j);
                pthread_exit(0);
            } 
        }
    }
    pthread_exit(0);
}

// Worker thread function to check subgrid validity
void* subgridWorker(void *param){
    int* data = (int*)param;
    int startRow = data[0];
    int startCol = data[1];
    for(int j = 1; j <= 9; ++j){
        if(!numberIsInSubgrid(j, startRow, startCol)){
            solution = 0;
            printf("Subgrid starting from row %d, column %d doesn't contain %d\n",startRow + 1, startCol +1, j);
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}

// Function to validate Sudoku solution using threads
void validateSudokuWithThreads(int option) {
    pthread_t tid[9];
    int data[2];
    
    // Create threads based on the option selected
    switch(option) {
        case 1: // One thread for each column
            for(int i = 0; i < 9; ++i) {
                pthread_create(&tid[i], NULL, columnWorker, NULL);
            }
            break;
        case 2: // One thread for each row
            for(int i = 0; i < 9; ++i) {
                pthread_create(&tid[i], NULL, rowWorker, NULL);
            }
            break;
        case 3: // One thread for each subgrid
            
            break;
        default:
            printf("Invalid option\n");
            return;
    }
    
    // Join threads
    for(int i = 0; i < 9; ++i) {
        pthread_join(tid[i], NULL);
    }
}

void rowProcessor() {
    for(int i = 0; i < 9; ++i) {
        for(int j = 1; j < 10; ++j) {
            if(numberIsInRow(j, i) != 1) {
                solution = 0; //no solution
                printf("Row didn't find %d\n", j);
            } 
        }
    }
}

void columnProcessor() {
    for(int i = 0; i < 9; ++i) {
        for(int j = 1; j < 10; ++j) {
            if(numberIsInCol(j, i) != 1) {
                solution = 0; //no solution
                printf("Column didn't find %d\n", j);
            } 
        }
    }
}

void subgridProcessor(void *param){
    int* data = (int *)param;
    int startRow = data[0];
    int startCol = data[1];
    for(int j = 1; j <= 9; ++j){
        if(!numberIsInSubgrid(j, startRow, startCol)){
            solution = 0;
            printf("Subgrid starting from row %d, column %d doesn't contain %d\n",startRow + 1, startCol +1, j);
        }
    }
}

void parentProcess(int pipefd, int write_end) {
    wait(NULL);
    close(write_end);
    read(pipefd, &solution, sizeof(int));
    close(pipefd);
}

void childProcess(int pipefd) {
    write(pipefd, &solution, sizeof(int));
    close(pipefd);
    exit(EXIT_SUCCESS);
}

void validateSudokuWithProcesses() {
    /*open a pipe*/
    int pipefd[2];
    pipe(pipefd);

    /*fork process*/
    pid_t childPid = fork();
    if(childPid < 0) {
        printf("could not create a child process\n");
    } else if(childPid == 0) {
        /*child process*/
        close(pipefd[0]);
        rowProcessor();
        childProcess(pipefd[1]);
    } else {
        /*parent process*/
        parentProcess(pipefd[0], pipefd[1]);
    }

    childPid = fork();
    if(childPid < 0) {
        printf("could not create a child process\n");
    } else if(childPid == 0) {
        /*child process*/
        close(pipefd[0]);
        columnProcessor();
        childProcess(pipefd[1]);
    } else {
        /*parent process*/
        wait(NULL);
        parentProcess(pipefd[0], pipefd[1]);
    }
}


int main(int argc, char** argv) {
    FILE* filePtr;
    int num;
    char ch;

    filePtr = fopen("board.txt", "r");

    if(filePtr == NULL) {
        printf("could not open file\n");
        return 1;
    }

    //read file into 2d board array
    for(int i = 0; i < 9; ++i) {
        for(int j = 0; j < 9; ++j) {
            if(fscanf(filePtr, "%d", &board[i][j]) != 1) {
                printf("error reading file\n");
                return 1;
            }
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }

   int option;
 
    printf("Options:\n");        
    printf("1. One thread for each column\n");
    printf("2. One thread for each row\n");
    printf("3. One thread for each subgrid\n");
    scanf("%d",&option);  

   
    if(option < 1 || option > 3) {
        printf("Invalid option\n");
        return 1;
    }

    // Perform Sudoku validation
    clock_t start, end;
    double time_taken;

    start = clock();

      if(option == 1 || option == 2) {
        validateSudokuWithThreads(option);
    } else if(option == 3) {
        validateSudokuWithProcesses();
    }
    
    end = clock();

    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("SOLUTION: %s (%f seconds)\n", solution ? "YES" : "NO", time_taken);
    
    fclose(filePtr);
    return 0;
}
