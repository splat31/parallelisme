wsl -d Ubuntu
sudo apt update
sudo apt install openmpi-bin python3-matplotlib libopenmpi-dev
sudo apt install python3-pip
sudo apt install python3-venv python3-dev libopenmpi-dev openmpi-bin
python3 -m venv .venv
source .venv/bin/activate
desactivate