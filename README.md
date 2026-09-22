# Yocto GPS Live Tracker

![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=c%2B%2B&logoColor=white)
![Yocto Project](https://img.shields.io/badge/Yocto-333333?style=flat&logo=yocto&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/Raspberry_Pi-A22846?style=flat&logo=raspberry-pi&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat&logo=linux&logoColor=black)
![GPS Module](https://img.shields.io/badge/Hardware-GPS-brightgreen?style=flat)
![Client/Server](https://img.shields.io/badge/Architecture-Client%2FServer-blue?style=flat)

![GPS Live Tracker Demonstration](./assets/demo.gif)

A real-time GPS tracking system built as a custom Yocto Embedded Linux appliance for a Raspberry Pi 4B. A multithreaded C++/Qt/QML server ingests coordinate telemetry over TCP and renders the live position on an OpenStreetMap UI, while a companion mock GPS client replays a recorded route so the full pipeline can be demoed without real hardware.

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Prerequisites and Dependencies](#prerequisites-and-dependencies)
- [Installation and Environment Configuration](#installation-and-environment-configuration)
- [Usage](#usage)
- [Testing and Deployment Protocols](#testing-and-deployment-protocols)

## Architecture Overview

The system is built around a multithreaded C++17 producer-consumer server, paired with a standalone mock client for telemetry generation:

![System Architecture Diagram](./assets/architecture_diagram.png)

*Data flow: Mock Client (or real device) → Raw TCP Socket → Circular Buffer → Backend (QTimer poll) → QML Map UI*

### Network Thread (Producer)
- A detached `std::thread` runs a raw POSIX TCP socket server bound to `0.0.0.0:5100`
- Parses comma-separated ASCII coordinate strings (`<double>,<double>`) received per connection
- Pushes parsed `GPSPoint` structs into a fixed-size circular buffer (`globals.h`)

### UI/Main Thread (Consumer)
- The Qt `QGuiApplication` event loop drives a `Backend` class exposed to QML as a context property
- A `QTimer` polls the circular buffer every 33 ms (~30 Hz) and emits `coordinateChanged` when new points are consumed
- The QtQuick/QML `Map` (OpenStreetMap plugin) is bound directly to `Backend.currentCoordinate`, centering the view and repositioning a `MapQuickItem` pin at zoom level 15

### Data Synchronization
- A 1024-slot circular buffer uses `std::atomic<size_t>` head/tail indices to hand data from the network thread to the UI thread without a lock
- Single-producer/single-consumer usage keeps this safe under default (sequentially consistent) atomic ordering

### Mock GPS Client
- A separate, dependency-light C++ binary (`client/client.cpp`) that streams a recorded route (`coords.txt`) to the server over TCP, one point every 50 ms
- Used in place of physical GPS hardware for local development and demoing

## Prerequisites and Dependencies

### Build System & Compiler
- CMake (minimum version 3.10)
- BitBake (for the Yocto image build)
- C++ compiler with C++17 standard support

### Libraries & SDKs
- **Qt 5** — `Core`, `Network`, `Gui`, `Widgets`, `Qml`, `Quick`, `Location`, `Positioning`; required for the server's TCP handling, event loop, and map UI
- **POSIX Threads (`pthread`)** — required by both the server (network thread) and the mock client (send-loop timing)
- **POSIX sockets** — used directly (`sys/socket.h`, `arpa/inet.h`) by both binaries; no networking library dependency

### Hardware & OS Requirements
- Raspberry Pi 4B running a custom Yocto Embedded Linux image
- Framebuffer-only display output (`QT_QPA_PLATFORM=eglfs`) — no X11/Wayland desktop required
- Network interface (`wlan0`) provisioned via `systemd-networkd` and `wpa_supplicant` at image-build time
- A second machine on the same network to run the mock client against the Pi (or run both locally for testing)

## Installation and Environment Configuration

1. **Clone and prepare directory**

   The server and client are independent CMake projects; build them separately.

   ```text
   server/   → gps_server: TCP listener + map UI
   client/   → gps_client: mock GPS route sender
   ```

2. **Local server build**

   ```bash
   cd server
   mkdir build && cd build
   cmake ..
   make
   ```

3. **Local client build**

   ```bash
   cd client
   mkdir build && cd build
   cmake ..
   make
   ```

   **Critical fix required before building:** `client.cpp` hardcodes both the server's IP address (`192.168.1.63`) and the route file's absolute path (`ROUTE_PATH`). Update both constants to match your environment before compiling — the client will otherwise fail to connect or fail to find `coords.txt`.

4. **Yocto/BitBake integration**

   - The application is built via the `gps-server_1.0.bb` recipe in the `meta-gps` layer, which inherits `cmake_qt5` and `systemd`
   - Declared dependencies: `qtbase`, `qtdeclarative`, `qtlocation`
   - `25-wlan0.network` and `wpa_supplicant-wlan0.conf` are baked into the image to bring `wlan0` up automatically on boot

   **Security note:** `wpa_supplicant-wlan0.conf` currently ships with a real SSID/PSK committed in plaintext. Replace these with placeholder credentials before publishing the repo, and inject the real values locally (e.g. via a gitignored override) instead.

## Usage

1. **Start the server**

   ```bash
   ./gps_server
   ```

   Listens on `0.0.0.0:5100` and opens the QML map window.

2. **Feed it coordinates**

   Run the mock client to stream a full route:

   ```bash
   ./gps_client
   ```

   Or push a single point manually for a quick check:

   ```bash
   echo "30.0444,31.2357" | nc localhost 5100
   ```

   The server parses the first value as latitude and the second as longitude.

3. **Watch the live map**

   ![QML Map UI and Tracking Pin](./assets/map_ui_screenshot.png)

   The red pin and map center update in real time as points arrive.

## Testing and Deployment Protocols

### Local Socket Testing

Simulate a single telemetry update without building the client:

```bash
echo "30.0444,31.2357" | nc localhost 5100
```

### Full Route Testing

Use `gps_client` against a running `gps_server` instance to validate the end-to-end path at realistic send intervals (50 ms/point), matching how a real device would report position.

### I2C-free Deployment Check

Unlike hardware-interface projects, this system has no GPIO/I2C dependency — deployment validation is limited to network reachability and the systemd service state:

```bash
systemctl status gps-server.service
```

### Network Firewall

Port `5100` (TCP) must be reachable from any device running `gps_client`, and the Pi's `wlan0` must successfully associate using the provisioned `wpa_supplicant` configuration.
