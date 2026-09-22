#pragma once
#include <QObject>
#include <QTimer>
#include <QGeoCoordinate>
#include <atomic>
#include "globals.h"

class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate currentCoordinate READ currentCoordinate NOTIFY coordinateChanged)

public:
    explicit Backend(QObject *parent = nullptr) : QObject(parent) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Backend::processBuffer);
        timer->start(33); 
    }

    QGeoCoordinate currentCoordinate() const { 
        return m_coordinate; 
    }

signals:
    void coordinateChanged();

private slots:
    void processBuffer() {
        bool changed = false;
        
        while (tail != head) {
            GPSPoint point = path_buffer[tail % BUFFER_SIZE];
            
            m_coordinate.setLatitude(point.lon);
            m_coordinate.setLongitude(point.lat);
            
            tail++;
            changed = true;
        }
        
        if (changed) {
            emit coordinateChanged();
        }
    }

private:
    QTimer *timer;
    QGeoCoordinate m_coordinate;
};