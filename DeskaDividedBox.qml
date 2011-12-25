import QtQuick 1.0

Rectangle {
    // Number of inner positions (bays) at the horizontal level
    property int innerWidth
    // Number of inner positions (bays) at the vertical level
    property int innerHeight
    // Width of each of the inner bays
    property double bayWidth: Math.max((parent.width / innerWidth) - 2, 1)
    // Height of each of the inner bays
    property double bayHeight: Math.max((parent.height / innerHeight) - 2, 1)

    property string name

    // Bay number occuiped by this item
    property int rackX
    property int rackY

    //onPosXChanged: alert("pwn")

    border.color: "black"
    border.width: 1

    width: parent.bayWidth
    height: parent.bayHeight
    x: rackX * parent.bayWidth
    y: rackY * parent.bayHeight

    /*Rectangle {


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
    }*/
}

