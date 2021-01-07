# Out-of-band-signaling
Universitary project for SOL (Sistemi Operativi). Tool that allows communication between one server and many clients using "out of band" signaling. 
Through this technique it's possible to send "false" or "useless" data instead of the real message, still allowing the correct exchange of information.
(Italian only)

1. Descrizione generale

Il progetto prevede la creazione di un sistema per l’out of band
signaling, ovvero una forma di scambio di informazioni tramite
segnalazione collaterale. Gli attori coinvolti sono un certo numero di
client e server ed un supervisor. I primi comunicano tramite socket,
mentre supervisor e server comunicano tramite pipe anonime.
Prima di procedere nella descrizione dettagliata degli Attori è bene
descrivere quali Strutture Dati e Macro siano state introdotte.

1.1.Macro

1.1.1.

Vi sono 3 macro utilizzate nella gestione degli errori:
- ERReq(Val1,Val2,Mex): controlla se Val1==Val2 e, nel
caso, stampa Mex tramite Perror()
- ERRdif(Val1,Val2,Mex): controlla se Val1!=Val2 e, nel
caso, stampa Mex tramite Perror()
- ERRmin(Val1,Val2,Mex): controlla se Val1<Val2 e, nel
caso, stampa Mex tramite Perror()
Val1 rappresenta il valore appena restituito da una certa
System Call;
Val2 rappresenta un valore “di riferimento” relativo ad una
certa SC;
Mex indica quale SC ha generato un errore.1.1.2.
Vi sono inoltre 2 macro utilizzate per effettuare la conversione
di un intero a 64bit da/verso NetworkByteOrder:
- htonll(val): concatena due htonl effettuando dei byte
shift in modo da ottenere un intero a 64 bit
- ntonll(val): concatena due ntonl effettuando dei byte
shift in modo da ottenere un intero a 64 bit



1.2. Strutture Dati

1.2.1.

Passando alle Strutture Dati è stata introdotta, tramite libreria
apposita “myHash.h”, una HashMap di supporto per il
Supervisor. In questa struttura vengono inserite le stime
inviate dai vari Server ed è strutturata come una serie di
coppie <ID,Array[2]>: la chiave è quindi l’ID di un Client,
mentre l’array contiene la migliore stima per quel Client e il
numero di Server che hanno contribuito al calcolo. E’ stata
utilizzata una Hashmap in quanto la complessità temporale per
l’operazione di inserzione, ovvero quella più frequentemente
eseguita dal Supervisor, è pari a O(n) al caso pessimo e O(1) al
caso medio.





2. Attori

2.1.Supervisor

Il Supervisor si occupa di avviare i vari Server e di comunicare con
loro, inoltre effettua lo storage dei secret. L’avvio dei Server avviene
subito dopo le operazioni iniziali (come l’installazione di un signalhandler, dichiarazione e inizializzazione di variabili utili ecc) tramite
la combinazione di fork() e una successiva execv() alla quale sono
passati come argomenti il numero identificativo del server, da 0 a k-
1, e il file descriptor di una pipe. A questo punto entra in un ciclo
infinito nel quale controlla tramite selettore se qualcuna delle pipe
contenga dei dati da leggere. I messaggi sono stringhe di 23 byte nel
formato “IDNumber Secret”, i quali vengono poi tokenizzati,
convertiti in interi e infine inviati alla Hashmap. Il Supervisor si
ferma alla ricezione di un doppio SIGINT, per evitare la possibile
presenza di errori con la Select, in quanto essa ritorna sempre se
interrotta da un segnale nonostante la presenza del flag
SA_RESTART, si è scelto, relativamente alla Select, di ignorare EINTR
e proseguire con la normale computazione.



2.2.Server

Il Server si occupa di comunicare con un certo numero di client e di
effettuare l’invio di messaggi verso il Supervisor. Il Server all’avvio
effettua una serie di operazioni preliminari, quali installazione di
signal handler e dichiarazione di variabili e strutture fondamentali
per la comunicazione tramite socket, dopodiché entra in un ciclo
infinito nel quale controlla tramite selettore il file descriptor del
Passive Socket, eseguendo l’operazione accept() quando necessario.
Per ogni nuova connessione avviata viene lanciato un thread che si
occupa di gestire sia la comunicazione con uno specifico client, sia
l’invio al Supervisor della sua stima di secret. Il Server si ferma alla
ricezione di SIGINT e, come nel caso del Supervisor, la Select ignora
EINTR.



2.3.Client

Il Client si occupa di inviare ad un certo numero di Server una
sequenza di messaggi. Inizialmente controlla che i parametri passati
da riga di comando rispettino le linee guida, dopodiché generacasualmente il Secret e l’ID (quest’ultimo viene generato
concatenando due rand() a 32bit). Tramite una funzione di shuffle
sceglie i Server a cui connettersi e inizia il ciclo principale nel quale
invia, ogni secret millisecondi, un messaggio ad uno dei Server.





3. Bash Script

3.1.Test

Lo script Test, come da consegne, si occupa semplicemente
dell’avvio di Supervisor e Client, redirigendo poi il loro output su file
specifici, e dell’invio cadenzato di SIGINT. Vengono effettuate delle
stampe su STDOUT per segnalare a quale punto della computazione
si è arrivati in un dato istante.



3.2.Misura

Lo script Misura effettua delle comparazioni fra i dati attesi e quelli
stimati dai Supervisor, prendendo in input il contenuto dei file
client.txt e supervisor.txt e restituendo in output una breve
statistica.





4. Istruzioni per l’esecuzione

L’esecuzione del Progetto avviene tramite Makefile. Nella cartella è
sufficiente eseguire l’operazione “make” per ottenere tutti gli eseguibili
necessari, seguito da “make test” per avviare lo script Test e
successivamente “make misura” per ottenere il resoconto. Inoltre tramite
“make clean” sarà possibile ripulire la cartella di lavoro da tutti gli
eseguibili e i file testuali, mentre con “make rmvtxt” verranno eliminatiesclusivamente i file testuali. Quest’ultima operazione è necessaria nel
caso si voglia effettuare una nuova esecuzione, in modo da avere i file
testuali sempre consistenti.
