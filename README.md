# Llama Server Launcher

A Qt6 application for managing and launching llama server configurations.

## Features

- **Menu Bar**: New, Open File, Open Recent Files, Save, Save As, Exit
- **Configuration Management**: Create, edit, duplicate, and delete configurations
- **Server Control**: Launch and stop buttons to run/stop the server
- **Real-time Output**: Output widget displaying server output in real-time
- **Configuration Storage**: Configs stored as JSON on disk
- **Server Binary Selection**: Global setting for server binary path with multiple options

## Building

```bash
cd LlamaServerLauncher
mkdir build && cd build
cmake ..
make
```

## Usage

1. **Select Server Binary**: Choose the llama server binary from the dropdown
2. **Enter Arguments**: Input server arguments in the text editor
3. **Save Configuration**: Use File > Save or Save As to persist configurations
4. **Launch Server**: Click the Launch button to start the server
5. **Monitor Output**: Watch real-time server output in the output widget
6. **Stop Server**: Click Stop to terminate the running server

## Configuration Files

Configurations are stored as JSON files in the `configs/` directory:

```json
{
    "name": "config_name",
    "args": "--contextsize 32768 --flashattention",
    "serverBin": "/path/to/llama-server"
}
```

## Requirements

- Qt6 (Widgets, Network modules)
- CMake 3.16 or higher
- C++17 compiler
