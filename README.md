# ESP32 Sensor Data Sender

This is a sample project to demostrate data upload flow from an imaging sensor end device using ESP32 to a cloud server.

# Assumption

- @TODO

# Contents

All C source files are in [main](main) folder. 

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c                 main entry point
└── README.md                  This is the file you are currently reading
```

# How to build

The project utilizes the esp-idf (ESP IoT Development Framework) and its build system.

