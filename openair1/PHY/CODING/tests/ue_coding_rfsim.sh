#!/bin/bash

create_namespace() {
  local ue_id=$1
  local name="ue$ue_id"
  echo "creating namespace for UE ID ${ue_id} name ${name}"
  ip netns add $name
  ip link add v-eth$ue_id type veth peer name v-ue$ue_id
  ip link set v-ue$ue_id netns $name
  BASE_IP=$((200+ue_id))
  ip addr add 10.$BASE_IP.1.100/24 dev v-eth$ue_id
  ip link set v-eth$ue_id up
  iptables -t nat -A POSTROUTING -s 10.$BASE_IP.1.0/255.255.255.0 -o lo -j MASQUERADE
  iptables -A FORWARD -i lo -o v-eth$ue_id -j ACCEPT
  iptables -A FORWARD -o lo -i v-eth$ue_id -j ACCEPT
  ip netns exec $name ip link set dev lo up
  ip netns exec $name ip addr add 10.$BASE_IP.1.$ue_id/24 dev v-ue$ue_id
  ip netns exec $name ip link set v-ue$ue_id up
}

delete_namespace() {
  local ue_id=$1
  local name="ue$ue_id"
  echo "deleting namespace for UE ID ${ue_id} name ${name}"
  ip link delete v-eth$ue_id
  ip netns delete $name
}

usage () {
  echo "$1 <ue_args>"
}

prog_name=$(basename $0)

if [[ $(id -u) -ne 0 ]] ; then echo "Please run as root"; exit 1; fi
if [[ $# -eq 0 ]]; then echo "error: no parameters given"; usage $prog_name; exit 1; fi

# Launch gNB
./nr-softmodem --sa --rfsim --log_config.global_log_options level,nocolor,time -O ../../../openair1/PHY/CODING/tests/gnb.sa.band78.106prb.rfsim.2x2.conf &
GNB_PID=$!

sleep 3

# Create 1 network namespaces
create_namespace 1

# Write UE arguments
UE_ARGS=""
for arg in $@
do
  UE_ARGS="${UE_ARGS} ${arg}"
done

# Launch UE 1
ip netns exec ue1 ./nr-uesoftmodem --sa --rfsim -r 106 --numerology 1 --band 78 -C 3319680000 --ue-nb-ant-tx 2 --ue-nb-ant-rx 2 --rfsimulator.serveraddr 10.201.1.100 $UE_ARGS &
UE1_PID=$!

# Wait
sleep 9

# Test phase using ping
SUCCESS=0
ip netns exec ue1 ping -c 5 -i 0.1 -w 1 12.1.1.1 
SUCCESS=$((${SUCCESS}+$?))

# Stop UEs and gNB
# SIGINT
kill -2 ${UE1_PID}
sleep 3
kill -2 ${GNB_PID}
sleep 3
# SIGKILL
kill -9 ${UE1_PID}
kill -9 ${GNB_PID}

# Delete network namespaces
delete_namespace 1

exit ${SUCCESS}

