import QtQuick 2.0
import QtQuick.Extras 1.4


Rectangle
{
    id: rightLeftContainer
    width: (parent.width * 0.95) / 2

    property string fileIndex: "-"
    property string address: "FF:FF:FF:FF"
    property string state: "Unknown"
    property string conatinerName: "Unknown"
    property bool indicatorActive: false
    property color indicatorColor: "red"
    property bool indicatorLeft: false

    property bool sdEnabled: false

    signal buttonClicked();
    function gdButtStartSlowBlinking()
    {
        gatherDataButton.startSlowBlinking();
    }
    function gdButtStartFastBlinking()
    {
        gatherDataButton.startFastBlinking();
    }
    function gdButtStopBlinking()
    {
        gatherDataButton.stopBlinking();
    }


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
        StatusIndicator {
            id: indicator
            width: title.height * 0.66
            height: width
            anchors.verticalCenter:  title.verticalCenter
            color: indicatorColor
            active: indicatorActive
        }
        Image {
            id: sdPic
            anchors.verticalCenter: title.verticalCenter
            source: "images/baseline_sd_storage_white_18dp.png"
            visible: sdEnabled
        }

        Component.onCompleted:
        {
            if (indicatorLeft)
            {
                indicator.anchors.leftMargin = title.width / 16;
                indicator.anchors.left = title.left;
                sdPic.anchors.left = title.right;
                sdPic.anchors.leftMargin =  title.width / 16;
            }
            else
            {
                indicator.anchors.rightMargin = title.width / 16;
                indicator.anchors.right = title.right;
                sdPic.anchors.right = title.left;
                sdPic.anchors.rightMargin =  title.width / 16;
            }
        }

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
        text: parent.fileIndex

        BottomLine {
            height: 1;
            width: parent.width
            color: "#898989"
        }
    }

    AppButton {
        id: gatherDataButton
        anchors.horizontalCenter: rightLeftContainer.horizontalCenter
        anchors.top: fileIndex.bottom
        anchors.topMargin: AppConstants.fieldMargin
        anchors.bottomMargin: AppConstants.fieldMargin
        width: fileIndex.width - AppConstants.fieldMargin / 2
        height: AppConstants.fieldHeight
        enabled: true
        pressedColor: AppConstants.infoColor
        baseColor: AppConstants.backgroundColor
        color: AppConstants.backgroundColor

        Text {
            anchors.centerIn: parent
            font.pixelSize: AppConstants.tinyFontSize
            text: qsTr("Gather Data!")
            color: gatherDataButton.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
        }

        onClicked: {
            buttonClicked();
        }
    }
}
