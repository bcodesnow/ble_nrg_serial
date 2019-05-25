
import QtQuick 2.5
import "."

Item {
    anchors.fill: parent

    property string errorMessage: ""
    property string infoMessage: ""
    property real messageHeight: msg.height
    property bool hasError: errorMessage != ""
    property bool hasInfo: infoMessage != ""

    function init()
    {
    }

    function close()
    {
        app.prevPage()
    }

    Rectangle {
        id: msg
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: AppConstants.fieldHeight
        color: hasError ? AppConstants.errorColor : AppConstants.infoColor
        visible: hasError || hasInfo

        Text {
            id: error
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            minimumPixelSize: 5
            font.pixelSize: AppConstants.smallFontSize
            fontSizeMode: Text.Fit
            color: AppConstants.textColor
            text: hasError ? errorMessage : infoMessage
        }
    }
}
