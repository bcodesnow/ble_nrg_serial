import QtQuick 2.9
import QtQuick.Extras 1.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import "."
//import com.dev 1.0
import Shared 1.0

AppPage {
    id: pageRoot
    property string devicesMainState : catchController.devicesMainState // why for the bloody hell is property alias not working for this case?! -> It can only refer to an object, or the property of an object, that is within the scope of the type within which the alias is declared.
    property bool devicesConnected : catchController.devicesConnected
    errorMessage: devicesConnected ? (devicesMainState == "Error" ? "Error" : "") : "No devices connected."
    infoMessage: devicesMainState != "Error" ? devicesMainState : ""

    Connections {
        target: catchController
        onMainStateOfAllDevicesChanged:
        {
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
        property int currentModeIndexCatch//: sessionSettingsPopup.currentModeIndex
        Connections {
            target: sessionSettingsPopup
            onCurrentModeIndexChanged: {
                catchConfirmPopup.currentModeIndexCatch = sessionSettingsPopup.currentModeIndex
            }
        }
        onCurrentModeIndexCatchChanged: {
             fileHandler.setCurrCatchMode(currentModeIndexCatch)
        }
        onPopupConfirmed: {
            catchController.onCatchSuccessConfirmed( index )
            catchConfirmPopup.visible = false
            if (catchController.bleUplEnabled && (index != 3 ))
                downloadProgressPopup.visible = true
        }
        onVisibleChanged: {
            if (visible) view.setCurrentIndex(1)
        }
    }
    MultiPopup {
        id: downloadProgressPopup
        currentPopupType: MultiPopupType.type_progress
        modal: false
        maintitle: "Wearable Data Download"
        subtitle: "Downloading sensor data ..."
        indeterminate: true // false8
        currentProgress: 100 // 0
        Component.onCompleted: {
            // todo: start popup animations at this point,
            //       not on loader windowChanged() signal
        }
        Audio {
            id: downlFinMusic
            autoLoad: true
            // autoPlay: true
            audioRole: Audio.AlarmRole
            source: "dlfin.mp3"
            muted: false
            volume: 0.5
        }
        onVisibleChanged: {
            downlFinMusic.play()
            console.log("play music:",downlFinMusic.duration)
        }
    }

    Rectangle {
        id: viewContainer
        anchors.top: parent.top
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
            text: "Connected devices"
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
                        source: model.item.deviceType === DeviceType.Wearable ? "images/wearable.bmp" : "images/stationary.png"
                        height: AppConstants.largeFontSize
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        mipmap: true
                        antialiasing: true
                        ColorOverlay {
                            anchors.fill: parent
                            source: parent
                            color: model.item.connectionAlive ? AppConstants.infoColor : AppConstants.errorColor
                        }
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
        //        anchors.top: viewContainer.bottom
        //        anchors.topMargin: AppConstants.fieldMargin/2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: AppConstants.fieldMargin
        height: AppConstants.fieldHeight*1.3
        width: parent.width - AppConstants.fieldMargin*2
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

            states: [
                State {
                    name: "Start"; when: (pageRoot.devicesMainState == "Stopped")
                    PropertyChanges { target: startImg; opacity: 1}
                    PropertyChanges { target: stopImg; opacity: 0}
                },
                State {
                    name: "Stop"; when: (pageRoot.devicesMainState != "Stopped" )
                    PropertyChanges { target: startImg; opacity: 0}
                    PropertyChanges { target: stopImg; opacity: 1}
                }
            ]
            transitions: [
                Transition {
                    NumberAnimation { property: "opacity"; easing.type: Easing.InOutQuad; duration: 1000  }
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

            NumberAnimation { id: rotLeft; target: stopImg; property: "rotation"; from: 360; to: 0; duration: 1000; running: false }
            NumberAnimation { id: rotRight; target: startImg; property: "rotation"; from: 0; to: 360; duration: 1000; running: false }

        } // !AppButton
    }
}
