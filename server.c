#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "helpers.h"
void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}
//Functie care imi aloca o celula de tip subscriber si imi populeaza campurile
subscribers* aloc_subscriber(char* id,int x){
	subscribers* aux = (subscribers*) malloc(sizeof(subscribers));
	if(!aux) return NULL;
	aux->id = (char*) malloc(MAX_ID_LENGTH*sizeof(char));
	sprintf(aux->id,"%s",id);
	aux->sock=x;
	aux->mL=NULL;
	aux->list=NULL;
	aux->next=NULL;
	return aux;
}
//Functie care imi aloca o celula de tip abonare si imi populeaza campurile
abonare* aloc_abonare(char *topic, int sf){
	abonare* aux = (abonare*) malloc(sizeof(abonare));
	if(!aux) return NULL;
	sprintf(aux->topic,"%s",topic);
	aux->sf=sf;
	aux->next=NULL;
	return aux;
}
//Functie care imi aloca o celula de tip topicused si imi populeaza campurile
topicused* aloc_topicused(char *topic){
	topicused* aux = (topicused*) malloc(sizeof(topicused));
	if(!aux) return NULL;
	sprintf(aux->name,"%s",topic);
	aux->next=NULL;
	return aux;
}
//Functie care imi aloca o celula de tipa message_list
//Copiaza, in info, parametrul m
message_list* aloc_mL(message* m){
	message_list* aux=(message_list*) malloc(sizeof(message_list));
	if(!aux) return NULL;
	aux->info=(message*) malloc(sizeof(message));
	memcpy((void*)aux->info,(void*)m,sizeof(message));
	aux->next=NULL;
	
	return aux;
}
//Functie care imi insereaza un mesaj intr-o lista de mesaje
int insert_message(message_list **list, message *m){
	message_list *u=NULL, *p=*list,*aux;
	if(p){
		while(p->next) 
			p=p->next;
		u=p;
	}
	aux=aloc_mL(m);
	if(!aux) return 0;
	if(u==NULL)
		*list=aux;
	else{
		u->next=aux;
		u->next->next=NULL;
	}
	return 1;
}
//Functie care imi insereaza un tipic intr-o lista de topicused
int insert_topic(topicused **list, char *topic){
	topicused *u=NULL, *p=*list,*aux;
	if(p){
		while(p->next) 
			p=p->next;
		u=p;
	}
	aux=aloc_topicused(topic);
	if(!aux) return 0;
	if(u==NULL)
		*list=aux;
	else{
		u->next=aux;
		u->next->next=NULL;
	}
	return 1;
}
//Functie care imi cauta un topic intr-o lista de topicused
int cautare_topic(topicused *list,char *topic){
	while(list){
		if(strcmp(list->name,topic)==0) return 1;
		list=list->next;
	}
	return 0;
}
//Functie care imi elimina un topic dintr-o lista de topicused
int eliminare_topic(topicused **list, char *topic){
	topicused *ant,*p;
	for(p=*list,ant=NULL;p!=NULL;ant=p,p=p->next)
		if(strcmp(p->name,topic)==0) break;
	if(p==NULL) return 0;
	if(ant==NULL) *list=p->next;
	else ant->next = p->next;
	free(p);
	return 1;
}
//Functie care imi insereaza un subscriber intr-o lista
int insert_subs(subscribers **list, char *id,int x){
	subscribers *u=NULL,*p=*list,*aux;
	if(p){
		while(p->next) 
			p=p->next;
		u=p;
	}
	aux=aloc_subscriber(id,x);
	if(!aux) return 0;
	if(u==NULL)
		*list=aux;
	else{
		u->next=aux;
		u->next->next=NULL;
	}
	return 1;
}
//Functie care imi insereaza o abonare intr-o lista de abonari
int insert_abonare(abonare **list, char *topic,int x){
	abonare *u=NULL,*p=*list,*aux;
	if(p){
		while(p->next)
			p=p->next;
		u=p;
	}
	aux=aloc_abonare(topic,x);
	if(!aux) return 0;
	if(u==NULL)
		*list=aux;
	else{
		u->next=aux;
		u->next->next=NULL;
	}
	return 1;
}
//Functie care imi elinima o abonare dintr-o lista de abonari
int eliminare_abonare(abonare **list, char *topic){
	abonare *ant,*p;
	for(p=*list,ant=NULL;p!=NULL;ant=p,p=p->next)
		if(strcmp(p->topic,topic)==0) break;
	if(p==NULL) return 0;
	if(ant==NULL) *list=p->next;
	else ant->next = p->next;
	free(p);
	return 1;
}
//Functie care verifica daca unul din subscriberi mai e abonat la un topic
int cauta_topic_in_subs(subscribers *list,char *topic){
	while(list){
		abonare *aux=list->list;
		while(aux){
			if(strcmp(aux->topic,topic)==0) return 1;
			aux=aux->next;
		}
		list=list->next;
	}
	return 0;
}
//Functie care modifica socketul unui subscriber in functie de id
int modificare_sock(subscribers *list,char *id,int x){
	subscribers *p=list;
	//Caut in lista de subscriberi unul care are id-ul egal cu cel primit ca parametru
	while(p){
		if(strcmp(p->id,id)==0){
			//Daca sock-ul este mai mare ca 0 adica este subs este conectat se intoarce -1
			if(p->sock>0) return -1;
			//Se seteaza socketul nou daca acesta este -1
			p->sock=x;
			if(x>0){
				//Daca socketul este mai mare ca 0 se trimit toate mesajele din lista
				while(p->mL){
					//Se trimite mesajul la subscriber
					send(p->sock,p->mL->info,BUFLEN,0);
					//Se elimina mesajul din lista de mesaje
					message_list *aux=p->mL;
					p->mL=aux->next;
					free(aux->info);
					free(aux);
				}
			}
			//Daca s-a gasit un subscriber care avea socketul -1 se intoarce 1.
			return 1;
		}
		p=p->next;
	}
	//Daca nu s-a gasit un subscriber cu id-ul potrivit se intoarce 0
	return 0;
}
//Functie care primeste o lista de subscriberi si un socket
//Intoarce un pointer la elementul din lista care are socketul=x
subscribers *cautare_sock(subscribers *list,int x){
	subscribers *p=list;
	while(p){
		if(p->sock==x)
			return p;
		p=p->next;
	}
	//Se intoarce null daca nu s-a gasit nici un element cu socketul x
	return NULL;
}
//Functie care primeste un topic si o lista de abonari si modifca sf-ul
int modificare_sf(abonare *list,char *topic,int sf){
	//Itereaza prin lista si cauta topicul dorit
	while(list){
		if(strcmp(list->topic,topic)==0){
			list->sf=sf;
			//Daca se gaseste si sf-ul se modifica se intoarce 1
			return 1;
		}
		list=list->next;
	}
	//Daca nu se gaseste se intoarce 0
	return 0;
}

//Functie care primeste un unsigned int si intoarce o adresa ip cu .
//Am gasit functia aceasta online si am folosit-o
//Nu eram sigur daca exista o functie in biblioteca care face asta
char* print_ip(unsigned int ip)
{
    unsigned char bytes[4];
    char *addr=(char*)malloc(20*sizeof(char*));
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;   
    sprintf(addr,"%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]); 
    return addr;       
}
/*Fuctie care primeste un socket de udp si un pointer la un mesaj
  Functia intoarce 0 daca nu s-a putut initializa mesajul
  Si 1 daca mesajul a fost ok
  Functia mai intoarce mesajul m ca efect lateral
*/
int primire_mesaj_udp(int sock_udp,message **m){
	struct sockaddr_in udp_addr;
	int size=sizeof(udp_addr);
	int ret;
	char buffer[BUFLEN];
	//Sir de caractere in care imi parsez datele despre clientul udp(port si ip)
	char infoudp[50];
	//Primesc un mesaj de la client UDP cu recvfrom
	ret=recvfrom(sock_udp, buffer, BUFLEN, 0, (struct sockaddr*) &udp_addr, (socklen_t*) &size);
		if(ret>0){
			//apelez functia print_ip pentru a scrie adresa ip in format human-readable
			char *ip=print_ip(ntohl(udp_addr.sin_addr.s_addr));
			//parsez infoudp cu ip si port
			sprintf(infoudp,"%s:%d",ip,ntohs(udp_addr.sin_port));
			free(ip);
			//copiez in m mesajul primit de la UDP
			memcpy((void*)*m,(void*) buffer,BUFLEN);
			if(!(*m)) return 0;
			//Verific tipul de date si parsez mesajul
			//Daca tipul este int scriu mesajul in msg in functie de semn si numar
			if((*m)->data_type==0){
				int semn=*(unsigned char*)((*m)->msg);
				int nr=ntohl(*(unsigned int *)((*m)->msg+1));
				if(semn==0)
					sprintf((*m)->msg,"%d",nr);
				else sprintf((*m)->msg,"-%d",nr);
			}
			//Daca tipul este short_real impart numarul la 100 si il scriu in mesaj
			if((*m)->data_type==1){
				float nr=ntohs(*(uint16_t*)((*m)->msg));
				nr=nr/100;
				sprintf((*m)->msg,"%0.2f",nr);
			}
			//Daca tipul este float determin semnul, baza si exponentul
			//Impart baza la 10 in functie de exponent
			//Pun numarul in mesaj
			if((*m)->data_type==2){
				int semn=*(unsigned char*)((*m)->msg);
				float base=ntohl(*(unsigned int*)((*m)->msg+1));
				int exp=*(unsigned char *)((*m)->msg+5);
				while(exp>0) {
					exp--;
					base/=10;
				}
				if(semn==0)
					sprintf((*m)->msg,"%f",base);
				else sprintf((*m)->msg,"-%f",base);
			}
			//Setez ultimul element din msg ca terminator de sir
			(*m)->msg[1500]='\0';
			//Copiez in addrinfo informatiile despre clinetul udp
			memcpy((void*)(*m)->addrinfo,(void*)infoudp,50);
			memset(buffer, 0, BUFLEN);
			return 1;
		}
		return 0;
}
//Functie care primeste o lista de subscriberi si un mesaj
//Cauta abonari la acelasi topic ca mesajul si trimit mesajul
void send_message(subscribers *list,message *m){
	//Itereaza prin lista de subscriberi
	while (list){
		//Itereaza prin lista de abonari a fiecarui subscriber
		abonare *aux=list->list;
		while(aux){
			//Daca gaseste in lista de abonari topicul mesajului il trimite
			if(strcmp(aux->topic,m->topic)==0){
				//Daca socketul subscriberului este -1 adica este deconectat
				//Si are setat SF mesajul este inserat in lista de mesaje a subs-ului
				if(list->sock==-1 && aux->sf==1){
					insert_message(&list->mL,m);
				}
				//Daca socketul este ok se trimit mesajul
				else send(list->sock,m,BUFLEN,0);
				break;
			}
			aux=aux->next;
		}
		list=list->next;
	}
}
//Functie de eliberare spatiu a unei liste da abonari
void eliberare_abonare(abonare** list){
	abonare* aux;
	while(*list){
		aux=*list;
		*list=(*list)->next;
		free(aux);
	}
}
//Functie de elibereare spatiu a unei liste de message_list
void eliberare_mL(message_list** list){
	message_list* aux;
	while(*list){
		aux=*list;
		*list=(*list)->next;
		free(aux->info);
		free(aux);
	}
}
//Functie de eliberare spatiu a unui topicused
void eliberare_topicused(topicused** list){
	topicused* aux;
	while(*list){
		aux=*list;
		*list=(*list)->next;
		free(aux);
	}
}
//Functie de eliberare spatiu a unei liste de subscriber
void eliberare_sub(subscribers** list){
	subscribers* aux;
	while(*list){
		aux=*list;
		*list=(*list)->next;
		free(aux->id);
		eliberare_mL(&aux->mL);
		eliberare_abonare(&aux->list);
		free(aux);
	}
}
//In main deschid socketurile si realizez multiplexarea de socketuri
int main(int argc, char *argv[])
{
	message *m=(message*) malloc(sizeof(message));
	topicused *list_topics=NULL;
	subscribers *list_subs=NULL,*aux;
	int sockfd, newsockfd, portno,sock_udp;
	char buffer[BUFLEN],delimitator[]=" \n";
	struct sockaddr_in serv_addr, cli_addr, addr;
	int n, i, ret;
	socklen_t clilen;
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds
	//int ack;
	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	//Deschid un socket de udp si dau bind pentru a fi vizibil de clientii udp
	sock_udp = socket(AF_INET,SOCK_DGRAM,0);
	DIE(sock_udp < 0, "socket-UDP");
	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
	addr.sin_port=htons(portno);

	ret = bind(sock_udp, (struct sockaddr *) &addr, sizeof(struct sockaddr));
	DIE(ret<0,"bind-udp");
	//il adaug in lista de file descriptori
	FD_SET(sock_udp, &read_fds);
	fdmax=sock_udp;

	//Deschid un socket TCP si dau bind pentru a fi vizibil
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket-TCP");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga noul file descriptor in multimea read_fds
	FD_SET(sockfd, &read_fds);
	
	if(sockfd>fdmax){
		fdmax = sockfd;
	}
	//Adaug in multime si 0 ce reprezinta STDIN
	FD_SET(0, &read_fds);
	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		//Daca primesc date pe STDIN citesc de la tastatura
		if(FD_ISSET(0,&tmp_fds)){
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);
			//Daca primesc comanda exit se incheie executia programului
			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}

		}
		//Fac multiplexare intre socketi
		for (i = 1; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				//Daca socketul este cel udp apelez functia de primire mesaj
				if (i == sock_udp){
					if(primire_mesaj_udp(i,&m)==0)break;
					//caut in lista de topicused topicul mesajului
					//daca nu gasesc ies din for
					if(cautare_topic(list_topics,m->topic)==0)break;
					//daca gasesc trimit mesajul la lista de subscriberi
					send_message(list_subs,m);
				}else
				//Daca socketul este tcp se face o noua conexiune
				if (i == sockfd) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");
					int enabled = 1;
					//Se dezactiveaza neagle-ul
					setsockopt(newsockfd,IPPROTO_TCP, TCP_NODELAY, (char*) &enabled,sizeof(int));
					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					//se primeste de la client ID-ul si se afiseaza mesajul de conectare
					ret=recv(newsockfd,buffer,sizeof(buffer),0);
					buffer[ret]='\0';
					printf("New client %s connected from %s:%d.\n",
							buffer,inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
					//Se verifica id-ul
					ret=modificare_sock(list_subs,buffer,newsockfd);
					//Daca se intoarce 0 inseamna ca clientul nu exista in lista
					//Se insereaza in lista de subscriberi
					if(ret==0){
						insert_subs(&list_subs,buffer,newsockfd);
					}
					//Daca clientul exista si are socket activ se trimite un mesaj
					//Mesajul are lungime 1, iar clientul se inchide
					if(ret==-1){
						FD_CLR(newsockfd,&read_fds);
						char empty[10]="";
						send(newsockfd,empty,1,0);
					}else
					//Daca id-ul este ok se verifica fdmax-ul
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}
				} else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					DIE(n < 0, "recv");

					if (n == 0) {
						// conexiunea s-a inchis
						// Caut subscriberul care a inchis conexiunea si ii fac socketul -1
						aux=cautare_sock(list_subs,i);
						if(aux!=NULL){
							aux->sock=-1;
						}
						//Se afiseaza un mesaj ca s-a inchis conexiunea
						printf("Client %s disconnected.\n", aux->id);
						close(i);
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
					} else {
						//Se verifica comanda primita de la subscriber
						//La inceput se salveaza ca necunoscuta
						char copy[100]="Unknown command",topic[50];
						char *ptr = strtok(buffer,delimitator);
						//Daca primesc subscribe se verifica sa aiba cel putin 2 parametri
						if(strcmp(ptr,"subscribe")==0){
							ptr=strtok(NULL,delimitator);
							if(ptr) {
								sprintf(topic,"%s",ptr);
								ptr=strtok(NULL,delimitator);
								if(ptr){
									//Daca are 2 parametri se modifica mesajul
									//Mesajul va fi de forma "subscribed <topic>"
									sprintf(copy,"subscribed %s\n",topic);
									//se cauta subs care are socketul curent
										aux=cautare_sock(list_subs,i);
									//se verifica daca are deja o abonare la topic
									//Se modifica SF-ul
									if(modificare_sf(aux->list,topic,atoi(ptr))==0)
										//Daca topicul nu exista in lista de abonari
										//Se introduce in lista
										insert_abonare(&aux->list,topic,atoi(ptr));
									//Se introduce topicul in lista de topicused daca nu exista
									if(cautare_topic(list_topics,topic)==0)
										insert_topic(&list_topics,topic);	
								}
							}
						}else
						//Daca primesc unsubscribe verific sa aiba cel putin 1 parametru
						if(strcmp(ptr,"unsubscribe")==0){

							ptr=strtok(NULL,delimitator);
							if(ptr){ 
								//Daca am un parametru 
								//se modifica mesajul in "unsubscribed <topic>"
								sprintf(copy,"unsubscribed %s\n",ptr);
								//Se cauta subs care are socketul curent
								aux=cautare_sock(list_subs,i);
								//Se elimina topicul din lista de abonari
								eliminare_abonare(&aux->list,ptr);
								//Se verifica daca mai are cineva topic-ul in lista
								if(cauta_topic_in_subs(list_subs,ptr)==0)
									//Se elimina din lista de topicused
									//Daca nu e abonat nimeni la el
									eliminare_topic(&list_topics,ptr);
							}
						}
						//se trimite raspuns la comanda primita
						send(i,copy,sizeof(buffer),0);
					}
				}
			}
		}
	}
	//Se elibereaza memoria
	free(m);
	eliberare_topicused(&list_topics);
	free(list_topics);
	eliberare_sub(&list_subs);
	free(list_subs);
	//se inchid socketurile
	close(sock_udp);
	close(sockfd);

	return 0;
}
