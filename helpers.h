#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>
//constanta ce reprezinta lungimea maxima a unui ID
#define MAX_ID_LENGTH 10
/*Structura ce reprezinta un mesaj primit de la udp si transmis la tcp
  topic-ul e numele topicului
  data_type e tipul de date
  msg e mesajul propriu zis primit
  addrinfo este un mesaj de forma ip:port completate dupa primirea mesajului de la udp
*/
typedef struct message{
	char topic[50];
	uint8_t data_type;
	char msg[1501];
	char addrinfo[50];
}message;
/* 	Lista de abonari. Fiecare subscriber are o lista cu toate abonarile
	topic reprezinta numele topicului la care se aboneaza
	sf reprezina tipul de store&foreward(0 sau 1)
	next este un pointer catre urmatorul element din lista
*/
typedef struct abonare{
	char topic[50];
	int sf;
	struct abonare *next;
}abonare;
/*	Lista de mesaje pastrate pentru a fi trimise unui subscriber dupa ce se reconecteaza
	info reprezinta mesajul de tip message
	next este un pointer catre urmatorul element din lista
*/
typedef struct message_list{
	message *info;
	struct message_list *next;
}message_list;
/*	Lista de subscriberi
	id este un string unde se pastreaza id-ul subscriberului. Dupa id este identificat
	sock este file descriptorul curent
	mL este o lista de mesaje a fiecarui subscriber.
	list este lista de abonari la anumite topicuri.
	next este un pointer catre urmatorul element din lista
*/
typedef struct subscribers{
	char* id;
	int sock;
	message_list* mL;
	abonare* list;
	struct subscribers *next;
}subscribers;
/* 	Lista de topic-uri folosite de cel putin un subscriber
	name reprezina numele topicului
	next este un pointer catre urmatorul element din lista
*/
typedef struct topicused{
	char name[50];
	struct topicused *next;
}topicused;
//MACRO-ul din schelet
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)
//Lungimea maxima primita la recv si trimisa la send
//Este egala cu marimea unei structuri de message
#define BUFLEN		1600
//Numarul maxim de clienti de pe listen	
#define MAX_CLIENTS	1000	

#endif
