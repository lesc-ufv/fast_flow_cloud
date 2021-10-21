#!/bin/bash

set -e

curl -fsSL https://rpm.nodesource.com/setup_lts.x | sudo -E bash -
sudo yum install -y nodejs
sudo pip3 install django
cd $FFC_ROOT/frontend 
sudo npm install --global yarn
yarn install -E




