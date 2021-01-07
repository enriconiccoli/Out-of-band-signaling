#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/select.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include "myHash.h"


#define ERRmin(val1,val2,string) if(val1 < val2){ perror(string); exit(EXIT_FAILURE);}
#define ERRdiff(val1,val2,string) if(val1 != val2){ perror(string); exit(EXIT_FAILURE);}
#define ERReq(val1,val2,string) if(val1 == val2){ perror(string); exit(EXIT_FAILURE);}


/*
	Hashmap definita globalmente in modo da poter essere acceduta e manipolata dall'handler del SIGINT
*/
struct table *t; 	



/*
	In caso di arrivo di un singolo SIGINT viene avviato un timer e stampata la hashmap su stderr, nel caso in cui arrivi un ulteriore SIGINT prima della
	scadenza del timer viene stampata la tabella su STDOUT, viene propagato il SIGINT ai server e, prima di uscire, viene liberata la heap
*/
static void gestore(){
	
	if(alarm(2)!=0){
		printTable(t,0);

		int errVal = kill(0,SIGINT);
		ERReq(errVal, -1, "Kill failed\n");

		freeTable(t);
		free(t);
		
		printf("SUPERVISOR EXITING\n");
		fflush(stdout);

		_exit(EXIT_SUCCESS);

	}
	else{
		printTable(t,1);
	}

}



/*
	Nel main, dopo aver effettuato la gestione dei segnali, altre operazioni iniziali quali dichiarazione e inizializzazione di variabili e controllo degli argv, si entra
	in un ciclo nel quale, tramite una fork e una exec, vengono lanciati i server con gli opportuni argomenti. Dopodichè si passa al ciclo principale: un while nel quale si
	effettua una select per poi ciclare sui vari file descriptor delle pipe, leggendo i messaggi inviati dai server e salvandone il contenuto in una hashmap
*/

int main(int argc, char* argv[]){

struct sigaction s;
memset (&s,0,sizeof(s));

s.sa_flags = SA_RESTART;
s.sa_handler=gestore;
sigaction(SIGINT,&s,NULL);


sigset_t mask;
sigemptyset(&mask);
sigaddset(&mask, SIGALRM);
sigprocmask(SIG_BLOCK, &mask, NULL);



ERRdiff(argc, 2, "Wrong number of arguments\n");



// "t" è una hashmap di dimensione 20 utilizzata per lo storage delle stime dei vari Secret
t = createTable(20);



int i,k,pid, errVal;
k = strtol(argv[1], NULL, 10);
int pipefd[k][2];
fd_set set;		
fd_set rdset;
int fd, nread=0;
int fd_num=3;
int64_t id;
int secret;
struct timeval timeout = {2,0};
char buf[23];


printf("SUPERVISOR STARTING %d\n", k);
fflush(stdout);


char *arg[4];
arg[0] = "server.out";
arg[3] = NULL;


for(i=0; i<k; i++){
	
	errVal = pipe(pipefd[i]);
	ERReq(errVal,-1,"Pipe failed\n");

	pid = fork();
	ERReq(pid,-1,"Pid failed\n");

	if(pid == 0){

		char n[10];
		errVal = sprintf(n, "%d", i);
		ERRmin(errVal,0,"Sprintf failed\n");
		arg[1] = n;

		char writepW[10];
		errVal = sprintf(writepW, "%d", pipefd[i][1]); 
		ERRmin(errVal,0,"Sprintf failed\n");
		arg[2] = writepW;
		

		errVal=close(pipefd[i][0]);
		ERReq(errVal,-1,"Close failed\n");

		errVal=execv("./server.out",arg);
		ERReq(errVal,-1,"Execv failed\n");

	}
	else{
		errVal=close(pipefd[i][1]);
		ERReq(errVal,-1,"Close failed\n");
	}
	

}

	

FD_ZERO(&set);

for(i=0;i<k;i++){

	FD_SET(pipefd[i][0],&set); 
	fd_num++;
}		



while(1){

	rdset=set; 
	errVal = select(fd_num+1,&rdset,NULL,NULL,&timeout);
	if(errno != EINTR){
		ERReq(errVal,-1,"Select in SUP failed\n");
	}

	for (fd = 3; fd<fd_num;fd++) {

		if (FD_ISSET(fd,&rdset)) {

			nread=read(fd,buf,23);
			ERReq(nread,-1,"Read of Supervisor failed\n");

			if (nread==0) {

				FD_CLR(fd,&set);
				errVal=close(fd); 
				ERReq(errVal,-1,"Close failed\n");

			}
			else { 

				printf("SUPERVISOR ESTIMATE ");


				char* token = strtok(buf, " ");
				id = strtoll(token, NULL, 16);
				

				token = strtok(NULL, " ");
				secret = strtol(token, NULL, 10);

				printf("%d FOR %lx FROM %d\n", secret, id, (fd-3));
				fflush(stdout);

				insert(t,id, secret);
						
			}
				
		} 

	}

}


return 0;
}
