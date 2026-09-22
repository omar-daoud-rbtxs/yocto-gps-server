#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>

#define ROUTE_PATH "/home/ebmtk/Documents/EmbLnxProj/gps_images/client/coords.txt"

int main() {
    std::ifstream route_file(ROUTE_PATH);
    if (!route_file.is_open()) {
        std::cerr << "Route not found.\n";
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5100);

    if (inet_pton(AF_INET, "192.168.1.63", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed\n";
        return -1;
    }

    std::cout << "Connected! Transmitting coordinates from file...\n";

    std::string coord;
    while (std::getline(route_file, coord)) {
        if (coord.empty()) continue;

        send(sock, coord.c_str(), coord.length(), 0);
    
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    close(sock);
    route_file.close();
    std::cout << "Transmission complete.\n";
    return 0;
}