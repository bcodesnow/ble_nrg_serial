import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import "."
//import com.dev 1.0
import Shared 1.0

AppPage {
    id: pageRoot

    property string devicesMainState : catchController.devicesMainState // why for the bloody hell is property alias not working for this case?!
    property bool buttonsEnabled : catchController.devicesConnected

    Connections {
        target: catchController
        onMainStateOfAllDevicesChanged:
        {
            switch(devicesMainState)
            {
            case "Unknown":
                console.log("Unknown devices main state:",devicesMainState);
                break;
            case "Stopped":

                break;
            case "Running":
                break;
            case "Ready to Trigger":
                break;
            }

            pageRoot.devicesMainState = devicesMainState // this should not be needed .. but it does not work without it!!
        }
        onAllWearablesAreWaitingForDownload:
        {
            catchConfirmPopup.visible = true
            downloadButton.enabled = true
        }
    }

    MultiPopup {
        id: catchConfirmPopup
        popupType: 2
    }

    Rectangle {
        id: viewContainer
        anchors.top: parent.top
        anchors.bottom: testButt.top
        // only BlueZ platform has address type selection
        // connectionHandler.requiresAddressType ? addressTypeButton.top : searchButton.top
        anchors.topMargin: AppConstants.fieldMargin + messageHeight
        anchors.bottomMargin: AppConstants.fieldMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - AppConstants.fieldMargin*2
        color: AppConstants.viewColor
        radius: AppConstants.buttonRadius
        Text {
            id: title
            width: parent.width
            height: AppConstants.fieldHeight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: AppConstants.textColor
            font.pixelSize: AppConstants.mediumFontSize
            // text: qsTr(catchController.devicesMainState)
            text:
                if ( catchController.devicesMainState === "" )
                    "Connecting devices..."
                else qsTr(catchController.devicesMainState)
            BottomLine {
                height: 1;
                width: parent.width
                color: "#898989"
            }
        }

        ListView {
            id: devices
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: title.bottom
            model: ladApter
            clip: true
            spacing: 3
            delegate:
                Rectangle {
                id: box
                height:AppConstants.fieldHeight * 1.2
                width: parent.width
                color:  index % 2 === 0 ? AppConstants.delegate1Color : AppConstants.delegate2Color
                //                {
                //                    if ( model.deviceFlags & 0x01)
                //                    {
                //                        AppConstants.infoColor
                //                    }
                //                    else
                //                    {
                //                        index % 2 === 0 ? AppConstants.delegate1Color : AppConstants.delegate2Color

                //                    }
                //                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log( model.item.deviceName )
                        // devices.itemAtIndex(0).setName("Gecispina");
                        //                        if (modelData.deviceFlags & 0x01)
                        //                        {

                        //                            deviceFinder.removeDeviceFromSelection(index);
                        //                        }
                        //                        else
                        //                        {
                        //                            deviceFinder.addDeviceToSelection(index);
                        //                        }
                    }
                }
                Text {
                    id: deviceIdentifierText
                    font.pixelSize: AppConstants.smallTinyFontSize
                    text: "#" + (model.item.deviceIndex+1) + ": " + model.item.deviceIdentifier + " " + model.item.deviceName
                    anchors.top: parent.top
                    anchors.topMargin: parent.height * 0.1
                    anchors.leftMargin: parent.height * 0.1
                    anchors.left: parent.left
                    color: AppConstants.textColor
                }
                Text {
                    id: deviceMainStateText
                    font.pixelSize: AppConstants.smallTinyFontSize
                    text:
                        if ( model.item.deviceMainState === "" )
                            "Not connected"
                        else model.item.deviceMainState
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: parent.height * 0.2
                    anchors.left: parent.left
                    anchors.leftMargin: parent.height * 0.1
                    color: AppConstants.textColor
                }

                Item {
                    id: deviceTypeItem
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.topMargin: parent.height * 0.1
                    anchors.rightMargin: parent.height * 0.1
                    width: deviceTypeImg.width
                    height: deviceTypeImg.height
                    Image {
                        id: deviceTypeImg
                        source: ( model.item.deviceType === DeviceType.Wearable ) ? "images/wearable.png" : "images/stationary.png"
                        height: AppConstants.largeFontSize
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Text {
                    id: deviceAddressText
                    font.pixelSize: AppConstants.tinyFontSize
                    text: model.item.deviceAddress

                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: parent.height * 0.1
                    anchors.rightMargin: parent.height * 0.1
                    anchors.right: parent.right
                    color: Qt.darker(AppConstants.textColor)
                }
            }
        }
    }


    AppButton {
        id: testButt
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: AppConstants.fieldMargin
        width: viewContainer.width / 3
        height: AppConstants.fieldHeight
        enabled: true//downloadEnabled
        onClicked: {
            //            devices.update()
            //            console.log("asd"+            ladApter.rowCount());
            //            ladApter.rst_model()
            catchController.startDownloadFromAllDevices();
            downloadEnabled = false
            //catchController.startTimesyncAllDevices();
        }

        Text {
            anchors.centerIn: parent
            font.pixelSize: AppConstants.smallTinyFontSize
            text: qsTr("DOWNLOAD")
            color: testButt.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
        }
    }
    AppButton {
        id: startStopButton
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: AppConstants.fieldMargin
        width: viewContainer.width / 3
        height: AppConstants.fieldHeight
        enabled: pageRoot.buttonsEnabled
//        state: "Start"//(catchController.devicesMainState === "Stopped") ? "Start" : "Stop"
        onClicked: {
            console.log("state"+state)
            if (state === "Start")
                catchController.sendStartToAllDevices();
            else
                catchController.sendStopToAllDevices();
        }

        Text {
            id: startStopButtonText
            anchors.centerIn: parent
            font.pixelSize: AppConstants.smallFontSize

        }
        onStateChanged: console.log("!!! State Changed"+state)

        states: [
            State {
            name: "Start"; when: (pageRoot.devicesMainState == "Stopped")
            PropertyChanges { target: startStopButtonText; text: "START" }
            PropertyChanges { target: startStopButtonText; color: "green" }
            },
            State {
            name: "Stop"; when: (pageRoot.devicesMainState != "Stopped")
            PropertyChanges { target: startStopButtonText; text: "STOP" }
            PropertyChanges { target: startStopButtonText; color: "red" }

            }
        ]
    }

}

//DualAppPage {
//    errorMessageL: deviceHandler_0.error
//    infoMessageL: deviceHandler_0.info

//    errorMessageR: deviceHandler_1.error
//    infoMessageR: deviceHandler_1.info

//    property bool catchConfirmationNeeded: true
//    property bool usingSDonDevice : false

//    msgBoxWidth: leftContainer.width
//    msgBoxOffsetFromSide: AppConstants.fieldMargin

//    //    property bool dataGatheredfromBoth:
//    //    property bool twoDevicesConnected:

////    ProgressPopup {
////        id: devicePopup
////        visible: true
////    }

//    Rectangle
//    {
//        id: viewContainer
//        anchors.top: parent.top
//        anchors.bottom: catchButton.top
//        anchors.topMargin: AppConstants.fieldMargin + messageHeightL
//        anchors.bottomMargin: AppConstants.fieldMargin
//        anchors.horizontalCenter: parent.horizontalCenter
//        width: parent.width - AppConstants.fieldMargin*2
//        color: AppConstants.viewColor
//        radius: AppConstants.buttonRadius


//        DeviceInfoContainer {
//            id: leftContainer
//            anchors.left: parent.left
//            anchors.top: parent.top
//            state: deviceHandler_0.deviceState
//            address: deviceHandler_0.deviceAddress
//            fileIndex: usingSDonDevice ? "File Index: " + deviceHandler_0.fileIndexOnDevice : "BLE Mode"
//            indicatorColor: deviceHandler_0.alive ? AppConstants.infoColor : AppConstants.errorColor;
//            conatinerName: "LEFT"
//            indicatorLeft: true
//            sdEnabled: deviceHandler_0.sdEnabled
//            //buttonsEnabled: deviceHandler_0.writeValid
//            onButtonClicked:
//            {
//                    deviceHandler_0.requestBLESensorData();
//                    leftContainer.gdButtStartFastBlinking();
//            }

//        }
//        Connections {
//            target: deviceHandler_0
//            onAliveArrived:
//            {
//                leftContainer.indicatorActive = !leftContainer.indicatorActive
//            }
//            onSensorDataAvailableArrived:
//            {
//                leftContainer.gdButtStartSlowBlinking();
//            }
//            onSensorDataReceived:
//            {
//                leftContainer.gdButtStopBlinking();
//            }
//        }

//        DeviceInfoContainer {
//            id: rightContainer
//            anchors.right: parent.right
//            anchors.top: parent.top
//            state: deviceHandler_1.deviceState
//            address: deviceHandler_1.deviceAddress
//            fileIndex: usingSDonDevice ? "File Index: " + deviceHandler_0.fileIndexOnDevice : "BLE Mode"
//            indicatorColor: deviceHandler_1.alive ? AppConstants.infoColor : AppConstants.errorColor;
//            conatinerName: "RIGHT"
//            sdEnabled: deviceHandler_1.sdEnabled
//            //buttonsEnabled: deviceHandler_1.writeValid
//            onButtonClicked:
//            {
//                    deviceHandler_1.requestBLESensorData();
//                    rightContainer.gdButtStartFastBlinking();
//            }
//        }
//        Connections {
//            target: deviceHandler_1
//            onAliveArrived:
//            {
//                rightContainer.indicatorActive = !rightContainer.indicatorActive
//            }
//            onSensorDataAvailableArrived:
//            {
//                rightContainer.gdButtStartSlowBlinking();
//            }
//            onSensorDataReceived:
//            {
//                rightContainer.gdButtStopBlinking();
//            }
//        }

//        Rectangle
//        {
//            id: midDecorator
//            anchors.horizontalCenter: parent.horizontalCenter
//            anchors.bottom: bottomField.top//parent.bottom
//            anchors.top: parent.top
//            width: parent.width * 0.010
//            height: parent.height * 0.85
//            radius: height*0.5
//        }
//        Rectangle
//        {
//            id: bottomDecorator
//            anchors.bottom: midDecorator.bottom
//            width: parent.width
//            height: midDecorator.width
//            radius: height*0.5
//        }

//        Rectangle
//        {
//            id: bottomField
//            anchors.bottom: parent.bottom
//            width: parent.width
//            height: AppConstants.fieldHeight
//            color: AppConstants.viewColor
//            radius: AppConstants.buttonRadius

//            Rectangle
//            {
//                id: bottomLeft
//                color: parent.color
//                anchors.left: parent.left
//                anchors.top: parent.top
//                anchors.bottom: parent.bottom
//                anchors.right: parent.horizontalCenter
//                anchors.leftMargin: AppConstants.fieldMargin / 2
//                anchors.rightMargin: AppConstants.fieldMargin / 2

//                AppButton
//                {
//                    id: restartButton
//                    width: parent.width / 4
//                    height: width
//                    anchors.centerIn: parent
//                    pressedColor: AppConstants.infoColor
//                    baseColor: AppConstants.backgroundColor
//                    color: AppConstants.backgroundColor

//                    Image {
//                        anchors.centerIn: parent
//                        width: parent.width * 0.85
//                        height: width
//                        source: "images/baseline_replay_white_18dp.png"
//                    }
//                    onClicked: deviceFinder.sendRestartToBothDevices();
//                }
//            }

//            Rectangle
//            {
//                id: bottomRight
//                color: parent.color
//                anchors.right: parent.right
//                anchors.top: parent.top
//                anchors.bottom: parent.bottom
//                anchors.left: parent.horizontalCenter
//                anchors.leftMargin: AppConstants.fieldMargin / 2
//                anchors.rightMargin: AppConstants.fieldMargin / 2

//                AppButton
//                {
//                    id: rstIdxButton
//                    width: rstTxt.width * 1.2
//                    height: parent.width / 4
//                    anchors.verticalCenter: parent.verticalCenter
//                    pressedColor: AppConstants.infoColor
//                    baseColor: AppConstants.backgroundColor
//                    color: AppConstants.backgroundColor

//                    Text {
//                        id: rstTxt
//                        anchors.centerIn: parent
//                        font.pixelSize: AppConstants.mediumFontSize
//                        color: AppConstants.textColor
//                        text: qsTr("RST")
//                    }
//                    onClicked: fileHandler.rst_idx();

//                }

//                Text {
//                    id: filIdxTxt
//                    anchors.centerIn: parent
//                    width: parent.width / 4
//                    height: width
//                    text: fileHandler.idx
//                    color: AppConstants.textColor
//                    font.pixelSize: AppConstants.mediumFontSize
//                    horizontalAlignment: Text.AlignHCenter
//                    verticalAlignment: Text.AlignVCenter
//                }

//                AppButton
//                {
//                    id: pluesButton
//                    width: parent.width / 4
//                    height: width
//                    anchors.right: parent.right
//                    anchors.verticalCenter: parent.verticalCenter
//                    pressedColor: AppConstants.infoColor
//                    baseColor: AppConstants.backgroundColor
//                    color: AppConstants.backgroundColor
//                    onClicked: fileHandler.incr_idx();

//                    Text {
//                        anchors.centerIn: parent
//                        font.pixelSize: AppConstants.mediumFontSize
//                        color: AppConstants.textColor
//                        text: qsTr("+")
//                    }

//                }

//            }
//        }

//    }




//    AppButton {
//        id: catchButton
//        anchors.horizontalCenter: parent.horizontalCenter
//        anchors.bottom: dropButton.top
//        anchors.bottomMargin: AppConstants.fieldMargin*0.5
//        width: viewContainer.width
//        height: AppConstants.fieldHeight
//        enabled: parent.catchConfirmationNeeded
//        pressedColor: AppConstants.infoColor
//        onClicked:
//        {
//            if (usingSDonDevice)
//                deviceFinder.sendConfirmationToBothDevices(1);

//            fileHandler.sendCatchSuccessFromQML(true);
//        }

//        Text {
//            anchors.centerIn: parent
//            font.pixelSize: AppConstants.tinyFontSize
//            color: AppConstants.textColor
//            text: qsTr("CATCH")
//        }
//    }

//    AppButton {
//        id: dropButton
//        anchors.horizontalCenter: parent.horizontalCenter
//        anchors.bottom: parent.bottom
//        anchors.bottomMargin: AppConstants.fieldMargin
//        width: viewContainer.width
//        height: AppConstants.fieldHeight
//        enabled: parent.catchConfirmationNeeded
//        blinkingColor: AppConstants.errorColor
//        pressedColor: AppConstants.errorColor
//        onClicked:
//        {
//            if (usingSDonDevice)
//                deviceFinder.sendConfirmationToBothDevices(0);

//            fileHandler.sendCatchSuccessFromQML(false);
//        }

//        Text {
//            anchors.centerIn: parent
//            font.pixelSize: AppConstants.tinyFontSize
//            text: qsTr("DROP")
//            color: dropButton.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
//        }
//    }
//}
