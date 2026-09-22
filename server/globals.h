#ifndef GLOBALS_H
#define GLOBALS_H

#include<atomic>
#include <cstddef>

struct GPSPoint {
    double lat;
    double lon;
};

const int BUFFER_SIZE = 1024;
inline GPSPoint path_buffer[BUFFER_SIZE];

inline std::atomic<size_t> head{0};
inline std::atomic<size_t> tail{0};
#endif