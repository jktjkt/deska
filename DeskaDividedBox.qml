import QtQuick 1.0

Rectangle {
    // Number of inner positions (bays) at the horizontal level
    property int innerWidth
    // Number of inner positions (bays) at the vertical level
    property int innerHeight
    // Width of each of the inner bays
    property double bayWidth: (innerWidth != 0) ? Math.max((width / innerWidth) - 2, 1) : NaN
    // Height of each of the inner bays
    property double bayHeight: (innerHeight != 0) ? Math.max((height / innerHeight) - 2, 1) : NaN

    // Consumed bays in the vertical direction
    property int consumesBaysX: 1
    // Consumed bays in the horizontal direction
    property int consumesBaysY: 1

    property string name

    // Bay number occuiped by this item
    property int rackX
    property int rackY

    Component.onCompleted: {
        console.log("DeskaDividedBox " + name)
        console.log(" width: " + width + ", height: " + height)
        console.log(" bayWidth: " + bayWidth + ", bayHeight: " + bayHeight)
        console.log(" innerWidth: " + innerWidth + ", innerHeight: " + innerHeight)
        console.log(" rackX: " + rackX + ", rackY: " + rackY)
    }

    border.color: "black"
    border.width: 1

    width: parent.bayWidth != NaN ? consumesBaysX * parent.bayWidth : 10
    height: parent.bayHeight != NaN ? consumesBaysY * parent.bayHeight : 10
    x: rackX * parent.bayWidth
    y: rackY * parent.bayHeight
}

