# ESP32 Sensor Data Sender

This is a sample project to demostrate data upload flow from an imaging sensor end device using ESP32 to a cloud server.

# Assumption

Here are some of the assumptions made:

- The code does not deal data acquisition from actual sensor. The data is asssumed to be a binary blob that will be sent to a server a long with some metadata about the sensor.

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

The project utilizes the `esp-idf`(ESP IoT Development Framework) and its build system.
The steps are listed here to quickly get the host system ready to build the sample.
These steps assume the host system is an Ubuntu (or Debian-based) and this project is cloned at `~/esp/esp_sensor_data_sender`

1. Install essential build tools
```
sudo apt-get install git wget flex bison gperf python3 python3-pip python-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util
```

2. Make python3 the default (Recommended)
```
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10
```

3. Get the esp-idf
```
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
```

4. Install tools
```
cd ~/esp/esp-idf
./install.sh
```

5. Setup environment
```
. ./export.sh
```

6. Build
```
cd ~/esp/esp_sensor_data_sender
idf.py set-target esp32
idf.py build
```

If you have a ESP32, you may try flash and run this application by `idf.py -p PORT flash`

For detail/troubleshooting, please refer to ["ESP-IDF Getting Started"](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started).

