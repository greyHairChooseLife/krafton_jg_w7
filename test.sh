#!/bin/sh
find traces/ | grep -E bal | fzf | xargs -I {} ./mdriver -vl -f {}
