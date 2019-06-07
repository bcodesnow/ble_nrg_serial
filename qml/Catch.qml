import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."


DualAppPage {

    errorMessageL: deviceHandler_0.error
    infoMessageL: deviceHandler_0.info

    errorMessageR: deviceHandler_1.error
    infoMessageR: deviceHandler_1.info

    property bool catchConfirmationNeeded: true
    property bool usingSDonDevice : false

    msgBoxWidth: leftContainer.width
    msgBoxOffsetFromSide: AppConstants.fieldMargin

    //    property bool dataGatheredfromBoth:
    //    property bool twoDevicesConnected:

    Rectangle
    {
        id: viewContainer
        anchors.top: parent.top
        anchors.bottom: catchButton.top
        anchors.topMargin: AppConstants.fieldMargin + messageHeightL
        anchors.bottomMargin: AppConstants.fieldMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - AppConstants.fieldMargin*2
        color: AppConstants.viewColor
        radius: AppConstants.buttonRadius


        DeviceInfoContainer {
            id: leftContainer
            anchors.left: parent.left
            anchors.top: parent.top
            state: deviceHandler_0.deviceState
            address: deviceHandler_0.deviceAddress
            fileIndex: usingSDonDevice ? "File Index: " + deviceHandler_0.fileIndexOnDevice : "BLE Mode"
            indicatorColor: deviceHandler_0.alive ? AppConstants.infoColor : AppConstants.errorColor;
            conatinerName: "LEFT"
            indicatorLeft: true
            sdEnabled: deviceHandler_0.sdEnabled
            onButtonClicked:
            {
                deviceHandler_0.requestBLESensorData();
                leftContainer.gdButtStartFastBlinking();
            }

        }
        Connections {
            target: deviceHandler_0
            onAliveArrived:
            {
                leftContainer.indicatorActive = !leftContainer.indicatorActive
            }
            onSensorDataAvailable:
            {
                leftContainer.gdButtStartSlowBlinking();
            }
            onSensorDataReceived:
            {
                leftContainer.gdButtStopBlinking();
            }
        }

        DeviceInfoContainer {
            id: rightContainer
            anchors.right: parent.right
            anchors.top: parent.top
            state: deviceHandler_1.deviceState
            address: deviceHandler_1.deviceAddress
            fileIndex: usingSDonDevice ? "File Index: " + deviceHandler_0.fileIndexOnDevice : "BLE Mode"
            indicatorColor: deviceHandler_1.alive ? AppConstants.infoColor : AppConstants.errorColor;
            conatinerName: "RIGHT"
            sdEnabled: deviceHandler_1.sdEnabled
            onButtonClicked:
            {
                deviceHandler_1.requestBLESensorData();
                rightContainer.gdButtStartFastBlinking();
            }
        }
        Connections {
            target: deviceHandler_1
            onAliveArrived:
            {
                rightContainer.indicatorActive = !rightContainer.indicatorActive
            }
            onSensorDataAvailable:
            {
                rightContainer.gdButtStartSlowBlinking();
            }
            onSensorDataReceived:
            {
                rightContainer.gdButtStopBlinking();
            }
        }

        Rectangle {
            id: midDecorator
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: bottomField.top//parent.bottom
            anchors.top: parent.top
            width: parent.width * 0.010
            height: parent.height * 0.85
            radius: height*0.5
        }

        Rectangle {
            id: bottomField
            anchors.bottom: parent.bottom
            width: parent.width
            height: AppConstants.fieldHeight
            color: AppConstants.viewColor
            radius: AppConstants.buttonRadius

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: AppConstants.fieldMargin / 2
                anchors.rightMargin: AppConstants.fieldMargin / 2

                AppButton
                {
                    id: restartButton
                    Layout.preferredWidth: parent.width / 5
                    height: parent.height*0.85
                    //                    anchors.right: rstIdxButton.left
                    //                    anchors.rightMargin: filIdxTxt.anchors.rightMargin
                    pressedColor: AppConstants.infoColor
                    baseColor: AppConstants.backgroundColor
                    color: AppConstants.backgroundColor
                    //                    anchors.margins: AppConstants.fieldMargin*0.5

//                    Text {
//                        anchors.centerIn: parent
//                        font.pixelSize: AppConstants.tinyFontSize
//                        color: AppConstants.textColor
//                        text: qsTr("START")
//                    }
                    Image {
                        anchors.centerIn: parent
                        source: "images/baseline_replay_white_18dp.png"
                    }
                    onClicked: deviceFinder.sendRestartToBothDevices();

                }
                Item
                {
                    id: spacer
                    Layout.preferredWidth: parent.width / 6
                }

                AppButton
                {
                    id: rstIdxButton
                    Layout.preferredWidth: parent.width / 6
                    height: parent.height*0.85
                    //anchors.left: parent.left
                    //                    anchors.right: filIdxTxt.left
                    //                    anchors.rightMargin: filIdxTxt.anchors.rightMargin
                    anchors.verticalCenter: parent.verticalCenter
                    pressedColor: AppConstants.infoColor
                    baseColor: AppConstants.backgroundColor
                    color: AppConstants.backgroundColor
                    //                    anchors.margins: AppConstants.fieldMargin*0.5

                                        Text {
                                            anchors.centerIn: parent
                                            font.pixelSize: AppConstants.mediumFontSize
                                            color: AppConstants.textColor
                                            text: qsTr("RST")
                                        }
//                    Image {
//                        anchors.centerIn: parent
//                        source: "images/baseline_replay_white_18dp.png"
//                    }

                    onClicked: fileHandler.rst_idx();

                }

                Text {
                    id: filIdxTxt
                    //anchors.centerIn: parent
                    Layout.preferredWidth: parent.width / 6
                    height: parent.height*0.85
                    text: fileHandler.idx
                    color: AppConstants.textColor
                    font.pixelSize: AppConstants.mediumFontSize
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                AppButton
                {
                    id: pluesButton
                    Layout.preferredWidth: parent.width / 6
                    height: parent.height * 0.85
                    //                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    pressedColor: AppConstants.infoColor
                    baseColor: AppConstants.backgroundColor
                    color: AppConstants.backgroundColor
                    //                    anchors.margins: AppConstants.fieldMargin*0.5
                    onClicked: fileHandler.incr_idx();

                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: AppConstants.mediumFontSize
                        color: AppConstants.textColor
                        text: qsTr("+")
                    }

                }
            }

        }
    }

    AppButton {
        id: catchButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: dropButton.top
        anchors.bottomMargin: AppConstants.fieldMargin*0.5
        width: viewContainer.width
        height: AppConstants.fieldHeight
        enabled: parent.catchConfirmationNeeded
        pressedColor: AppConstants.infoColor
        onClicked:
        {
            if (usingSDonDevice)
                deviceFinder.sendConfirmationToBothDevices(1);

            fileHandler.sendCatchSuccessFromQML(true);
        }

        Text {
            anchors.centerIn: parent
            font.pixelSize: AppConstants.tinyFontSize
            color: AppConstants.textColor
            text: qsTr("CATCH")
        }
    }

    AppButton {
        id: dropButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: AppConstants.fieldMargin
        width: viewContainer.width
        height: AppConstants.fieldHeight
        enabled: parent.catchConfirmationNeeded
        blinkingColor: AppConstants.errorColor
        pressedColor: AppConstants.errorColor
        onClicked:
        {
            if (usingSDonDevice)
                deviceFinder.sendConfirmationToBothDevices(0);

            fileHandler.sendCatchSuccessFromQML(true);
        }

        Text {
            anchors.centerIn: parent
            font.pixelSize: AppConstants.tinyFontSize
            text: qsTr("DROP")
            color: dropButton.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
        }
    }
}
