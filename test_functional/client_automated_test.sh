#!/bin/bash
SERVER="127.0.0.1"
TEMPLATE_FILE="template.cfg"
TEST_CASES_FILE="test_cases.txt"
CLIENTS=('TEST_CLIENT1' 'TEST_CLIENT2' 'TEST_CLIENT3' 'TEST_CLIENT4' 'TEST_CLIENT5' 'TEST_CLIENT6' 'TEST_CLIENT7' 'TEST_CLIENT8');

#Sudoable test
sudo ifconfig
initrv=$?
clear

function display_usage()
{
	echo ""
	echo "chmod +x ./client_automated_test.sh"
	echo "./client_automated_test.sh"
	echo ""
}

function run_client()
{
	local client_name=$1
	./client $TEMPLATE_FILE $SERVER $client_name $TEST_CASES_FILE
}

if [[ $initrv > 0 ]]
then
	echo "Current user needs to have root rights in order to execute OME"
	display_usage  
	exit 1
fi

echo ""
echo "Client automation is starting :"
echo ""

start=$(($(date +%s%N)/1000000))
for i in "${CLIENTS[@]}"
do
	echo "Starting $i"
	run_client $i &
done

#Wait for all forked child processes
wait

finish=$(($(date +%s%N)/1000000))
current_execution_time=$(($finish-$start))
echo ""
echo "Time : $current_execution_time milliseconds "


echo ""
echo "Client automation finished , press enter key to quit."
echo ""
read