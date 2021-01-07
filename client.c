#include<stdio.h>
#include<stdlib.h>
#include<inttypes.h>
#include<arpa/inet.h>
#include<time.h>
#include<string.h>
#include<sys/unistd.h>
#include<errno.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include<sys/un.h>
#include<time.h>



#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ERRmin(val1,val2,string) if(val1 < val2){ perror(string); exit(EXIT_FAILURE);}
#define ERRdiff(val1,val2,string) if(val1 != val2){ perror(string); exit(EXIT_FAILURE);}
#define ERReq(val1,val2,string) if(val1 == val2){ perror(string); exit(EXIT_FAILURE);}




/*
	Questa è una funzione di shuffle che si occupa di generare e gestire un array dinamico di SIZE posizioni. Si occupa quindi anche dell'estrazione di valori random e della loro successiva rimozione 		dall'array, in modo da poter effettuare nuovamente un'altra estrazione senza avere la possibilità di incontrare doppioni.

	Credits to: "paxdiablo" on Stackoverflow 
*/
int myRandom(int size){
	int i, n;
	static int numNums = 0;
	static int *numArr = NULL;



	//Se size >=0 viene generato un array di size posizioni, con valori da 0 a size-1
	if (size >= 0) {
        	if (numArr != NULL){
            		free (numArr);
		}
		if ((numArr = malloc (sizeof(int) * size)) == NULL){
			perror("No mem\n");
			exit(EXIT_FAILURE);
		}

		for (i = 0; i  < size; i++){
			numArr[i] = i;
		}
		numNums = size;
    	}

	//size == -2 utilizzato come valore speciale per ripulire la memoria 
   	if(size == -2){
		free(numArr);
		return -2;
    	}


	//Se array vuoto viene restituito -1
    	if (numNums == 0){
       		return -1;
    	}

   
	//Estrazione di un elemento e rimozione dello stesso. La sostituzione di quest'ultimo con numArr[numNums-1] (ultimo valore dell'array) e il successivo decremento di numNums fanno sì che 
	//i numeri già estratti non possano ripetersi e che tutti i numeri in celle di indice maggiore di numNums non siano raggiungibili
    	n = rand() % numNums;
    	i = numArr[n];
   	numArr[n] = numArr[numNums-1];
   	numNums--;
    	if (numNums == 0) {
		free (numArr);
		numArr = 0;
    	}

    	return i;
}










int main(int argc, char* argv[]){

ERRdiff(argc, 4, "Wrong number of arguments\n");

int p,k,w, i, j,errVal,res;
int fd[p];
int64_t id, idNO;
int32_t secret;
char buf[20];


p = strtol(argv[1], NULL, 10);
k = strtol(argv[2], NULL, 10);
w = strtol(argv[3], NULL, 10);

//Controllo della conformità dei valori tramite macro
ERRmin(p,1,"Wrong values\n");
ERRmin(k,p,"Wrong values\n");
ERRmin(w,(3*p),"Wrong values\n");


//Generazione casuale di secret e dell'ID a 64 bit
srand(time(NULL)*getpid());
secret = (rand() %3000)+1;
id = rand();
id = (id << 32) | rand();


printf("CLIENT %"PRIx64 " SECRET %" PRId32 "\n", id, secret);


struct sockaddr_un sa;
sa.sun_family=AF_UNIX;


//In questo ciclo vengono scelti casualmente i p server facendo uso della funzione di shuffle descritta precedentemente
res = myRandom(k);
j = 0;
while(j < p){

		
		char num[snprintf(NULL,0,"%d",res)+1];
		errVal = sprintf(num, "%d", res);
		ERRmin(errVal,0,"Sprintf failed\n");
		
		//Socket definito unendo la stringa sock al numero appena estratto
		char sock[16] = "./OOB-server-";	
		strcat(sock, num);	

		strcpy(sa.sun_path, sock);
		fd[j]=socket(AF_UNIX,SOCK_STREAM,0);	
		ERReq(fd[j],-1,"Socket failed\n");
		
		errVal = connect(fd[j],(struct sockaddr*)&sa, sizeof(sa));
		ERReq(errVal,-1,"Connect failed\n");
	

		res = myRandom(-1);
		j++;

}
myRandom(-2);


//Uso della macro per conversione in NBO e successivo inserimento del valore nel buffer
idNO = htonll(id);
errVal = sprintf(buf, "%"PRId64" ", idNO);
ERRmin(errVal,0,"Sprintf failed\n");



struct timespec tim;
tim.tv_sec = (time_t) (secret/1000);
tim.tv_nsec = (time_t) ((secret%1000)*1000000);


//Scelta casuale del server a cui inviare dati
j=0;
while(j<w){

	i=(rand() %p);
	errVal = write(fd[i],buf, 20);
	ERReq(errVal,-1,"Client Write failed");

	errVal = nanosleep(&tim, NULL);
	ERReq(errVal,-1,"Nanosleep failed");

	j++;
}


for(i=0;i<p;i++){
	errVal = close(fd[i]);
	ERReq(errVal,-1,"Close failed");
}

printf("CLIENT %"PRIx64" DONE\n", id);

return 0;
}
