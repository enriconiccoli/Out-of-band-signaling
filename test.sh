#! /bin/bash

echo Starting Supervisor and Servers...
./supervisor.out 8 1> tmp.txt 2> supervisor.txt &
sleep 2


for (( i=0; i<10; i++ ))
do
	./client.out 5 8 20 1>> client.txt &
	./client.out 5 8 20 1>> client.txt &
	echo Starting the couple of clients number $i &
	sleep 1
done



for (( j=0; j<5; j++ ))
do
	sleep 10 
	kill -2 $(pidof supervisor.out)
done

sleep 10 
kill -2 $(pidof supervisor.out) 
kill -2 $(pidof supervisor.out)


