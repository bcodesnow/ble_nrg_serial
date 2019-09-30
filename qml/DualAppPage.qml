
import QtQuick 2.12
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

    property real msgBoxWidth: (parent.width * 0.95 ) / 2
    property real msgBoxOffsetFromSide: 5

    function init()
    {
    }

    Rectangle {
        id: msgl
        anchors.top: parent.top
        anchors.topMargin: AppConstants.fieldMargin / 2
        width: msgBoxWidth
        anchors.left: parent.left
        anchors.leftMargin: msgBoxOffsetFromSide
        height: AppConstants.fieldHeight
        color: hasErrorL ? AppConstants.errorColor : AppConstants.infoColor
        visible: hasErrorL || hasInfoL
        radius: AppConstants.buttonRadius

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
        anchors.topMargin: AppConstants.fieldMargin / 2
        width: msgBoxWidth
        anchors.right: parent.right
        anchors.rightMargin: msgBoxOffsetFromSide
        height: AppConstants.fieldHeight
        color: hasErrorR ? AppConstants.errorColor : AppConstants.infoColor
        visible: hasErrorR || hasInfoR
        radius: AppConstants.buttonRadius


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
