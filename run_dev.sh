#!/bin/bash

# Compile C program if needed
if [ ! -f "truss_engine" ]; then
    echo "🔧 Compiling truss analysis engine..."
    gcc src/truss.c -o truss_engine -lm
fi

# Run Python GUI
echo "🚀 Starting Truss Analysis GUI..."
python src/truss.py