#!/bin/sh
if [ "$1" = "-d" ]; then
    EXECUTABLE="./mdriver-debug"
else
    EXECUTABLE="./mdriver"
fi

find traces/ | grep -E bal | fzf | xargs -I {} $EXECUTABLE -va -f {}
