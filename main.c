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
        //not implemented  validateSudokuWithProcesses();
        return 1;
    }
    
    end = clock();

    time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("SOLUTION: %s (%f seconds)\n", solution ? "YES" : "NO", time_taken);
    
    fclose(filePtr);
    return 0;
}
