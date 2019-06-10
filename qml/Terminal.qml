import QtQuick 2.9
import QtQuick.Controls 2.2
import "."

// Orientation 1 Portrait, Orientation 2 Landscape
//ApplicationWindow {
//    id: rootWindow
//    visible: true
//    width: Screen.width //640
//    height: Screen.height //480
//    title: qsTr("BLE UART Terminal")

AppPage
{

    property int idOfDevHandlerReceivingTerminalMsgs: 0

    Switch {
        id: autoscrollSW
        anchors.top: terminalBackground.bottom //( Screen.orientation === Qt.PortraitOrientation  ) ? terminalBackground.bottom : parent.top
        anchors.left: parent.left //( Screen.orientation === Qt.PortraitOrientation  ) ? parent.left : terminalBackground.right
        anchors.margins: width
        height: parent.height / 40
        width: height * 2
        contentItem:
            Text {
            text: "Turn OFF Autoscroll"
            font.pixelSize: AppConstants.tinyFontSize
            color: AppConstants.textColor
            anchors.left: parent.right
            anchors.leftMargin: parent.width / 2
            verticalAlignment: Text.AlignVCenter

        }
        onCheckedChanged:
        {
            terminalListView.autoScrollEnabled = !terminalListView.autoScrollEnabled
        }
    }

    Switch {
        id: deviceSelector
        anchors.top: autoscrollSW.bottom //( Screen.orientation === Qt.PortraitOrientation  ) ? terminalBackground.bottom : parent.top
        anchors.left: parent.left //( Screen.orientation === Qt.PortraitOrientation  ) ? parent.left : terminalBackground.right
        anchors.margins: width
        height: parent.height / 40
        width: height * 2
        contentItem:
            Text {
            text: "Device 0 <-> Device 1"
            font.pixelSize: AppConstants.tinyFontSize
            color: AppConstants.textColor
            anchors.left: parent.right
            anchors.leftMargin: parent.width / 2
            verticalAlignment: Text.AlignVCenter

        }
        onCheckedChanged:
        {
            if (idOfDevHandlerReceivingTerminalMsgs === 0)
            {
                idOfDevHandlerReceivingTerminalMsgs = 1;
            }
            else
            {
                idOfDevHandlerReceivingTerminalMsgs = 0;
            }
        }
    }

    Switch {
        id: sdEnableSW
        anchors.top: deviceSelector.bottom //( Screen.orientation === Qt.PortraitOrientation  ) ? terminalBackground.bottom : parent.top
        anchors.left: parent.left //( Screen.orientation === Qt.PortraitOrientation  ) ? parent.left : terminalBackground.right
        anchors.margins: width
        height: parent.height / 40
        width: height * 2

        contentItem: Text {
            font.pixelSize: AppConstants.tinyFontSize
            text: "Turn On SD Card Logging"
            color: AppConstants.textColor
            anchors.left: parent.right
            anchors.leftMargin: parent.width / 2
            verticalAlignment: Text.AlignVCenter
        }
        onCheckedChanged:
        {
            if ( checked )
                deviceFinder.sendEnableSDtoBothDevices(true);
            else
                deviceFinder.sendEnableSDtoBothDevices(false)
        }
    }


    Connections {
        target: terminalToQml
        onMessageArrived:
        {
            terminalModel.append({ "str": str, "clr": clr, "fmt": fmt});
        }
    }

    ListModel {
        id: terminalModel
        Component.onCompleted:
        {
            terminalToQml.isActive = true;
        }
    }

    Rectangle
    {
        id: terminalBackground
        property int marginsLeftRight: 32
        anchors.left: parent.left
        anchors.leftMargin: marginsLeftRight
        anchors.top: parent.top
        anchors.topMargin: marginsLeftRight
        width:  parent.width - marginsLeftRight*2// ) ( Screen.orientation === Qt.PortraitOrientation  ) ? ( parent.width - marginsLeftRight*2 ) : ( ( parent.width / 2 ) - marginsLeftRight*2 )
        height: parent.height / 2  // ( Screen.orientation === Qt.PortraitOrientation  ) ? ( parent.height / 2 ) * 0.95 :  parent.height * 0.95
        radius: parent.height / 32
        color: "gray"
        gradient: Gradient {
            GradientStop { position: 0.0; color: "gray" }
            GradientStop { position: 1.0; color: "black" }
        }

        Rectangle
        {
            width: parent.width * 0.90
            height: parent.height * 0.90
            color: AppConstants.backgroundColor //"gray"
            anchors.centerIn: parent

            ListView {
                id: terminalListView
                property bool autoScrollEnabled: true
                flickableDirection: Flickable.VerticalFlick
                flickDeceleration: 50
                boundsBehavior: Flickable.StopAtBounds
                width: parent.width
                height: parent.height * 0.7
                anchors.left: parent.left
                anchors.top: parent.top
                spacing: parent.height / 64
                cacheBuffer: 200
                model: terminalModel
                delegate:
                    Text {
                    color: clr
                    width: parent.width
                    font.pixelSize: 10
                    text: str
                    font.bold: fmt ? true : false
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
                onCountChanged: {
                    if (autoScrollEnabled)
                        terminalListView.currentIndex = count - 1
                }

                ScrollBar.vertical: ScrollBar {}
            }

            Rectangle
            {
                id: terminalInputBlock
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                color: "whitesmoke"
                height: parent.height * 0.2
                width: parent.width
                radius: height / 16

                TextEdit
                {
                    id: txtInput
                    anchors.top: parent.top
                    anchors.left: parent.left
                    width: parent.width * 0.75
                    height: parent.height
                    font.pixelSize: AppConstants.tinyFontSize

                    property string lastCMD: "get_state()"

                    Keys.onUpPressed:
                    {
                        txtInput.text = lastCMD;
                    }
                    Keys.onDownPressed:
                    {
                        txtInput.clear();
                    }
                    Keys.onReturnPressed:
                    {
                        sendCMD();
                    }
                    function sendCMD()
                    {
                        if(idOfDevHandlerReceivingTerminalMsgs)
                            deviceHandler_1.sendCMDStringFromTerminal(txtInput.text);
                        else
                            deviceHandler_0.sendCMDStringFromTerminal(txtInput.text);
                        lastCMD = txtInput.text;
                        txtInput.clear();
                    }
                }
                RoundButton
                {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    width: parent.width * 0.2
                    height: parent.height
                    radius: terminalInputBlock.radius
                    text: "Send"
                    font.pixelSize: AppConstants.tinyFontSize
                    onClicked:
                    {
                        txtInput.sendCMD();
                    }
                }
            }
        }
    }
}
