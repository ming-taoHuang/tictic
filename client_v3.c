#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
    #ifdef DEBUG
    perror(msg);
    #else
    printf("Either the server shut down or the other player disconnected.\nGame over.\n");
    #endif 

    exit(0);
}

/*
 * Socket Read Functions
 */

/* Reads a message from the server socket. */
void recv_msg(int sockfd, char * msg)
{
    /* All messages are 3 bytes. */
    memset(msg, 0, 4);
    int n = read(sockfd, msg, 3);
    
    if (n < 0 || n != 3) /* Not what we were expecting. Server got killed or the other client disconnected. */ 
        error("ERROR reading message from server socket.");

    #ifdef DEBUG
    printf("[DEBUG] Received message: %s\n", msg);
    #endif 
}

void recv_msg_list(int sockfd, char * msg)
{
    //memset(msg, 0, 4);
    int n = read(sockfd, msg, strlen(msg));
    
    if (n < 0 || n > 10) /* Not what we were expecting. Server got killed or the other client disconnected. */ 
        error("ERROR reading message from server socket.");

    #ifdef DEBUG
    printf("[DEBUG] Received message: %s\n", msg);
    #endif 
}

/* Reads an int from the server socket. */
int recv_int(int sockfd)
{
    int msg = 0;
    int n = read(sockfd, &msg, sizeof(int));
    
    if (n < 0 || n != sizeof(int)) 
        error("ERROR reading int from server socket");
    
    #ifdef DEBUG
    printf("[DEBUG] Received int: %d\n", msg);
    #endif 
    
    return msg;
}

/*
 * Socket Write Functions
 */

/* Writes an int to the server socket. */
int write_server_int(int sockfd, int msg)
{
    int n = write(sockfd, &msg, sizeof(int));
    if (n < 0)
        error("ERROR writing int to server socket");
    
    #ifdef DEBUG
    printf("[DEBUG] Wrote int to server: %d\n", msg);
    #endif 
}

/*
 * Connect Functions
 */

void write_server_msg(int sockfd, char* msg)
{
    int n = write(sockfd, msg, sizeof(msg));
    if (n < 0)
        error("ERROR writing message to server socket");
    
    #ifdef DEBUG
    printf("[DEBUG] Wrote message to server: %d\n", msg);
    #endif 
}


/* Sets up the connection to the server. */
int connect_to_server(char * hostname, int portno)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
 
    /* Get a socket. */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
    if (sockfd < 0) 
        error("ERROR opening socket for server.");
	
    /* Get the address of the server. */
    server = gethostbyname(hostname);
	
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	
	/* Zero out memory for server info. */
	memset(&serv_addr, 0, sizeof(serv_addr));

	/* Set up the server info. */
    serv_addr.sin_family = AF_INET;
    memmove(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno); 

	/* Make the connection. */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting to server");

    #ifdef DEBUG
    printf("[DEBUG] Connected to server.\n");
    #endif 
    
    return sockfd;
}

/*
 * Game Functions
 */

/* Draws the game board to stdout. */
void draw_board(char board[][3])
{
    printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
}

/* Get's the players turn and sends it to the server. */
void take_turn(int sockfd)
{
    char buffer[10];
    
    while (1) { /* Ask until we receive. */ 
        printf("Enter 0-8 to make a move, or 9 for number of active players: ");
	    fgets(buffer, 10, stdin);
	    int move = buffer[0] - '0';
        if (move <= 9 && move >= 0){
            printf("\n");
            /* Send players move to the server. */
            write_server_int(sockfd, move);   
            break;
        } 
        else
            printf("\nInvalid input. Try again.\n");
    }
}

/* Gets a board update from the server. */
void get_update(int sockfd, char board[][3])
{
    /* Get the update. */
    int player_id = recv_int(sockfd);
    int move = recv_int(sockfd);

    /* Update the game board. */
    board[move/3][move%3] = player_id ? 'X' : 'O';    
}

/*
 * Main Program
 */

int main(int argc, char *argv[])
{
	int pass;
	char authen[4];
	char au_name[10];
	char name[10];
	char nname[5][10];
	char list1[5];
	char list[10];
//	int sockfd;
//	int id;
    /* Make sure host and port are specified. */
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    /* Connect to the server. */
    int sockfd = connect_to_server(argv[1], atoi(argv[2]));

    /* The client ID is the first thing we receive after connecting. */
    int id = recv_int(sockfd);
    
        
    /*Log In*/
    /*while(1){
        printf("Type the password:");
        fgets(pass,6,stdin);
        if(!strcmp(pass,"123456")){
               printf("Success Log In !!\n");
               break;
        }
        else    error("Wrong Password !!\n");
    }*/

    while(1){
	printf("Enter the Name:");
	//memset(name,0,10);
	//fgets(name,10,stdin);
	scanf("%s",name);
	printf("%s",name);
	write_server_msg(sockfd,name);
	//memset(au_name,0,10);
        recv_msg(sockfd,au_name);

	if (!strcmp(au_name, "CHK")) { 
	        printf("Check your name!\n");  
        }
	else{
		error("Error receiving message");
		exit(1);
	}
    	printf("Enter the password:");
	//fgets(pass,9,stdin);
	scanf("%d",&pass);
	write_server_int(sockfd,pass);
	printf("Password sent\n\n");
	
        recv_msg(sockfd, authen);

        if (!strcmp(authen, "ACC")) { /* check the password */
	        printf("Log In Successful!\n\n");
		printf("check the player!!\n\n");
	        break;  
        }
	else{
		error("Error receiving message");
		exit(1);
	}
    }
    #ifdef DEBUG
    printf("[DEBUG] Client ID: %d\n", id);
    #endif 

    char msg[4];
    char board[3][3] = { {' ', ' ', ' '}, /* Game board */
                         {' ', ' ', ' '}, 
                         {' ', ' ', ' '} };

    printf("Tic-Tac-Toe\n------------\n");

    /* Wait for the game to start. */
    do {
        recv_msg(sockfd, msg);
        if (!strcmp(msg, "HLD"))
            printf("Waiting for a second player...\n");
    } while ( strcmp(msg, "SRT") );

    /* The game has begun. */
    printf("Game on!\n");
    printf("Your are %c's\n", id ? 'X' : 'O');

    draw_board(board);

    while(1) {
        recv_msg(sockfd, msg);

        if (!strcmp(msg, "TRN")) { /* Take a turn. */
	        printf("Your move...\n");
	        take_turn(sockfd);
        }
        else if (!strcmp(msg, "INV")) { /* Move was invalid. Note that a "TRN" message will always follow an "INV" message, so we will end up at the above case in the next iteration. */
            printf("That position has already been played. Try again.\n"); 
        }
        else if (!strcmp(msg, "CNT")) { /* Server is sending the number of active players. Note that a "TRN" message will always follow a "CNT" message. */
            int num_players = recv_int(sockfd);
            printf("There are currently %d active players.\n", num_players); 
        }
        else if (!strcmp(msg, "UPD")) { /* Server is sending a game board update. */
            get_update(sockfd, board);
            draw_board(board);
        }
        else if (!strcmp(msg, "WAT")) { /* Wait for other player to take a turn. */
            printf("Waiting for other players move...\n");
        }
        else if (!strcmp(msg, "WIN")) { /* Winner. */
            printf("You win!\n");
            break;
        }
        else if (!strcmp(msg, "LSE")) { /* Loser. */
            printf("You lost.\n");
            break;
        }
        else if (!strcmp(msg, "DRW")) { /* Game is a draw. */
            printf("Draw.\n");
            break;
        }
        else /* Weird... */
            error("Unknown message.");
    }
    
    printf("Game over.\n");

    /* Close server socket and exit. */
    close(sockfd);
    return 0;
}
