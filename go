#!/bin/bash
echo
echo
echo
echo "Compiling..."


CHOICE=${1:-"1"}
Remove=${2:-"1"}
if [ $Remove == "1" ]; then
    rm -f HayStackFile
    rm *.jpeg
fi    
    
execute="./run 9646"
execute="./run 9647"
make
if [ "$?" -eq "0" ]; then

	if [ $CHOICE == "1" ]; then
		exec=$execute
 	elif [ $CHOICE == "2" ]; then
 		exec=$execute2
 	fi
 
 	echo $exec
 	
 	if [ "$1" == "-d" ]; then
  		echo "Debuging..."
  		echo "----"
   		valgrind --tool=memcheck  --leak-check=full --track-origins=yes --show-reachable=yes ${exec}
	 	echo "----"
 		echo "END"
 	else	
 		
   			echo "Running..."
			echo "----"
   			$exec
   			echo "----"
 			echo "END"
  		
 	fi
fi
# make clean
echo

