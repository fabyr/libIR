#!/bin/bash
cd "$(dirname "$0")"
make clean
LDFLAGS="-fPIC -lm -shared" make