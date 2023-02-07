#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#include<iostream>
#include<sstream>
#include<fstream>
using namespace std;

#define MYPORT "22745"  //serverCS's port number

#define MAXBUFLEN 100


string course_code[1000]  ;
string course_credit[1000];
string course_prof[1000]  ;
string course_days[1000]  ;
string course_name[1000]  ;
string valid_category[4] = {"Credit", "Professor", "Days", "CourseName"};
string reply = "0";
//get sockaddr, IPv4 or IPv6   from bj's code

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void read_cs(void){
    ifstream input;
    string str;
    int i = 0;
    input.open("cs.txt"); //open file
    if(input.is_open()){
    while(getline(input,str)){
        stringstream substring(str);
        string substr;
        while(getline(substring, substr,',')){
            if(i % 5 == 0){
                course_code[i / 5] = substr;
                i++;
            }
            else if(i % 5 == 1){
                course_credit[i / 5] = substr; 
                i++;
            }
            else if(i % 5 == 2){
                course_prof[i / 5] = substr;
                i++;
            }
            else if(i % 5 == 3){
                course_days[i / 5] = substr;
                i++;
            }
            else if (i % 5 == 4){
                course_name[i / 5] = substr;
                i++;
            }
        }       
    }
    }
    // cout <<"finish reading file" <<endl;
}

int main(void){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    if((rv = getaddrinfo("127.0.0.1", MYPORT, &hints, &servinfo)) != 0 ){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop thought all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("listener: socket");
            continue;
        }
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }

    if(p == NULL){
        fprintf(stderr, "listener: faied to bind socket\n");
        return 2;
    }
    freeaddrinfo(servinfo);
    printf("The serverCS is up and running using UDP on port 22745\n");

    read_cs();
    //keep server running
    while(1){
        addr_len = sizeof their_addr;
        char recv_code[50] = "\0";
        char recv_category[50] = "\0";

        if((numbytes = recvfrom(sockfd, recv_code, 50, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1){
            perror("recvfrom");
            exit(1);
        }
        string code = recv_code;
        if((numbytes = recvfrom(sockfd, recv_category, 50, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1){
            perror("recvfrom");
            exit(1);
        }
        string category = recv_category;

        cout << "The ServerCS recieved a request from the Main Server about the "<< category <<" of " << code <<endl; 
        reply = "0";
        //do something  send recv         
        for(int i = 0; i < 1000; i++){
            if(!code.compare(course_code[i])){
                for(int j = 0; j < 4; j++){
                    if(!category.compare(valid_category[j])){
                        if(!category.compare("Credit")){
                            reply = course_credit[i];
                            break;
                        }
                        else if(!category.compare("Professor")){
                            reply = course_prof[i];
                            break;
                        } 
                        else if(!category.compare("Days")){
                            reply = course_days[i];
                            break;
                        }
                        else if(!category.compare("CourseName")){
                            reply = course_name[i];
                            break;
                        }
                    }
                }
            }
        }

        // cout << "the result is : " << reply << endl;
        if(!reply.compare("0")){
            cout << "Didn't find the course: "<< code << endl;
        }
        else{
            cout << "The course information has been found: The "<< category << " of "<< code <<" is "<< reply << endl;
        }

        if((numbytes = sendto(sockfd, reply.c_str(), 50, 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
            perror("sendto");
            exit(1);
        }        
        // cout<< "what you send is :" << reply.c_str() << endl;
        cout<< "The ServerCS finished sending the response to the Main Server "<< endl;
        


    }
    close(sockfd);
    
    return 0;



}