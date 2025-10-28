#!/bin/sh

find traces/ | grep -E bal | fzf | xargs -I {} gdbserver :1234 ./mdriver-debug -va -f {}
