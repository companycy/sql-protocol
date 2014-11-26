sqlproxy
========

To build mysql-server on ubuntu 12, boost1.55 is required.
Exec below commands to upgrade:

sudo apt-get install -y python-software-properties
sudo add-apt-repository ppa:boost-latest/ppa
sudo apt-get update
aptitude search boost
sudo apt-get install -y libboost1.55-dev libncurses5-dev bison




