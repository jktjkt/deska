import QtQuick 1.0

Rectangle {
id: serverovna
//color: "white"
width: 1200
height: 700

    DeskaDividedBox {
        id: rack_L01
        width: parent.width / 8
        height: parent.height - 40
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
            // a 10U blade chassis
            rackY: 30
            consumesBaysY: 10
            innerWidth: 8
            innerHeight: 2
            name: "chassis"

            DeskaDividedBox {
                rackX: 0
                rackY: 0
                name: "0.0"
            }

            DeskaDividedBox {
                rackX: 1
                rackY: 1
                name: "1.1"
            }

            DeskaDividedBox {
                rackX: 2
                name: "2.0"
                color: "yellow"
            }

            /*DeskaDividedBox {
                rackX: 8
                rackY: 2
                name: "err"
            }*/

            DeskaDividedBox {
                rackX: 7
                rackY: 0
                name: "7.0"
            }
        }
    }

    DeskaDividedBox {
        id: rack_L02
        name: "L02"
        anchors.left: rack_L01.right
        width: parent.width / 8
        height: parent.height - 40
    }

}
