import QtQuick 2.9
import QtQuick.Controls 2.2
import "."


DualAppPage {

    errorMessageL: deviceHandler_0.error
    infoMessageL: deviceHandler_0.info

    errorMessageR: deviceHandler_1.error
    infoMessageR: deviceHandler_1.info

    property bool catchConfirmationNeeded: true
    property bool usingSDonDevice : false

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
            onButtonClicked:
            {
                deviceHandler_0.requestBLESensorData();
            }

        }
        Connections {
            target: deviceHandler_0
            onAliveArrived:
            {
                leftContainer.indicatorActive = !leftContainer.indicatorActive
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
        }
        Connections {
            target: deviceHandler_1
            onAliveArrived:
            {
                rightContainer.indicatorActive = !rightContainer.indicatorActive
            }
        }

        Rectangle {
            id: midDecorator
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: fileIndexBLE.top//parent.bottom
            anchors.top: parent.top
            width: parent.width * 0.010
            height: parent.height * 0.85
            radius: height*0.5
        }

        Rectangle {
            id: fileIndexBLE
            anchors.bottom: parent.bottom
            width: parent.width
            height: AppConstants.fieldHeight
            color: AppConstants.viewColor
            radius: AppConstants.buttonRadius

            AppButton
            {
                width: parent.width / 5
                height: width * 0.5
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                pressedColor: AppConstants.infoColor
                baseColor: AppConstants.backgroundColor
                color: AppConstants.backgroundColor
                anchors.margins: AppConstants.fieldMargin*0.5

                Text {
                    anchors.centerIn: parent
                    font.pixelSize: AppConstants.mediumFontSize
                    color: AppConstants.textColor
                    text: qsTr("RST")
                }
                onClicked: fileHandler.rst_idx();

            }

            Text {
                id: filIdxTxt
                anchors.centerIn: parent
                text: fileHandler.idx
                color: AppConstants.textColor
                font.pixelSize: AppConstants.mediumFontSize
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            AppButton
            {
                width: parent.width / 5
                height: width * 0.5
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                pressedColor: AppConstants.infoColor
                baseColor: AppConstants.backgroundColor
                color: AppConstants.backgroundColor
                anchors.margins: AppConstants.fieldMargin*0.5
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
            else
            {
                fileHandler.sendCatchSuccessFromQML(true);
            }
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
            catchButton.startBlinking();
            if (usingSDonDevice)
                deviceFinder.sendConfirmationToBothDevices(0);
            else
            {
                fileHandler.sendCatchSuccessFromQML(true);
            }
        }

        Text {
            anchors.centerIn: parent
            font.pixelSize: AppConstants.tinyFontSize
            text: qsTr("DROP")
            color: dropButton.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
        }
    }
}
