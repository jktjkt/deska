import QtQuick 1.0

Rectangle {
id: serverovna
//color: "white"
width: 1200
height: 600


    DeskaDividedBox {
        id: rack_L01
        name: "L01"
        width: parent.width / 10
        height: parent.height
        color: "#ffdddd"

        innerWidth: 1
        innerHeight: 42

        rackX: 10

        DeskaDividedBox {
            rackX: 0
            rackY: 25
            color: "#aaffaa"
            Text {
                anchors.fill: parent
                text: "foo"
            }
        }

        DeskaDividedBox {
            rackX: 0
            rackY: 10
            color: "#ccccff"
            Text {
                anchors.fill: parent
                text: "bar"
            }
        }
    }

    DeskaDividedBox {
        id: rack_L02
        name: "L02"
        anchors.left: rack_L01.right
        width: parent.width / 10
        height: parent.height
    }

}
