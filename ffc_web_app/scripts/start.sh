#!/bin/bash

cd $FFC_ROOT/backend
python3 manage.py runserver 0.0.0.0:8000 &> $FFC_ROOT/logs/backend.log &
cd $FFC_ROOT/frontend
yarn run start &> $FFC_ROOT/logs/frontend.log &
cd $FFC_ROOT

