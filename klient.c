#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>

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
    int is_empty;
} Wpis;

key_t key;
int shmid;
Wpis *wpisy;
int MAX_MESSAGES;

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    char *username = argv[2];

    if ((key = ftok(argv[1], 'A')) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    if ((shmid = shmget(key, 0, 0666)) == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    if ((wpisy = (Wpis *)shmat(shmid, (void *)0, 0)) == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    MAX_MESSAGES = wpisy[0].MAX_MESSAGES; // ustalanie ilości wpisów na serwerze

    printf(YELLOW "Aktualne wpisy:" RESET "\n");
    printf(YELLOW "--------------------" RESET "\n");

    for (int i = 0; i < MAX_MESSAGES; ++i) {
        
        if (wpisy[i].is_empty == 1 ) {
            printf(BLUE "ID: %d\n" RESET, i);
            printf(BLUE "Username: %s" RESET "\n", wpisy[i].username);
            if(wpisy[i].message[0] != '\0')
            printf(BLUE "Message: %s" RESET "\n", wpisy[i].message);
            else printf(BLUE "Message: Typing... " RESET "\n");
            printf(BLUE "Likes: %d" RESET "\n", wpisy[i].likes);
            printf(YELLOW "--------------------" RESET "\n");
        }
    }

    printf(BLUE "Chcesz dać (L)ike czy utworzyć (N)owy wpis?" RESET "\n");
    char c;
    scanf(" %c", &c);
    
    if (c == 'L' || c == 'l') {

        printf(BLUE "Podaj id wpisu, który chcesz polubić" RESET "\n");
        int id;
        scanf("%d", &id);

        if(wpisy[id].is_empty == 0){

            printf(RED "Nie ma takiego wpisu" RESET "\n");
            exit(EXIT_FAILURE);

        }else wpisy[id].likes++;

    } else 
    if (c == 'N' || c == 'n') {
        for (int i = 0; i < MAX_MESSAGES; ++i) {
            if (wpisy[i].is_empty == 0) {

                wpisy[i].is_empty = 1;//chyba dzięki temu nie trzeba semaforów
                
                snprintf(wpisy[i].username, MAX_USERNAME_LEN, "%s", username);

                printf(BLUE "Co ci chodzi po głowie?" RESET "\n");
                
                getchar(); 
                fgets(wpisy[i].message, sizeof(wpisy[i].message), stdin);
                size_t len = strlen(wpisy[i].message);
                
                if (len > 0 && wpisy[i].message[len - 1] == '\n') {
                    wpisy[i].message[len - 1] = '\0';
                }
                
                wpisy[i].likes = 0;
                
                break;

            }else if(i == MAX_MESSAGES - 1){
                printf(RED "Nie ma miejsca na nowy wpis" RESET "\n");
                exit(EXIT_FAILURE);}

        }
    } else printf(RED "Niepoprawny wybór" RESET "\n");
    

    if (shmdt(wpisy) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);}

    printf(BLUE "Dziękujemy za skorzystanie z aplikacji" RESET "\n");
    
    return 0;
}
