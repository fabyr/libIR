#!/bin/bash
cd "$(dirname "$0")"
LDFLAGS="-fPIC -lm -shared" make