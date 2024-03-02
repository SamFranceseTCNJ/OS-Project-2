#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
int board[9][9];
int solution = 1;

int numberIsInCol(int check, int col) {
    for(int i = 0; i < 9; ++i) {
        if(board[i][col] == check) {
            return 1;
        }
    }
    return -1;
}

void* columnWorker(void* param) {
    for(int i = 0; i < 9; ++i) {
        for(int j = 1; j < 10; ++j) {
            if(numberIsInCol(j, i) != 1) {
                solution = 0; //no solution
                printf("didn't find %d\n", j);
                pthread_exit(0);
            } 
        }
    }
    pthread_exit(0);
}

int main(int argc, char** argv) {
    FILE* filePtr;
    pthread_t tid;
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

    pthread_create(&tid, NULL, columnWorker, NULL);
    pthread_join(tid, NULL);

    printf("SOLUTION: %d\n", solution);
    
    fclose(filePtr);
    return 0;
}