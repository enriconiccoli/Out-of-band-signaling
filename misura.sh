#! /bin/bash

counter=0
i=0
declare -a Idarr
declare -a secarr


# Tramite questo ciclo si scorre il file supervisor.txt sino ad arrivare agli ultimi valori stampati, cioè quelli più aggiornati.
# Per fare ciò si utilizza una variabile Counter che tiene traccia di quante linee vuote incontra e, una volta arrivati a 5, si inizia a salvare 
# in due array separati gli ID e i Secret presenti in quella linea
while IFS= read -r line; do

    if [ "$counter" -eq "5" ] && [ ! -z "$line" ]; then
        IFS=' ' read -r -a array <<< "$line"
	Idarr[i]=${array[4]}
	secarr[i]=${array[2]}
        
	i=$((i+1))
    fi

    if [ -z "$line" ]; then
        counter=$((counter+1))
    fi
    
done < supervisor.txt





j=0
totalWrong=0
totalError=0

#Nel ciclo while esterno vengono controllate tutte le linee di client.txt, per ogni linea contenente la parola, e quindi successivamente anche il valore, "SECRET" viene avviato
#un ulteriore ciclo while che scorre l'array degli ID fino a trovare una corrispondenza con l'ID presente in quella specifica linea di client.txt.
#A questo punto si controlla se la stima del secret sia corretta e, nel caso non lo sia, si salvano i dati utili a generare una statistica
while IFS= read -r line; do

    if [[ $line == *"SECRET"* ]]; then

        IFS=' ' read -r -a array <<< "$line"
	j=0
	found=0
        until [ $found -eq 1 ]; do 

	    if [[ "${array[1]}" == "${Idarr[j]}" ]]; then
		
		valMax=25
		valMax=$((valMax+array[3]))

		valMin=25
		valMin=$((array[3]-valMin))
		
		
		if (( secarr[j] > valMax )) || (( secarr[j] < valMin )); then
			totalWrong=$((totalWrong+1))
			tmp=$((array[3]-secarr[j]))
			if (( tmp<0 )); then
				tmp=$((-tmp))
			fi
			totalError=$((totalError+tmp))
		fi
		
		found=1

	    fi
            j=$((j+1))
	done
    fi

done < client.txt

if [[ $totalWrong != "0" ]]; then
	media=$((totalError/totalWrong))
	echo "On a total of $i valutation, there are $totalWrong wrong values. The average error is $media"
else
	echo "On a total of $i valutation, there aren't wrong values."
fi

















 
