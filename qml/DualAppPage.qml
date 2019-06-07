
import QtQuick 2.5
import "."

Item {
    anchors.fill: parent

    property string errorMessageL: ""
    property string infoMessageL: ""
    property real messageHeightL: msgl.height
    property bool hasErrorL: errorMessageL != ""
    property bool hasInfoL: infoMessageL != ""

    property string errorMessageR: ""
    property string infoMessageR: ""
    property real messageHeightR: msgr.height
    property bool hasErrorR: errorMessageR != ""
    property bool hasInfoR: infoMessageR != ""

    function init()
    {
    }

    function close()
    {
        app.prevPage()
    }

    Rectangle {
        id: msgl
        anchors.top: parent.top
        anchors.left: parent.left
        width: ( parent.width / 2 ) * 0.95
        anchors.margins: 10
        height: AppConstants.fieldHeight
        color: hasErrorL ? AppConstants.errorColor : AppConstants.infoColor
        visible: hasErrorL || hasInfoL
        radius: height / 8

        Text {
            id: errorl
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPixelSize: 5
            font.pixelSize: AppConstants.tinyFontSize
            fontSizeMode: Text.Fit
            color: AppConstants.textColor
            text: hasErrorL ? errorMessageL : infoMessageL
        }
    }

    Rectangle {
        id: msgr
        anchors.top: parent.top
        width: parent.width / 2
        anchors.right: parent.right
        height: AppConstants.fieldHeight
        color: hasErrorR ? AppConstants.errorColor : AppConstants.infoColor
        visible: hasErrorR || hasInfoR
        radius: height / 8


        Text {
            id: errorr
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPixelSize: 5
            font.pixelSize: AppConstants.tinyFontSize
            fontSizeMode: Text.Fit
            color: AppConstants.textColor
            text: hasErrorR ? errorMessageR : infoMessageR
        }
    }
}
