#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void DearOldDad(int *ShmPTR);
void PoorStudent(int *ShmPTR);

int main(int argc, char *argv[]) {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Allocate shared memory for 2 integers:
    // ShmPTR[0] = BankAccount
    // ShmPTR[1] = Turn
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        perror("*** shmget error (server) ***");
        exit(1);
    }

    // Attach shared memory
    ShmPTR = (int *) shmat(ShmID, NULL, (void *) -1);
    if (ShmPTR == (int *) -1) {
        perror("*** shmat error (server) ***");
        exit(1);
    }

    // Initialize shared variables
    ShmPTR[0] = 0; // BankAccount = 0
    ShmPTR[1] = 0; // Turn = 0

    // Fork child process
    pid = fork();
    if (pid < 0) {
        perror("*** fork error ***");
        exit(1);
    } else if (pid == 0) {
        PoorStudent(ShmPTR);
        exit(0);
    }

    DearOldDad(ShmPTR);

    wait(&status);
    shmdt((void *) ShmPTR);
    shmctl(ShmID, IPC_RMID, NULL);
    exit(0);
}

void DearOldDad(int *ShmPTR) {
    srand(time(NULL) ^ getpid());

    for (int i = 0; i < 25; i++) {
        // 1. Sleep random amount between 0 - 5 seconds
        sleep(rand() % 6);

        // 3. Strict alternation: wait while Turn != 0
        while (ShmPTR[1] != 0);

        // 2. Copy BankAccount to local variable account
        int account = ShmPTR[0];

        // 4. If account <= 100, try to deposit
        if (account <= 100) {
            int balance = rand() % 101; // Random amount between 0 - 100

            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        // Copy local variable back to BankAccount and pass turn
        ShmPTR[0] = account;
        ShmPTR[1] = 1;
    }
}

void PoorStudent(int *ShmPTR) {
    srand(time(NULL) ^ getpid());

    for (int i = 0; i < 25; i++) {
        // 1. Sleep random amount between 0 - 5 seconds
        sleep(rand() % 6);

        // 3. Strict alternation: wait while Turn != 1
        while (ShmPTR[1] != 1);

        // 2. Copy BankAccount to local variable account
        int account = ShmPTR[0];

        // 4. Random balance needed between 0 - 50
        int balance = rand() % 51;
        printf("Poor Student needs $%d\n", balance);

        // 5 & 6. Withdraw if funds are sufficient
        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        // Copy local variable back to BankAccount and pass turn
        ShmPTR[0] = account;
        ShmPTR[1] = 0;
    }
}