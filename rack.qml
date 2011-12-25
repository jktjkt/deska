import QtQuick 1.0

Rectangle {
id: serverovna
color: "white"
width: 800
height: 1200

//Column {
//   anchors.fill: parent

    Rectangle {
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

        Text {
            text: "A"
            x: 0
            y: parent.height / 3
        }

        Text {
            text: "B"
            x: 0
            y: parent.height / 3 * 2
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
    }

//}

}
