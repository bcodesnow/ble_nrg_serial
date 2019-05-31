import QtQuick 2.9
import QtQuick.Controls 2.2
import "."


AppPage {

    errorMessage: deviceHandler_0.error
    infoMessage: deviceHandler_0.info

    property bool catchConfirmationNeeded: false

    Rectangle {
        id: viewContainer
        anchors.top: parent.top
        anchors.bottom: catchButton.top
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
        id: catchButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: dropButton.top
        anchors.bottomMargin: AppConstants.fieldMargin*0.5
        width: viewContainer.width
        height: AppConstants.fieldHeight
        enabled: parent.catchConfirmationNeeded
        onClicked:
        {
            deviceFinder.sendConfirmationToBothDevices(1);
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
        onClicked:
        {
            deviceFinder.sendConfirmationToBothDevices(0);
        }

        Text {
            anchors.centerIn: parent
            font.pixelSize: AppConstants.tinyFontSize
            text: qsTr("DROP")
            color: dropButton.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
        }
    }
}
