#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int board[9][9];
int solution = 1;
struct ThreadArgs {
    int* data;
    int startRow;
    int startCol;
};

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
    struct ThreadArgs* args = (struct ThreadArgs*)param;
    for(int j = 1; j < 10; ++j) {
        if(numberIsInCol(j, args->startCol) != 1) {
            solution = 0; //no solution
            printf("Column didn't find %d\n", j);
            pthread_exit(0);
        } 
    }
    pthread_exit(0);
}

// Worker thread function to check row validity
void* rowWorker(void* param) {
    struct ThreadArgs* args = (struct ThreadArgs*)param;
    for(int j = 1; j < 10; ++j) {
        if(numberIsInRow(j, args->startRow) != 1) {
            solution = 0; //no solution
            printf("Row didn't find %d\n", j);
            pthread_exit(0);
        } 
    }
    pthread_exit(0);
}

void* subgridWorker(void *param) {
    struct ThreadArgs* args = (struct ThreadArgs*)param;
   int startRow = args->startRow;
   int startCol = args->startCol;
   for(int j = 1; j <= 9; ++j){
       if(numberIsInSubgrid(j, startRow, startCol) != 1){
           solution = 0;
           printf("Subgrid starting from row %d, column %d doesn't contain %d\n", startRow + 1, startCol + 1, j);
           pthread_exit(0); 
       }
   }
   pthread_exit(0); 
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

//function to check subgrids for PROCESSES ONE THREAD
void subgridProcessor(int startRow, int startCol){
   for(int j = 1; j <= 9; ++j){
       if(numberIsInSubgrid(j, startRow, startCol) != 1){
           solution = 0;
           printf("Subgrid starting from row %d, column %d doesn't contain %d\n", startRow + 1, startCol + 1, j);
       }
   }
}

// Function to validate Sudoku solution using threads
void validateSudokuWithThreads(int option) {
    int NUM_THREADS = 27;
    pthread_t tid[NUM_THREADS];
    struct ThreadArgs args[NUM_THREADS];
    
    // Create threads based on the option selected
    switch(option) {
        case 1: // One thread for everything 
            rowProcessor();
            columnProcessor();
            for(int i = 0; i < 9; i += 3) {
                for(int j = 0; j < 9; j += 3) {
                    subgridProcessor(i, j);
                }
            }
            break;
        case 2: // One thread for each subgrid, column, and row
            /*subgrid threads*/
            int thread = 0;
            int startCol = 0;
            int startRow = 0;
            for(int i = 0; i < 9; i += 3) {
                for(int j = 0; j < 9; j += 3) {
                    args[thread].startRow = i;
                    args[thread].startCol = j;
                    thread++;
                }
            }
            /*create subgrid threads*/
            for(int i = 0; i < 9; ++i) {
                pthread_create(&tid[i], NULL, subgridWorker, (void*)&args[i]);
            }
            /*column threads*/
            for(int i = 9; i < 18; ++i) {
                args[i].startCol = startCol;
                pthread_create(&tid[i], NULL, columnWorker, (void*)&args[i]);
                startCol++;
            }
            /*row threads*/
            for(int i = 18; i < 27; ++i) {
                args[i].startRow = startRow;
                pthread_create(&tid[i], NULL, rowWorker, (void*)&args[i]);
                startRow++;
            }
            break;
        default:
            printf("Invalid option\n");
            return;
    }
    
    // Join threads
    if(option != 1) {
        for(int i = 0; i < NUM_THREADS; ++i) {
            pthread_join(tid[i], NULL);
        }
    }
}


void forkProcess(int read_end, int write_end, pid_t childPid, int section) {
    if(childPid < 0) {
        printf("could not create a child process\n");
    } else if(childPid == 0) {
        if(section == 1) {
            rowProcessor();
        } else if(section == 2) {
            columnProcessor();
        } else {
            for(int i = 0; i < 9; i += 3) {
                for(int j = 0; j < 9; j += 3) {
                    subgridProcessor(i, j);
                }
            }
        }
        write(write_end, &solution, sizeof(int));
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
        read(read_end, &solution, sizeof(int));
    }
}

void validateSudokuWithProcesses() {
    /*open a pipe*/
    int pipefd[2];
    pipe(pipefd);

    /*fork process*/
    pid_t childPid = fork();
    forkProcess(pipefd[0], pipefd[1], childPid, 1); /*ROW*/
    childPid = fork();
    forkProcess(pipefd[0], pipefd[1], childPid, 2); /*COLUMN*/
    childPid = fork();
    forkProcess(pipefd[0], pipefd[1], childPid, 3); /*SUBGRID*/

    close(pipefd[0]);
    close(pipefd[1]);
}
    
// Function to perform statistical experiment
void statisticalExperiment(int option) {
    const int runs = 50;
    double total_time = 0;

    printf("Running statistical experiment with option %d...\n", option);

    for(int i = 0; i < runs; ++i) {
        clock_t start, end;
        double time_taken;
        
        start = clock();
        
        if(option >= 1 && option < 3) {
            validateSudokuWithThreads(option);
        } else if(option == 3) {
            validateSudokuWithProcesses();
        } else {
            printf("Invalid option\n");
            return;
        }

        end = clock();
        time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
        total_time += time_taken;
    }

    double average_time = total_time / runs;
    printf("Average time taken: %f seconds\n", average_time);
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

   int option = atoi(argv[1]);
 
    printf("Options:\n");        
    printf("1. One thread for everything\n");
    printf("2. One thread for every column\n");
    printf("3. Three processes\n");
    printf("You chose: %d\n", option);

   
    if(option < 1 || option > 3) {
        printf("Invalid option\n");
        return 1;
    }

    // Perform Sudoku validation
    clock_t start, end;
    double time_taken;

    start = clock();

    if(option >= 1 && option < 3) {
        validateSudokuWithThreads(option);
    } else if(option == 3) {
        validateSudokuWithProcesses();
    }
    
    end = clock();

    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("SOLUTION: %s (%f seconds)\n", solution ? "YES" : "NO", time_taken);
    
    // Perform statistical experiment
    statisticalExperiment(option);

    fclose(filePtr);
    return 0;
}
