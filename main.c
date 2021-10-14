#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>

#include "inc/doc.h"
#include "inc/doc_json.h"
#include "inc/strfmt.h"
#include "inc/curses_extra.h"
#include "inc/mytui.h"

int main(int argc, char **argv){

    initscr();
    // cbreak();
    timeout(-1);
    noecho();
    clear();
    curs_set(0);

    // check colors
    if(!has_colors()){
        printw("This terminal doesn't support colors!\nPlease use another terminal");
        getch();
        exit(-1);
    }
    if(!can_change_color()){
        printw("This terminal doesn't support custom colors!\nPlease use another terminal");
        getch();
        exit(-1);
    }

    start_color();

    frames_init();
    mytui_init();

    // first render
    mytui_draw();
    update_panels();
    refresh();

    // main loop
    while(1){
        
        // ----------------------- draw ------------------------
        mytui_draw();
        update_panels();
        refresh();

        // ----------------------- input -----------------------
        int input = getch();

        mytui_logic(input);

        switch(input){
            case 'q':
                endwin();
                exit(0);
                break;

            default:
                break;
        }

    }

    endwin();

    // int sd = socket(AF_INET, SOCK_STREAM, 0);

    // struct sockaddr_in server_addr;
    // server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(80);

    // inet_pton(AF_INET, "185.199.108.153", &server_addr.sin_addr);

    // // local host
    // // server_addr.sin_addr.s_addr = INADDR_ANY;

    // if(connect(sd, (struct sockaddr*)(&server_addr), sizeof(server_addr))){
    //     printf("Error connecting to server\n");
    //     exit(-1);
    // }

    // char buffer[5000];
    // snprintf(buffer, 5000, "GET / HTTP/1.1\r\n\r\n");

    // if(send(sd, buffer, strlen(buffer), 0) < 0){
    //     printf("Error sending GET\n");
    // }

    // int size = recv(sd, buffer, 5000ULL, 0);

    // printf("[%i]\n%s", size, buffer);

    // shutdown(sd, SHUT_RDWR);

    return 0;
}