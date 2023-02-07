a. Full name: weiqiang qian

b. Student ID: 5555112745

c. What you have done in the assignment?
	Finish all the phases and the requirement in this project except extra credit.

d. What your code files are and what each one of them does.
	client.cpp: It will read username and password from keyboard, and send it to main server, then recieve the authentication from main server; Then it will read course code and category from keyboard and send them to ask main server to get the course information. All the procedure through TCP.
	serverM.cpp: It will recieve the credential informaiton from client, then encrypt them and sent them to severC through UDP; then recieve the authentication result from serverC through UDP and pass it to the client though TCP. Also it will recieve the query request from client and pass it to the right department(serverEE or serverCS) through UDP, then recieve the information from department server through UDP and send them to the client though TCP.
	serverC.cpp: It will read the input file. and compare it with the received authentication request through UDP. if it match, then authentication pass, if not, it will return the corresponding result to the main server through UDP.
	serverEE.cpp: It will read the input file, and find if the message he received from serverM though UDP exist in the file, it will return the information if match, otherwise return didn't find the course.
	serverCS.cpp: It will read the input file, and find if the message he received from serverM though UDP exist in the file, it will return the information if match, otherwise return didn't find the course.

e. The format of all the messages exchanged.
	 All the messages are char array when it was recieved. if it needs to be proceed, I transfer it to string.

f. Any idiosyncrasy of your project.
	No idiosyncrasy. The only thing need to pay attention is the code can read no more than 1000 username, passwords, and the other informations.

g. Reused Code: (include reference)
	1.Most part of the TCP, UDP initialization procedure was referenced by Beej’s code. There are also annotated in the code.
	2.【socket】C语言的Socket网络编程_bandaoyu的博客-CSDN博客_c语言socket编程. (n.d.). Blog.csdn.net. Retrieved November 29, 2022, from https://blog.csdn.net/bandaoyu/article/details/83312254
‌

