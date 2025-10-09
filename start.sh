#!/bin/bash

DIR="$HOME/code/caesar"

if tmux has-session -t caesar 2>/dev/null; then
  tmux kill-session -t caesar
fi


tmux new-session -d -s caesar -n editor -c "$DIR"
tmux set-environment -t caesar VCPKG_ROOT "/Users/avenue/code/caesar/vcpkg"
tmux send-keys -t caesar:editor "nvim ." C-m

tmux new-window -d -n compiler -c "$DIR/build"

tmux new-window -d -n misc -c "$DIR"

tmux attach-session -t caesar
