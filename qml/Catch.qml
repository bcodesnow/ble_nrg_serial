import QtQuick 2.9
import QtQuick.Controls 2.2
import "."


AppPage {

    errorMessage: deviceHandler_0.error
    infoMessage: deviceHandler_0.info

    Rectangle {
        id: viewContainer
        anchors.top: parent.top
        anchors.bottom: connectButton.top
        anchors.topMargin: AppConstants.fieldMargin + messageHeight
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
            fileIndex: deviceHandler_0.fileIndexOnDevice
            conatinerName: "LEFT"
        }

        DeviceInfoContainer {
            id: rightContainer
            anchors.right: parent.right
            anchors.top: parent.top
            state: deviceHandler_1.deviceState
            address: deviceHandler_1.deviceAddress
            fileIndex: deviceHandler_1.fileIndexOnDevice
            conatinerName: "RIGHT"
        }

        Rectangle {
            id: midDecorator
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            width: parent.width * 0.025
            height: parent.height * 0.85
            radius: height*0.5
        }
    }

        AppButton {
            id: connectButton
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: searchButton.top
            anchors.bottomMargin: AppConstants.fieldMargin*0.5
            width: viewContainer.width
            height: AppConstants.fieldHeight
            visible: true //connectionHandler.requiresAddressType // only required on BlueZ
            state: "disconnected"
            onClicked:
            {
                //state == "disconnected" ? state = "disconnected" : state = "connected"
                if (state === "disconnected")
                {
                    state = "connected"
                    deviceFinder.connectToMultipleServices();
                    app.showPage("Terminal.qml")
                }
                else
                {
                    state = "disconnected"
                    // handle termination gracefully
                }
            }
            states: [
                State {
                    name: "disconnected"
                    PropertyChanges { target: addressTypeText; text: qsTr("DIESE EINE BUTTON") }
                    //PropertyChanges { target: deviceHandler; addressType: AddressType.PublicAddress }
                },
                State {
                    name: "connected"
                    PropertyChanges { target: addressTypeText; text: qsTr("DIESE EINE BUTTON") }
                    //PropertyChanges { target: deviceHandler; addressType: AddressType.RandomAddress }
                }
            ]

            Text {
                id: addressTypeText
                anchors.centerIn: parent
                font.pixelSize: AppConstants.tinyFontSize
                color: AppConstants.textColor
            }
        }

    AppButton {
        id: searchButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: AppConstants.fieldMargin
        width: viewContainer.width
        height: AppConstants.fieldHeight
        enabled: !deviceFinder.scanning
        onClicked: deviceFinder.startSearch()

        Text {
            anchors.centerIn: parent
            font.pixelSize: AppConstants.tinyFontSize
            text: qsTr("DIESE ANDERE BUTTON")
            color: searchButton.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
        }
    }
}
