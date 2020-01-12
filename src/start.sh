#!/bin/sh
cd python/control
xterm -hold -e ./control.py&

cd ../..
sleep 2
xterm -hold -e ./frontend &
sleep 0.5
xterm -hold -e ./event_builder &
sleep 0.5
xterm -hold -e ./logger &
sleep 0.5
xterm -hold -e ./analyzer &

