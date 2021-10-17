#!/bin/bash

set -e

curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -
sudo apt-get install -y nodejs
pip3 install django

cd $FFC_ROOT/frontend 
sudo npm install --global yarn
yarn install -E




