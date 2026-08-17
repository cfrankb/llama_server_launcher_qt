#!/bin/bash

# Test script for LlamaServerLauncher
# This script creates sample configuration files for testing

CONFIGS_DIR="./configs"

# Create configs directory if it doesn't exist
mkdir -p "$CONFIGS_DIR"

# Create sample configuration files
cat > "$CONFIGS_DIR/Qwen3-14B-Q6_K.json" << 'EOF'
{
    "name": "Qwen3-14B-Q6_K",
    "args": "Qwen3-14B-Q6_K.gguf --gpulayers 40 --contextsize 32768 --flashattention --quantkv 1",
    "serverBin": "koboldcpp"
}
EOF

cat > "$CONFIGS_DIR/llama-server-default.json" << 'EOF'
{
    "name": "llama-server-default",
    "args": "--model model.gguf --ctx-size 32768 --port 5001 --flash-attn on",
    "serverBin": "./bin/llama-server"
}
EOF

cat > "$CONFIGS_DIR/mistral7.json" << 'EOF'
{
    "name": "mistral7",
    "args": "Mistral-7B-Instruct-v0.3-Q8_0.gguf --gpulayers 32 --contextsize 24576 --flashattention --quantkv 1 --usecublas",
    "serverBin": "koboldcpp"
}
EOF

echo "Sample configuration files created in $CONFIGS_DIR"
ls -la "$CONFIGS_DIR"
