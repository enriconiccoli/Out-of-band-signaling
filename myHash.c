#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define ERRmin(val1,val2,string) if(val1 < val2){ perror(string); exit(EXIT_FAILURE);}
#define ERRdiff(val1,val2,string) if(val1 != val2){ perror(string); exit(EXIT_FAILURE);}
#define ERReq(val1,val2,string) if(val1 == val2){ perror(string); exit(EXIT_FAILURE);}


/*
	Questa hashmap è una struttura di supporto necessaria per eseguire il salvataggio dei dati inviati dai vari server al supervisor. E' composta da coppie <ID, Array[2]>, in cui
	ID rappresenta l'intero a 64 bit identificativo di un client e l'array la miglior stima relativa a quel client e il numero di server che hanno contribuito. 
	Lo scheletro di questa struttura è stato reperito online, successivamente sono state apportate varie modifiche per adeguarla alle necessità di questo progetto

	Credits to Kaushik Baruah

*/



struct node{
    int64_t key;
    int val[2];	
    struct node *next;
};

struct table{
    int size;
    struct node **list;
};


struct table *createTable(int size){
    struct table *t = (struct table*)malloc(sizeof(struct table));
    ERReq(t, NULL, "Malloc failed\n");

    t->size = size;
    t->list = (struct node**)malloc(sizeof(struct node*)*size);
    ERReq(t->list, NULL, "Malloc failed\n");

    int i;
    for(i=0;i<size;i++){
        t->list[i] = NULL;
    }
    return t;
}


int hashCode(struct table *t,int64_t key){
    if(key<0){
        return -(key%t->size);
    }
    return key%t->size;
}


/*
	Tramite questa funzione si effettua l'inserimento di un valore nella Hashmap
*/
void insert(struct table *t,int64_t key, int secret){
    int pos = hashCode(t,key);

    struct node *list = t->list[pos];

    

    struct node *temp = list;
    while(temp){
	
	// Nel caso in cui la chiave esista si controlla se il valore ad essa associato, rappresentante la migliore stima attuale, sia maggiore della variabile "secret" e, nel caso,
	// si assegna "secret" a val[0]. Successivamente viene incrementato il numero dei server partecipanti al calcolo
        if(temp->key==key){
	    if(temp->val[0] > secret){
	    	temp->val[0]= secret;
	    }	 
	    temp->val[1]=temp->val[1]+1;	
            return;
        }

        temp = temp->next;
    }


    //Se Key non corrisponde a nessun ID già presente in tabella creo una nuova coppia <ID, Arr[2]> inizializzando i valori dell'array a "secret" e 1
    struct node *newNode = (struct node*)malloc(sizeof(struct node));
    ERReq(newNode, NULL, "Malloc failed\n");

    newNode->key = key;		
    newNode->val[0] = secret;
    newNode->val[1] = 1;

    newNode->next = list;
    t->list[pos] = newNode;
}



/*
	PrintTable scorre tutta la hashmap e ne stampa il contenuto
*/
void printTable(struct table *t, int redirect){

int i=0;
while(i<20){
	struct node *list = t->list[i];
	struct node *temp = list;

	while(temp != NULL){


		if(redirect == 0){
			fprintf(stdout, "SUPERVISOR ESTIMATE %d FOR %"PRIx64" BASED ON %d\n", temp->val[0], temp->key, temp->val[1]);
			fflush(stdout);

		}
		else{
			fprintf(stderr, "SUPERVISOR ESTIMATE %d FOR %"PRIx64" BASED ON %d\n", temp->val[0], temp->key, temp->val[1]);
		}

		temp = temp->next;

	}
	i++;

}

if(redirect == 0){
	fprintf(stdout, "\n");
	fflush(stdout);
}
else{
	fprintf(stderr, "\n");
}


}


/*
 Tramite questa funzione viene effettuata la free su tutti gli elementi della Hashmap

*/
void freeTable(struct table *t){

int i=0;
while(i<20){
	struct node *list = t->list[i];
	struct node *temp = list;

	while(list != NULL){

		temp = list;
		list = list->next;
		free(temp);
	}
	i++;

}


}





