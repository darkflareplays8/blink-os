import QtQuick 2.0
import calamares.slideshow 1.0

Presentation {
    id: presentation

    function nextSlide() {
        presentation.currentSlide++
        if (presentation.currentSlide >= presentation.slideCount)
            presentation.currentSlide = 0
    }

    Timer {
        interval: 4000
        running: presentation.activatedInCalamares
        repeat: true
        onTriggered: nextSlide()
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0a0f"

            Rectangle {
                anchors.centerIn: parent
                width: 2
                height: parent.height * 0.4
                color: "#00d4ff"
                opacity: 0.4
            }

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -60
                text: "Blink OS"
                font.pixelSize: 48
                font.family: "Inter"
                font.weight: Font.Light
                color: "#ffffff"
                letterSpacing: 8
            }

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 10
                text: "Fast. Clean. Yours."
                font.pixelSize: 16
                font.family: "Inter"
                color: "#00d4ff"
                letterSpacing: 4
                opacity: 0.8
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0a0f"

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -40
                text: "Built on Debian."
                font.pixelSize: 28
                font.family: "Inter"
                font.weight: Font.Light
                color: "#ffffff"
                letterSpacing: 2
            }

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 10
                text: "Rock solid. No bloat. Runs on anything."
                font.pixelSize: 14
                font.family: "Inter"
                color: "#aaaaaa"
                letterSpacing: 1
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0a0f"

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -40
                text: "Familiar from day one."
                font.pixelSize: 28
                font.family: "Inter"
                font.weight: Font.Light
                color: "#ffffff"
                letterSpacing: 2
            }

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 10
                text: "If you've used Windows, you already know Blink."
                font.pixelSize: 14
                font.family: "Inter"
                color: "#aaaaaa"
                letterSpacing: 1
            }
        }
    }

    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0a0f"

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -40
                text: "Almost there."
                font.pixelSize: 28
                font.family: "Inter"
                font.weight: Font.Light
                color: "#00d4ff"
                letterSpacing: 2
            }

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 10
                text: "Setting up your new system."
                font.pixelSize: 14
                font.family: "Inter"
                color: "#aaaaaa"
                letterSpacing: 1
            }
        }
    }
}
