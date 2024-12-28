#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define MAX_USERNAME_LEN 20
#define MAX_MESSAGE_LEN 100

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

typedef struct wpis {
    char username[MAX_USERNAME_LEN];
    char message[MAX_MESSAGE_LEN];
    int likes;
    int MAX_MESSAGES;
    int is_empty;//0 - pusty, 1 - nie pusty
} Wpis;

key_t key;
int shmid;
Wpis *wpisy;
int MAX_MESSAGES;

void wypisz(int signal) {

    sleep(1);

    printf("\n");
    printf(YELLOW "--------------------" RESET "\n");

    for (int i = 0; i < MAX_MESSAGES-1; ++i) {
        
        if (wpisy[i].is_empty != 0) {
            printf(BLUE "ID: %d" RESET "\n", i);
            printf(BLUE "Username: %s"RESET"\n", wpisy[i].username);
            if(wpisy[i].message[0] != '\0')
            printf(BLUE "Message: %s" RESET "\n", wpisy[i].message);
            else printf(BLUE "Message: Typing... " RESET "\n");
            printf(BLUE "Likes: %d" RESET "\n", wpisy[i].likes);
            printf(YELLOW "--------------------" RESET "\n");
        }
    }
}

void zakoncz(int signal) {

    if (shmdt(wpisy) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);}

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);}

    printf("\n" RED "Server closed." RESET "\n");
    exit(EXIT_SUCCESS);
}

// funkcja dodająca dwa przykładowe wpisy
void saveExampleEntries() {
    
    snprintf(wpisy[0].username, MAX_USERNAME_LEN, "JP2");
    snprintf(wpisy[0].message, MAX_MESSAGE_LEN, "2137");
    wpisy[0].likes = 5;
    wpisy[0].is_empty = 1;
    
    snprintf(wpisy[1].username, MAX_USERNAME_LEN, "Kolega");
    snprintf(wpisy[1].message, MAX_MESSAGE_LEN, "Pytam o link");
    wpisy[1].likes = 3;
    wpisy[1].is_empty = 1; 

    printf("Example entries saved to shared memory.\n");
}


int main(int argc, char *argv[]) {

    signal(SIGTSTP, wypisz);
    signal(SIGINT, zakoncz);

    printf(BLUE "[Server]" RESET "\n");

    //int pid = getpid();
    //printf("PID: %d\n", pid);

    char *arg=argv[2];
     while (*arg) {
        if (!isdigit(*arg)) {
            printf("Wrong argument. It should be a numeric value.\n");
            return EXIT_FAILURE;
        }
        arg++;
    }

    MAX_MESSAGES = atoi(argv[2]);
    
    if(MAX_MESSAGES<1){
        printf("Wrong argument");
        exit(EXIT_FAILURE);
    }

    if (argc != 3) {
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);}

    if ((key = ftok(argv[1], 'A')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);}

    printf("key: %d\n", key);
    
    if ((shmid = shmget(key, MAX_MESSAGES * sizeof(Wpis), 0666 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);}

    printf("shmid: %d\n", shmid);

    if ((wpisy = (Wpis *)shmat(shmid, NULL, 0)) == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);}
    

    for(int i=0;i<MAX_MESSAGES;i++){//inicjalizacja wpisow
        wpisy[i].MAX_MESSAGES = MAX_MESSAGES;
        wpisy[i].is_empty = 0;
        wpisy[i].message[0] = '\0';
        wpisy[i].likes = 0;
    }
     
    //saveExampleEntries();//przykladowe wpisy

    while (1) 
        sleep(1);
    

    return 0;
}
