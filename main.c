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

void* subgridWorker(void *param) {
   int* data = (int*)param;
   int startRow = data[0] * 3;
   int startCol = data[1] * 3;
   for(int j = 1; j <= 9; ++j){
       if(numberIsInSubgrid(j, startRow, startCol) != 1){
           solution = 0;
           printf("Subgrid starting from row %d, column %d doesn't contain %d\n", startRow + 1, startCol + 1, j);
           pthread_exit(0); 
       }
   }
   pthread_exit(0); 
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
            for(int i = 0; i < 3; ++i) {
                for(int j = 0; j < 3; ++j) {
                    pthread_create(&tid[i * 3 + j], NULL, subgridWorker, NULL);
                }
            }
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
    
// Function to perform statistical experiment
void statisticalExperiment(int option) {
    const int runs = 50;
    double total_time = 0;

    printf("Running statistical experiment with option %d...\n", option);

    for(int i = 0; i < runs; ++i) {
        clock_t start, end;
        double time_taken;
        
        start = clock();
        
        if(option >= 1 && option <= 3) {
            validateSudokuWithThreads(option);
        } else if(option == 4) {
            //validateSudokuWithProcesses();
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

   int option;
 
    printf("Options:\n");        
    printf("1. One thread for each column\n");
    printf("2. One thread for each row\n");
    printf("3. One thread for each subgrid\n");
    scanf("%d",&option);  

   
    if(option < 1 || option > 4) {
        printf("Invalid option\n");
        return 1;
    }

    // Perform Sudoku validation
    clock_t start, end;
    double time_taken;

    start = clock();

      if(option >= 1 && option <= 3) {
        validateSudokuWithThreads(option);
    } else if(option == 4) {
        //not implemented  validateSudokuWithProcesses();
        return 1;
    }
    
    end = clock();

    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("SOLUTION: %s (%f seconds)\n", solution ? "YES" : "NO", time_taken);
    
    // Perform statistical experiment
    statisticalExperiment(option);

    fclose(filePtr);
    return 0;
}
