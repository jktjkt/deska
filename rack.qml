import QtQuick 1.0

Rectangle {
id: serverovna
color: "white"
width: 800
height: 1200

    /*Rectangle {
        id: box_L01
        border.width: 1
        border.color: "black"
        width: parent.width / 10
        height: parent.height / 2
        anchors {
            left: parent.left
            top: parent.top
            topMargin: label.height
            leftMargin: width / 20
        }

        Text {
            id: label
            text: "L01"
            anchors.bottom: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: width / 20
        }


        // Below are various boxes which are placed directly in this rack. In contrast to the machine room, each rack is strictly
        // divided into a number of fixed positions.  That's why we utilize the absolute positioning at the rack level.

        // Number of inner positions (bays) at the horizontal level
        property int innerWidth: 1
        // Number of inner positions (bays) at the vertical level
        property int innerHeight: 42
        // Width of each of the inner bays
        property double bayWidth: Math.max((parent.width / innerWidth) - 2, 1)
        // Height of each of the inner bays
        property double bayHeight: Math.max((parent.height / innerHeight) - 2, 1)

        Rectangle {
            // Bay number occuiped by this item
            property int posX: 1
            property int posY: 1

            width: parent.bayWidth
            height: parent.bayHeight

            x: posX * parent.bayWidth
            y: posY * parent.bayHeight

            border.width: 1
            border.color: "blue"

            Text {
                text: "Sample box"
                anchors.fill: parent
            }
        }
    }

    Rectangle {
        anchors.left: box_L01.right
        id: box_L02
        border.width: 1
        border.color: "black"
        width: parent.width / 10
        height: parent.height / 2

        Text {
            text: "L02"
        }
    }*/

    DeskaDividedBox {
        id: rack_L01
        name: "L01"
        width: parent.width / 10
        height: parent.height / 2
        //color: "red"

        innerWidth: 1
        innerHeight: 42

        rackX: 10

        DeskaDividedBox {
            rackX: 0
            rackY: 25
            color: "green"
            /*Text {
                anchors.fill: parent
                text: "foo"
            }*/
        }

        DeskaDividedBox {
            rackX: 0
            rackY: 10
            color: "blue"
            /*Text {
                anchors.fill: parent
                text: "bar"
            }*/
        }
    }

    DeskaDividedBox {
        id: rack_L02
        name: "L02"
        anchors.left: rack_L01.right
        width: parent.width / 10
        height: parent.height / 2
    }

}
