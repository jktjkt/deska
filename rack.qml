import QtQuick 1.0

Rectangle {
id: serverovna
//color: "white"
width: 1200
height: 700

    DeskaDividedBox {
        id: rack_L01
        width: parent.width / 10
        height: parent.height - 100
        color: "#ffdddd"

        innerWidth: 1
        innerHeight: 42

        Text {
            text: "L01"
            anchors {
                top: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }
        }

        DeskaDividedBox {
            // a full-sized pizza box
            rackX: 0
            rackY: 25
            color: "#aaffaa"
            name: "1U pizza box"
        }

        DeskaDividedBox {
            // a 2U pizza box
            rackX: 0
            rackY: 10
            consumesBaysY: 2
            color: "#ccccff"
            name: "2U machine"
        }

        DeskaDividedBox {
        }
    }

    DeskaDividedBox {
        id: rack_L02
        name: "L02"
        anchors.left: rack_L01.right
        width: parent.width / 10
        height: parent.height - 100
    }

}
