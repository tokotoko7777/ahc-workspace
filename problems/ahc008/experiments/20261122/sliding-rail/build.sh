#!/bin/bash
set -e
ROOT=../..
RUSTC=$ROOT/ahc004_focus_assets/rustup/toolchains/stable-x86_64-unknown-linux-gnu/bin/rustc
RELEASE=$ROOT/ahc008_focus_assets/tools/target/release
deps=()
for path in "$RELEASE"/build/*/*/out; do deps+=(-L "dependency=$path"); done
"$RUSTC" --edition 2021 -O probe.rs "${deps[@]}" --extern "tools=$RELEASE/libtools.rlib" -o probe
