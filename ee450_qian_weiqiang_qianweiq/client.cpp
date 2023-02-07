#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<iostream>

#include<netdb.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
using namespace std;
/*
** client.c -- a stream socket client demo
*/
#define TCPPORT "25745" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once
// get sockaddr, IPv4 or IPv6:


char query_response[50];


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[]){
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    string username;
    string password;
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);

    memset(&hints, 0 , sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((rv = getaddrinfo(argv[1], TCPPORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and connect to the first we can 
    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol))== -1){
        perror("client: socket");
        continue;
    }
        if(connect(sockfd, p->ai_addr,p->ai_addrlen) == -1){
            close(sockfd);
            continue;
        }
        break;
    }   
    if(p == NULL){
        fprintf(stderr,"client: failed to conect\n");
        return 2;
    }
    
    if(getsockname(sockfd,(struct sockaddr *)&sin,&len)==-1){
        perror("getsockname");
    }
    int client_port = ntohs(sin.sin_port);

     cout << "The client is up and running." << endl; 
    for(int i = 2; i >= 0; i -- ){
        username.clear();
        username.clear();
        cout << "Please enter the username: " ;
        cin >>  username;
        int l_1 = username.size();
        int name_sent = send(sockfd, username.c_str(), l_1, 0);

        cout << "Please enter the password: " ;        
        cin >> password;
        int l_2 = password.size();
        int pass_sent = send(sockfd, password.c_str(), l_2, 0);
        

        //recv the authentication response fron serverM
        char response[50];
        if(recv(sockfd, response,50,0) == -1){
            perror("recieve");
            exit(1);
        }
        // cout << "you received: " << response[0] << endl;

        if(response[0] == '2'){
            cout << username << " recieved the result of the authentication using TCP over port " << client_port << "." <<endl;
            cout << "Authentication is successful" << endl;
            break;
        }
        else if(i != 0 && response[0] == '0'){
            cout << username << " recieved the result of authentication using TCP over port " << client_port <<"." <<endl;
            cout << "Authentication failed: Username Does not exist."<<endl;
            cout << "Attemps remaining: "<< i << endl;
            continue;
        }
        else if(i != 0 && response[0] == '1'){
            cout << username << " recieved the result of authentication using TCP over port " << client_port <<"." <<endl;
            cout << "Authentication failed: Password does not match." << endl;
            cout << "Attemps remaining: " << i << endl;
            continue;
        }
        else if(i == 0 && response[0] != '2'){
            cout << "Authentication Failed for 3 attemps. CLient will shit down" << endl;
            exit(1);
        }


    }
    //phase3
    while(1){
        memset(query_response, '\0', sizeof query_response);
        cout << "Please enter the couse code to query: " << endl;
        string course;
        cin >> course;
        int len_course = course.size();
        int course_sent = send(sockfd, course.c_str(), len_course, 0);
        cout << "Please enter the category(Credit/Professor/Days/CourseName)" << endl;
        string category;
        cin >> category;
        int len_category = category.size();
        int category_sent = send(sockfd, category.c_str(), len_category, 0);

        cout << username << " sent a request to the main server." << endl;

        //recv the query response fron serverM
        // char query_response[50];
        if(recv(sockfd, query_response,50,0) == -1){
            perror("recieve");
            exit(1);
            }
            cout<< "The client recieved the response from the Main server using TCP over port "<< client_port<<" ."<< endl;
            // cout<< "The response is : "<< query_response<<endl;
            string q_res = query_response;
            // cout <<"You recieved: "<< q_res << endl;
            if(q_res.compare("0")){
                cout<<"The "<< category << " of "<< course << " is "<< q_res << endl;            
            }
            else{
                cout << "Didn't find the course: "<< course << endl;
            }

            cout << " "<<endl;
            cout << " "<<endl;
            cout << "-----Start a new request-----"<<endl;
    }



    freeaddrinfo(servinfo); //all done with this structure
    close(sockfd);

    return 0;
}