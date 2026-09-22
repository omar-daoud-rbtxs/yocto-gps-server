import QtQuick 2.15
import QtQuick.Window 2.15
import QtLocation 5.15
import QtPositioning 5.15

Window {
    width: 800
    height: 600
    visible: true
    title: "GPS Live Tracker"

    // Load OpenStreetMap
    Plugin {
        id: mapPlugin
        name: "osm" 
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        zoomLevel: 15 // Street-level zoom
        
        // Lock the camera center to our C++ variable
        center: Backend.currentCoordinate

        // The UI Pin
        MapQuickItem {
            // Lock the pin location to our C++ variable
            coordinate: Backend.currentCoordinate
            
            // This ensures the exact center of the red circle points to the GPS coord
            anchorPoint.x: pinVisual.width / 2
            anchorPoint.y: pinVisual.height / 2

            sourceItem: Rectangle {
                id: pinVisual
                width: 24
                height: 24
                color: "red"
                radius: 12
                border.color: "white"
                border.width: 3
            }
        }
    }
}