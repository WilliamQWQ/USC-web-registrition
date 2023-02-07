#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
#include<iostream>

using namespace std;
#define TCPPORT "25745"  // the port users will be connecting to 
#define UDPPORT "24745"
#define SERVERC "21745"
#define SERVERCS "22745"
#define SERVEREE "23745"
#define BACKLOG 10 // how many pending connections queue will hold


char recvcourse_client[50] = "\0";
char recvcategory_client[50] = "\0";
char res_EE[50];

void sigchld_handler(int s){
// waitpid() might overwritte errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;

}
//get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string encryption(string str){
    for(int i = 0; i < str.size(); i++){
        if(isdigit(str[i])){
            str[i] += 4;
            if(str[i] > '9'){
                str[i] = (char)(str[i] - 10);
            }
        }
        else if(isalpha(str[i])){
            if(str[i] >= 'a' && str[i] <= 'z'){
                str[i] += 4;
                int tmp = (int) (str[i] - 'a');
                tmp %= 26;
                str[i] = (char) ('a' + tmp);
            }
            else if(str[i] >= 'A' && str[i] <= 'Z'){
                str[i] += 4;
                int tmp = (int) (str[i] - 'A');
                tmp %= 26;
                str[i] = (char)('A' + tmp);
            }
        }
        else continue;
    }
    return str;
}


int main(void)
{
    int sockfd, new_fd;  //listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr, their_Udp_addr; // connector's address information
    socklen_t sin_size;
    socklen_t addr_len;
    struct sigaction sa;
    int yes = 1;
    int rv;
    int numbytes;
    int numbytes_C;
    char recvname_client[50] = "\0";
    char recvpass_client[50] = "\0";
    // TCP initialization  from bj code
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // use my IP;

    if((rv = getaddrinfo(NULL, TCPPORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p= p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if(bind(sockfd, p-> ai_addr, p-> ai_addrlen) == -1){
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo); // all done with this structure;

    if(p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }  

    cout <<"The main server is up and running."<< endl;

    sa.sa_handler = sigchld_handler; //reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }
    //End tcp initialization

    // UCP socket initialization from bj code
    
    int sockfd_udp;// serverM UDP;
    struct addrinfo hints_UDP, *servinfo_UDP,*p_UDP;
    int rv_udp;

    memset(&hints_UDP, 0 , sizeof hints_UDP);
    hints_UDP.ai_family = AF_UNSPEC;
    hints_UDP.ai_socktype = SOCK_DGRAM;
    
    if((rv_udp = getaddrinfo("127.0.0.1", UDPPORT, &hints_UDP, &servinfo_UDP)) != 0){
        fprintf(stderr, "getaddinfo: %s\n", gai_strerror(rv_udp));
        return 1;
    }

    //loop through all the results and bind to the first we can 

    for(p_UDP = servinfo_UDP; p_UDP != NULL; p_UDP = p_UDP->ai_next){
        if((sockfd_udp = socket(p_UDP->ai_family, p_UDP->ai_socktype, p_UDP->ai_protocol)) == -1){
            perror("talker: socket");
            continue;
        }
        break;
    }

    if(p_UDP == NULL){
        fprintf(stderr, "talker: failed to creat socket\n");
        return 2;
    }
    freeaddrinfo(servinfo_UDP);
    //UDP initialization end;

    //initialoize serverC socket from bj's code
    int sockfd_C; // socket for serverC not used in serverM
    struct addrinfo hints_C,*servinfo_C,*p_C;
    int rv_C;

    memset(&hints_C,0,sizeof hints_C);
    hints_C.ai_family = AF_UNSPEC;
    hints_C.ai_socktype = SOCK_DGRAM;

       if((rv_C = getaddrinfo("127.0.0.1", SERVERC, &hints_C, &servinfo_C)) != 0){
        fprintf(stderr, "getaddinfo: %s\n", gai_strerror(rv_udp));
        return 1;
    }

    //loop through all the results and bind to the first we can 

    for(p_C = servinfo_C; p_C != NULL; p_C = p_C->ai_next){
        if((sockfd_C = socket(p_C->ai_family, p_C->ai_socktype, p_C->ai_protocol)) == -1){
            perror("talker: socket");
            continue;
        }
        break;
    }

    p_C = servinfo_C;
    if(p_C == NULL){
        fprintf(stderr, "talker: failed to creat socket\n");
        return 2;
    }
    //end initialization.
       //initialoize serverCS socket from bj's code
    int sockfd_CS; // socket for serverC not used in serverM
    struct addrinfo hints_CS,*servinfo_CS,*p_CS;
    int rv_CS;

    memset(&hints_CS,0,sizeof hints_CS);
    hints_CS.ai_family = AF_UNSPEC;
    hints_CS.ai_socktype = SOCK_DGRAM;

       if((rv_CS = getaddrinfo("127.0.0.1", SERVERCS, &hints_CS, &servinfo_CS)) != 0){
        fprintf(stderr, "getaddinfo: %s\n", gai_strerror(rv_udp));
        return 1;
    }

    //loop through all the results and bind to the first we can 

    for(p_CS = servinfo_CS; p_CS != NULL; p_CS = p_CS->ai_next){
        if((sockfd_CS = socket(p_CS->ai_family, p_CS->ai_socktype, p_CS->ai_protocol)) == -1){
            perror("talker: socket");
            continue;
        }
        break;
    }

    p_CS = servinfo_CS;
    if(p_CS == NULL){
        fprintf(stderr, "talker: failed to creat socket\n");
        return 2;
    }
    //end initialization.
 
    printf("server: waiting for connections...\n");

   //initialoize serverEE socket from bj's code
    int sockfd_EE; // socket for serverC not used in serverM
    struct addrinfo hints_EE,*servinfo_EE,*p_EE;
    int rv_EE;

    memset(&hints_EE,0,sizeof hints_EE);
    hints_EE.ai_family = AF_UNSPEC;
    hints_EE.ai_socktype = SOCK_DGRAM;

       if((rv_EE = getaddrinfo("127.0.0.1", SERVEREE, &hints_EE, &servinfo_EE)) != 0){
        fprintf(stderr, "getaddinfo: %s\n", gai_strerror(rv_udp));
        return 1;
    }

    //loop through all the results and bind to the first we can 

    for(p_EE = servinfo_EE; p_EE != NULL; p_EE = p_EE->ai_next){
        if((sockfd_EE = socket(p_EE->ai_family, p_EE->ai_socktype, p_EE->ai_protocol)) == -1){
            perror("talker: socket");
            continue;
        }
        break;
    }

    p_EE = servinfo_EE;
    if(p_EE == NULL){
        fprintf(stderr, "talker: failed to creat socket\n");
        return 2;
    }
    //end initialization.

    // cout << "The main server is up and running." << endl; 
    while(1){  // main accept() loop
        //TCP recieving using.
        if(listen(sockfd, BACKLOG) == -1){
            perror("Listen");
            exit(1);
        }
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if(new_fd == -1){
            perror("accept");
            continue;
        }
        for(int i = 2; i >= 0 ; i--){
            memset(recvname_client, '\0', sizeof recvname_client);
            if(recv(new_fd, recvname_client, 50, 0) <=0 ){break;}
                
            
            memset(recvpass_client, '\0', sizeof recvpass_client);
            if(recv(new_fd, recvpass_client, 50, 0) == -1){break;}
            cout << "The main server recieved the authentication for "<< recvname_client << " using TCP over port " << TCPPORT << endl;
            // cout << "The password is " << recvpass_client <<endl;
            string hn = recvname_client;
            string pw = recvpass_client; 

            string en_hn = encryption(hn);

            addr_len = sizeof their_addr;
            if((numbytes = sendto(sockfd_udp,en_hn.c_str(), 50, 0, p_C->ai_addr, p_C->ai_addrlen)) == -1){
                perror("talker: sendto");
                exit(1);
            } 

            string en_pw = encryption(pw);
            if((numbytes = sendto(sockfd_udp,en_pw.c_str(), 50, 0, p_C->ai_addr, p_C->ai_addrlen)) == -1){
                perror("talker: sendto");
                exit(1);
            } 

            cout << "The main serve sent an authentication request to serverC." << endl;
            
            //recvfrom serverC within UDP socket , got the authentication result.
            char res_C[50];
            if((numbytes_C = recvfrom(sockfd_udp, res_C, 50, 0, (struct sockaddr *)&their_Udp_addr, &addr_len)) == -1){
                perror("recvfrom");
                exit(1);
            }
            string res = res_C;
            cout << "The main server recieved the result of the authentication request from ServerC using UDP over port "<< UDPPORT << endl;
            // cout << "the inputs is: " << res[0] << endl;
            string feed_back;
            feed_back.push_back(res[0]);
            if(send(new_fd, feed_back.c_str(),50,0) == -1){
                perror("send");
            }
            cout << "The main server sent the authentication result to the client." << endl;
            if(res[0] == '2'){
                break;
            }
        }
        //phase3 recv query from client
    while(1){    
        // char recvcourse_client[50] = "\0";
        // char recvcategory_client[50] = "\0";
        memset(recvcourse_client, '\0', sizeof recvcourse_client);
        if(recv(new_fd, recvcourse_client, 50, 0) <= 0){break;}
        memset(recvcategory_client, '\0', sizeof recvcategory_client);
        if(recv(new_fd, recvcategory_client, 50, 0) <= 0){
            break;
        }
        cout << "The main server recieved from " << recvname_client << " to query course " <<  recvcourse_client << " about "<< recvcategory_client<<" using TCP over port " << TCPPORT << endl;
        string course_client   = recvcourse_client;
        // cout << "the course is: " << course_client << endl; 
        string prefix = course_client.substr(0,2);
        // addr_len = sizeof their_addr;
        if(!prefix.compare("EE")){
            if((numbytes = sendto(sockfd_udp, course_client.c_str(), 50, 0, p_EE->ai_addr,p_EE->ai_addrlen)) == -1){
                perror("talker: sendto");
                exit(1);
            }
            string category_client = recvcategory_client;
            if((numbytes = sendto(sockfd_udp, category_client.c_str(), 50, 0, p_EE->ai_addr,p_EE->ai_addrlen)) == -1){
                perror("talker: sendto");
                exit(1);
            }
            cout << "The main server sent a request to server" << prefix << endl;

            char res_EE[50];
            if((numbytes_C = recvfrom(sockfd_udp, res_EE, 50, 0, (struct sockaddr *)&their_Udp_addr, &addr_len)) == -1){
                perror("recvfrom");
                exit(1);
            }
            string information_EE = res_EE;
            cout << "The main server recieved the respose from server"<<prefix<<" using UDP over port "<< SERVEREE<<". "<<endl;
            if(send(new_fd, information_EE.c_str(),50,0) == -1){
                perror("send");
            }
            // cout<<"THe information is: "<< information_EE << endl;
            cout<< "The main server sent the query information to the client."<<endl;

        }
        else if(!prefix.compare("CS")){
            if((numbytes = sendto(sockfd_udp, course_client.c_str(), 50, 0, p_CS->ai_addr,p_CS->ai_addrlen)) == -1){
                perror("talker: sendto");
                exit(1);
            }
            string category_client = recvcategory_client;
            if((numbytes = sendto(sockfd_udp, category_client.c_str(), 50, 0, p_CS->ai_addr,p_CS->ai_addrlen)) == -1){
                perror("talker: sendto");
                exit(1);
            }
            cout << "The main server sent a request to server" << prefix << endl;

            char res_CS[50];
            if((numbytes_C = recvfrom(sockfd_udp, res_CS, 50, 0, (struct sockaddr *)&their_Udp_addr, &addr_len)) == -1){
                perror("recvfrom");
                exit(1);
            }
            string information = res_CS;
            cout << "The main server recieved the respose from server"<<prefix<<" using UDP over port "<< SERVERCS<<". "<<endl;
            if(send(new_fd, information.c_str(),50,0) == -1){
                perror("send");
            }
            // cout<<"THe information is: "<< information << endl;
            cout<< "The main server sent the query information to the client."<<endl;
        }
        else{
            string no = "0";
            if(send(new_fd, no.c_str(),50,0) == -1){
                perror("send");
            }
            // cout<<"THe information is: "<< no << endl;
            cout<< "The main server sent the query information to the client."<<endl;
        }
    }

        close(new_fd); // parent doesn't need this
    }
    return 0;
}

