import QtQuick 2.9
import QtQuick.Extras 1.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import "."
//import com.dev 1.0
import Shared 1.0

AppPage {
    id: pageRoot
    property string devicesMainState : catchController.devicesMainState // why for the bloody hell is property alias not working for this case?! -> It can only refer to an object, or the property of an object, that is within the scope of the type within which the alias is declared.
    property bool devicesConnected : catchController.devicesConnected

    Connections {
        target: catchController
        onMainStateOfAllDevicesChanged:
        {
            switch(catchController.devicesMainState)
            {
            case "Unknown":
                console.log("Unknown devices main state:", catchController.devicesMainState);
                break;
            case "Stopped":

                break;
            case "Running":
                break;
            case "Ready to Trigger":
                break;
            }
            pageRoot.devicesMainState = catchController.devicesMainState // this should not be needed .. but it does not work without it!! -> why member AND signal with transfer value? i think qml ... you in the ... right there
        }
        onAllWearablesAreWaitingForDownload:
        {
            catchConfirmPopup.visible = true
        }
    }

    MultiPopup {
        id: catchConfirmPopup
        currentPopupType: MultiPopupType.type_catch
        onPopupConfirmed: {
            console.log("Ball catched:", index)
            catchController.onCatchSuccessConfirmed( index )
            catchConfirmPopup.visible = false
            if (catchController.bleUplEnabled && (index != 3 ))
                downloadProgressPopup.visible = true
        }
    }
    MultiPopup {
        id: downloadProgressPopup
        currentPopupType: 1
        maintitle: "Wearable Data Download"
        subtitle: "Downloading sensor data ..."
        indeterminate: true // false
        currentProgress: 100 // 0
    }

    Rectangle {
        id: viewContainer
        anchors.top: parent.top
        // only BlueZ platform has address type selection
        // connectionHandler.requiresAddressType ? addressTypeButton.top : searchButton.top
        anchors.topMargin: AppConstants.fieldMargin + messageHeight
        anchors.bottomMargin: AppConstants.fieldMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - AppConstants.fieldMargin*2
        height: parent.height*2/3
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
                    "Connect devices..."
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
                Text {
                    id: deviceIdentifierText
                    font.pixelSize: AppConstants.smallTinyFontSize
                    text: "#" + (model.item.deviceIndex+1) + " " + model.item.deviceIdentifier + ": " + model.item.deviceName
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
                        source: ( model.item.deviceType === DeviceType.Wearable ) ? ( model.item.connectionAlive ? "images/w_conn.png" : "images/w_disc.png" ): "images/stationary.png"
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

    Rectangle {
        id: buttonBar
        anchors.top: viewContainer.bottom
        anchors.topMargin: AppConstants.fieldMargin/2
        height: AppConstants.fieldHeight*1.3
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - AppConstants.fieldMargin*2 - AppConstants.fieldHeight*2
        color: "transparent"
        radius: AppConstants.buttonRadius
        border.color: Qt.darker(AppConstants.textColor,3)
        border.width: 1

        AppButton {
            id: startStopBtn
            anchors.centerIn: parent
            anchors.margins: 4
            width: parent.width - anchors.margins*1.5
            height: parent.height * 0.9
            enabled: pageRoot.devicesConnected && AppConstants.sessionPopupFinished
            state: "Start"

            onStateChanged: {
                console.log("start stop button changed to:",state)
            }

            states: [
                State {
                    name: "Start"; when: (pageRoot.devicesMainState == "Stopped")
                    PropertyChanges { target: startImg; opacity: 1}
                    PropertyChanges { target: stopImg; opacity: 0}
                    // PropertyChanges { target: startStopImg; source: "images/playbtn.png" }
                },
                State {
                    name: "Stop"; when: (pageRoot.devicesMainState != "Stopped" )
                    PropertyChanges { target: startImg; opacity: 0}
                    PropertyChanges { target: stopImg; opacity: 1}
                    // PropertyChanges { target: startStopImg; source: "images/stopbtn.png" }
                }
            ]
            transitions: [
                Transition {
                    NumberAnimation { property: "opacity"; easing.type: Easing.InOutQuad; duration: 2500  }
                }
            ]

            onClicked: {
                if (state === "Start")
                {
                    rotRight.start()
                    catchController.sendStartToAllDevices();
                }
                else
                {
                    rotLeft.start()
                    catchController.sendStopToAllDevices();
                }
            }

            Image {
                id: startImg
                source: "images/playbtn.png"
                anchors.centerIn: parent
                height: parent.height * 0.85
                fillMode: Image.PreserveAspectFit
                mipmap: true
                antialiasing: true
                //opacity: 1
                ColorOverlay {
                    anchors.fill: parent
                    source: parent
                    color: pageRoot.devicesConnected ? "transparent" : "gray"
                }
            }

            Image {
                id: stopImg
                source: "images/stopbtn.png"
                anchors.centerIn: parent
                height: parent.height * 0.85
                fillMode: Image.PreserveAspectFit
                mipmap: true
                antialiasing: true
                // opacity: 0
                ColorOverlay {
                    anchors.fill: parent
                    source: parent
                    color: pageRoot.devicesConnected ? "transparent" : "gray"
                }
            }

            NumberAnimation { id: rotLeft; target: stopImg; property: "rotation"; from: 360; to: 0; duration: 2000; running: false }
            NumberAnimation { id: rotRight; target: startImg; property: "rotation"; from: 0; to: 360; duration: 2000; running: false }

        } // !AppButton
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
