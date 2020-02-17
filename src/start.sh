#!/bin/sh
pkill frontend
pkill event_builder
pkill logger
pkill analyzer
 ./frontend &
 ./event_builder &
 ./logger &
 ./analyzer &

