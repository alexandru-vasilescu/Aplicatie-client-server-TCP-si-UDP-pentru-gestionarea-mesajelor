#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file); 
	exit(0);
}
//In main fac conexiunea cu serverul primesc mesajul de tip msg
int main(int argc, char *argv[])
{	
	//mesajul primit
	message *m=(message*) malloc(sizeof(message));
	//socketul subscriberului, valori folosite pentru debug
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;

	if (argc < 4) {
		usage(argv[0]);
	}

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	//Asociez un socket de tip TCP subscriberului
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	//Completez structura adresei serverului cu port si ip primite ca parametru
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");
	//fac conexiunea cu serverul
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");
	//trimit serverului id-ul primit ca parametru
	send(sockfd,argv[1],MAX_ID_LENGTH,0);
	//adaug in lista de file descriptori socketul
	FD_SET(sockfd,&read_fds);
	fdmax=sockfd;
	//adaug 0 ce reprezinta STDIN-ul pentru a citi de la tastatura
	FD_SET(0,&read_fds);
	//Variabila folosita in caz ca am trimis un mesaj de (un)subscribe
	//Daca e 1 parsez bufferul intr-un message
	//Daca e 0 afisez direct
	int i=1;
	while (1) {
  		tmp_fds = read_fds; 
		//Verific de unde primesc date, STDIN sau server
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		//Daca primesc de la STDIN citesc de la tastatura
		if(FD_ISSET(0,&tmp_fds)){

  			// se citeste de la tastatura
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);
			//Verific daca am primit exit caz in care se incheie programul
			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}
			// se trimite mesaj la server
			n = send(sockfd, buffer, strlen(buffer), 0);
			DIE(n < 0, "send");
			memset(buffer,0,BUFLEN);
			//Setez i=0 pentru a afisa direct raspunsul serverului
			i=0;
		}else
		//Daca primesc fdmax primesc date de la server
		if(FD_ISSET(fdmax,&tmp_fds)){
			memset(buffer,0,BUFLEN);
			//n reprezina numarul de octeti primiti de la server
			//In general n o sa fie un numar mare daca totul a functionat cum trebuie
			n=recv(fdmax,buffer,sizeof(buffer),0);
			DIE( n < 0 ,"recv");
			//Daca i=0 afisez direct mesajul
			if(i==0){
				//Daca n=0 inseamna ca serverul inchis conexiunea
				//Daca n=1 inseamna ca exista deja un client cu acest ID si inchid programul
				if(n==0 || n==1){
					close(fdmax);
					break;
				}
				else
					//Se afiseaza raspunsul serverului la comanda data
					//In server se verifica daca comanda citita este buna
					//Se verifica numarul parametrilor si numele comenzii
					printf("%s\n",buffer);
				i=1;
			}else{
				//in m se copiaza ce s-a primit prin recv
				memcpy((void*)m,(void*) buffer,BUFLEN);
				//Daca n=0 inseamna ca serverul inchis conexiunea
				//Daca n=1 inseamna ca exista deja un client cu acest ID si inchid programul
				if(n==0 || n==1){
					close(fdmax);
					break;
				}
				else{	
					//Se afiseaza informatiile despre clientul udp si topicul				
					printf("%s - %s - ",m->addrinfo,m->topic);
					//Se verifica ce tip de date am primit si se afiseaza tipul
					switch (m->data_type){
						case 0:
							printf("INT");
							break;
						case 1:
							printf("SHORT_REAL");
							break;
						case 2:
							printf("FLOAT");
							break;
						case 3:
							printf("STRING");
							break;
					}
					//Se afiseaza mesajul primit
					printf(" - %s\n",m->msg);
				}
			}	
		}
	}
	//Se elibereaza m
	free(m);
	//Se inchide socketul
	close(sockfd);
	return 0;
}
