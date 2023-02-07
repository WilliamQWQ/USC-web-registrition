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
#include<map>
#include<fstream>
#include<vector>
#include<sstream>
using namespace std;

#define MYPORT "21745"  //serverC's port number

#define MAXBUFLEN 100

string hn[1000];
string pw[1000];
string check("0");
//get sockaddr, IPv4 or IPv6   from bj's code

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void read_cred(void){
    ifstream input;
    string str;
    int i = 0;
    input.open("cred.txt"); //open file
    if(input.is_open()){
        while(getline(input,str)){
            stringstream substring(str);
            string substr;
            while(getline(substring, substr,',')){
                if(i % 2 == 0){
                    hn[i/2] = substr;
                    i++;
                }
                else{ 
                    pw[i/2] = substr;
                    i++;
                }
            }       
        }
    }
    // cout <<"finish reading file" <<endl;
}
string moveEnter(string s){
    string str;
    for(int i = 0; i < s.size(); i++){
        if(s[i] != 13){
            str += s[i];
        }
    }
    return str;
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
    printf("The serverC is up and running using UDP on port 21745\n");

    //keep server running
    read_cred();
    while(1){
        addr_len = sizeof their_addr;
        char en_hn[50] = "\0";
        char en_pw[50] = "\0";

        if((numbytes = recvfrom(sockfd, en_hn, 50, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1){
            perror("recvfrom");
            exit(1);
        }
        string authen_hn = en_hn;
        if((numbytes = recvfrom(sockfd, en_pw, 50, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1){
            perror("recvfrom");
            exit(1);
        }
        string authen_pw = en_pw;
        check = "0";
        cout << "The ServerC recieved an authentication request from the Main Server." << endl;
        //check username
        // cout<< "original check is :" << check<<endl;
        for(int i = 0; i < 1000; i++){
            if(!authen_hn.compare(hn[i])){
                string s1 = moveEnter(pw[i]);
                string s2 = moveEnter(authen_pw);
                if(!s1.compare(s2)){
                    check.replace(0,1,"2");
                    break;
                }else{
                    check.replace(0,1,"1");
                    break;
                }
            }
        }
        // cout << "checking result is : "<< check << endl;
        if((numbytes = sendto(sockfd, check.c_str(), 1, 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
            perror("sendto");
            exit(1);
        }        
        // cout<< "what you send is :" << check.c_str() << endl;
       cout<< "The ServerC finished sending the response to the Mian Server "<< endl;
    }
    close(sockfd);

    return 0;



}

