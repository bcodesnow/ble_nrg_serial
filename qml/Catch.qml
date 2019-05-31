import QtQuick 2.9
import QtQuick.Controls 2.2
import "."


AppPage {

    errorMessage: deviceFinder.error
    infoMessage: deviceFinder.info

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


        Rectangle
        {
            id: leftContainer
            width: parent.width / 2
            anchors.top: parent.top
            anchors.left: parent.left

            Text {
                id: titleLeft
                width: parent.width
                height: AppConstants.fieldHeight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: AppConstants.textColor
                font.pixelSize: AppConstants.mediumFontSize
                text: qsTr("LEFT")

                BottomLine {
                    height: 1;
                    width: parent.width
                    color: "#898989"
                }
            }
        }

        Rectangle
        {
            id: rightContainer
            width: parent.width / 2
            anchors.top: parent.top
            anchors.right: parent.right

            Text {
                id: titleRight
                width: parent.width
                height: AppConstants.fieldHeight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: AppConstants.textColor
                font.pixelSize: AppConstants.mediumFontSize
                text: qsTr("RIGHT")

                BottomLine {
                    height: 1;
                    width: parent.width
                    color: "#898989"
                }
            }
            Text {
                id: addresRight
                width: parent.width
                height: AppConstants.fieldHeight
                anchors.top: titleRight.bottom
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: AppConstants.textColor
                font.pixelSize: AppConstants.smallFontSize
                text: deviceHandler_0.deviceAddress

                BottomLine {
                    height: 1;
                    width: parent.width
                    color: "#898989"
                }
            }
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
