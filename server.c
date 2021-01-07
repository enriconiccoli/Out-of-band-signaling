#include<sys/select.h>
#include<sys/un.h> 
#include<stdio.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include<stdlib.h>
#include<sys/unistd.h>
#include<inttypes.h>
#include<arpa/inet.h>
#include<errno.h>
#include<signal.h>
#include<pthread.h>


#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#define ERRmin(val1,val2,string) if(val1 < val2){ perror(string); exit(EXIT_FAILURE);}
#define ERRdiff(val1,val2,string) if(val1 != val2){ perror(string); exit(EXIT_FAILURE);}
#define ERReq(val1,val2,string) if(val1 == val2){ perror(string); exit(EXIT_FAILURE);}



/*
	Tramite questa struct vengono passati i parametri (Numero identificativo del Server, file descriptor di Socket e file descriptor di Pipe) al thread-server
*/
struct funargs{
	int num;
	int s_fd;
	int p_fd;
};



/*
	File descriptor della pipe viene definito globalmente per permettere la sua chiusura direttamente nell'handler di SIGINT
*/
int pipefd;

static void gestore(){

	int errVal=close(pipefd);
	ERReq(errVal, -1, "Close of pipe failed\n");

	_exit(EXIT_SUCCESS);

}



void * IOTask(void *arg){

double ms1,s1,s2;
int timeMin = 0;  
int first=1;
int errVal;

struct timespec t;

char buf[20];
char pbuf[23];
char timestamp[4];
int nread=1;
int64_t id, idNO;
struct funargs *args = (struct funargs*) arg;

int sockfd = (*args).s_fd;
int serNum = (*args).num;
int pipefd = (*args).p_fd;

id=0;

while(nread != 0){

	nread=read(sockfd,buf,20);
	ERReq(nread,-1,"Read of Server failed\n");

	clock_gettime(CLOCK_REALTIME, &t);

	if (nread==0) {							

		errVal=close(sockfd);
		ERReq(errVal, -1, "Close of Socket failed\n"); 
		
	}
	else { 	
 					
		idNO = strtoll(buf, NULL, 10);
		id = ntohll(idNO);
		

		/*
			In questa fase viene effettuato il calcolo del tempo intercorso fra la ricezione dei messaggi, viene fatta una distinzione a seconda che si tratti 
			del primo messaggio ricevuto o dei successivi. Viene quindi salvato il minor valore stimato, pari a 0 in caso non siano stati ricevuti messaggi consecutivi.
		*/
		if(first){
			ms1=(double)t.tv_nsec/1000000000;
			s1=t.tv_sec;
			s1 = s1 + ms1;
			first=0;
		}
		else{
			s2 = s1;

			s1=t.tv_sec;
			ms1=(double)t.tv_nsec/1000000000;
			s1 = s1 + ms1;

			if(timeMin > ((s1-s2)*1000) || timeMin == 0){
				timeMin = (s1-s2)*1000;
			}
		}
		printf("SERVER %d INCOMING FROM: %"PRIx64" @%f s\n",serNum,id, s1);
		fflush(stdout);

	}
}


/*
	L'invio della stima avviene tramite un messaggio nella forma "ClientID Secret", ed avviene se e solo se sono stati ricevuti almeno 2 messaggi dal client. Il valore 
	della variabile "timeMin" infatti viene inizializzato a 0 ed aggiornato, la prima volta, alla ricezione del secondo messaggio ricevuto
*/
if(timeMin !=0 ){

	printf("SERVER %d CLOSING %"PRIx64" ESTIMATE %d\n",serNum,id, timeMin);
	fflush(stdout);

	errVal = sprintf(timestamp, "%d", timeMin);
	ERRmin(errVal, 0, "Sprintf failed\n");

	errVal = sprintf(pbuf, "%lx", id);
	ERRmin(errVal, 0, "Sprintf failed\n");

	strcat(pbuf, " ");					
	strcat(pbuf, timestamp);
	strcat(pbuf, "\0");

	errVal=write(pipefd, pbuf, 23);
	ERReq(errVal, -1, "Write on pipe failed\n");
}	


}


/*
	Questa funzione costituisce il cuore del server: un selettore cicla sul passive socket e, ad ogni nuova connessione accettata, crea un thread che la gestisca
*/
static void run_server(struct sockaddr_un * psa,int num, int pipefd) {

int fd_sk;
int fd_c;	
int fd_num=0;	
int fd;		



fd_set set;
fd_set rdset; 		
int nread;		
int errVal;


pthread_t thread;


fd_sk=socket(AF_UNIX,SOCK_STREAM,0);
ERReq(fd_sk,-1,"Socket failed\n");			



errVal = bind(fd_sk,(struct sockaddr *)psa,sizeof(*psa));	
ERReq(errVal,-1,"Bind failed\n");

errVal=listen(fd_sk,SOMAXCONN);				
ERReq(errVal,-1,"Listen failed\n");



if (fd_sk > fd_num) fd_num = fd_sk;		

FD_ZERO(&set);		
FD_SET(fd_sk,&set);	


 
struct timeval timeout = {2,0};


while (1) {
	rdset=set;

	errVal = select(fd_num+1,&rdset,NULL,NULL,&timeout);
	if(errno != EINTR){
		ERReq(errVal,-1,"Select in SER failed\n");
	}


	for (fd = 0; fd<=fd_num;fd++) {	
		
		if (FD_ISSET(fd,&rdset)) {		    

			if (fd == fd_sk) {		

				fd_c=accept(fd_sk,NULL,0);
				ERReq(fd_c,-1,"Accept failed\n");			


				printf("SERVER %d CONNECT FROM CLIENT\n", num);
				fflush(stdout);
	

				struct funargs *args = malloc(sizeof(struct funargs));
				ERReq(args, NULL, "Malloc failed\n");

				(*args).num = num;
				(*args).s_fd = fd_c;
				(*args).p_fd = pipefd;


				errVal = pthread_create(&thread, NULL, IOTask, (void*)args);
				ERRdiff(errVal, 0, "Thread creation failed\n");

				errVal=pthread_detach(thread);
				ERRdiff(errVal, 0, "Thread detach failed\n");
			}

		} 

	}
	 
}



}



/*
	Nella funzione main viene installato l'handler per la gestione di SIGINT, dopodichè si procede a chiamare la funzione run_server definita e descritta sopra 
*/
int main (int argc, char* argv[]){

int pipefd,i,num,errVal;
struct sigaction s;
memset (&s,0,sizeof(s));

s.sa_flags = SA_RESTART;
s.sa_handler=gestore;
sigaction(SIGINT,&s,NULL);

 
char sock[16] = "./OOB-server-";	
strcat(sock, argv[1]);

if(access(sock,F_OK) != -1){
	errVal = unlink(sock);
	ERReq(errVal, -1, "Unlink failed\n");
}

num = strtol(argv[1], NULL, 10);
pipefd = strtol(argv[2], NULL, 10);

printf("SERVER %d ACTIVE\n", num);
fflush(stdout);

struct sockaddr_un sa; 
strcpy(sa.sun_path, sock);
sa.sun_family=AF_UNIX;


run_server(&sa,num, pipefd);




return 0;
}
