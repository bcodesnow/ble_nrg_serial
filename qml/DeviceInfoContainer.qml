import QtQuick 2.0

Rectangle
{
    id: rightLeftContainer
    width: parent.width / 2

    property int fileIndex: 0
    property string address: "FF:FF:FF:FF"
    property string state: "Unknown"
    property string conatinerName: "Unknown"

    //anchors.top: parent.top
    //anchors.right: parent.right

    Text {
        id: title
        width: parent.width
        height: AppConstants.fieldHeight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: AppConstants.textColor
        font.pixelSize: AppConstants.mediumFontSize
        text: parent.conatinerName

        BottomLine {
            height: 1;
            width: parent.width
            color: "#898989"
        }
    }
    Text {
        id: address
        width: parent.width
        height: AppConstants.fieldHeight
        anchors.top: title.bottom
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: AppConstants.textColor
        font.pixelSize: AppConstants.smallFontSize
        text: parent.address

        BottomLine {
            height: 1;
            width: parent.width
            color: "#898989"
        }
    }
    Text {
        id: state
        width: parent.width
        height: AppConstants.fieldHeight
        anchors.top: address.bottom
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: AppConstants.textColor
        font.pixelSize: AppConstants.smallFontSize
        text: parent.state

        BottomLine {
            height: 1;
            width: parent.width
            color: "#898989"
        }
    }
    Text {
        id: fileIndex
        width: parent.width
        height: AppConstants.fieldHeight
        anchors.top: state.bottom
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: AppConstants.textColor
        font.pixelSize: AppConstants.smallFontSize
        text: "File Index: " + parent.fileIndex

        BottomLine {
            height: 1;
            width: parent.width
            color: "#898989"
        }
    }
}
