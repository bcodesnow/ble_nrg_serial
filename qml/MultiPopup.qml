import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls 2.12 as QQC
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4
import QtQuick.Dialogs 1.2
import "."

Popup {
    id: multiPopup
    anchors.centerIn: parent
    closePolicy: Popup.NoAutoClose
    // focus: visible
    modal: true
    dim: false
    padding: 0.0
    //  clip: true

    width:
        switch(currentPopupType) {
        case MultiPopupType.type_password:
            parent.width - AppConstants.fieldMargin*2
            break;
        case MultiPopupType.type_satan:
            parent.width
            break;
        default:
            parent.width - AppConstants.fieldMargin
        }
    height:
        switch(currentPopupType) {
        case MultiPopupType.type_progress:
            parent.height / 3
            break;
        case MultiPopupType.type_catch:
            parent.height * 3/5
            break;
        case MultiPopupType.type_adapter:
            parent.height * 3/5
            break;
        case MultiPopupType.type_session:
            parent.height * 4/5
            break;
        case MultiPopupType.type_password:
            parent.height / 3
            break;
        case MultiPopupType.type_satan:
            parent.height
            break;
        }

    property int currentPopupType: 0
    property string maintitle: "Main Title"
    property string subtitle: "Sub Title"
    property int currentProgress: 100
    property bool indeterminate: false
    property double maxOpacity: 0.90

    signal popupConfirmed(int arg1, int arg2)

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: maxOpacity; duration: 500}
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: maxOpacity; to: 0.0; duration: 500}
    }
    Loader {
        id: popupLoader
        anchors.fill: parent
        sourceComponent:
            switch(currentPopupType) {
            case MultiPopupType.type_progress:
                progressPopup
                break;
            case MultiPopupType.type_catch:
                catchPopup
                break;
            case MultiPopupType.type_adapter:
                adapterPopup
                break;
            case MultiPopupType.type_session:
                sessionPopup
                break;
            case MultiPopupType.type_password:
                if (QML_OS_LINUX) passwordPopup
                break;
            case MultiPopupType.type_satan:
                satanPopup
                break;
            }
    }

    Component {
        id: progressPopup
        Rectangle {
            id: progressPopupRoot
            anchors.fill: parent
            color: Qt.lighter( AppConstants.backgroundColor )
            opacity: maxOpacity
            radius: AppConstants.buttonRadius*2

            property double startTime
            property int secondsElapsed
            property bool animationsActive: false
            onVisibleChanged: {
                if (visible)
                {
                    restartCounter()
                    elapsedTimer.start()
                }
                else elapsedTimer.stop()
            }

            function restartCounter()  {
                startTime = 0;
            }
            function timeChanged()  {
                if(startTime===0)
                {
                    startTime = new Date().getTime();
                }
                var currentTime = new Date().getTime();
                secondsElapsed = (currentTime-startTime)/1000;
            }

            Timer  {
                id: elapsedTimer
                interval: 1000;
                running: false;
                repeat: true;
                triggeredOnStart: true
                onTriggered: progressPopupRoot.timeChanged()
            }

            Connections {
                target: catchController
                onDownloadOfAllDevFinished: {
                    multiPopup.visible = false
                }
            }

            ColumnLayout {
                id: progressColumn
                anchors.fill: parent
                TextEdit {
                    Layout.preferredHeight: AppConstants.smallFontSize*2
                    Layout.preferredWidth: parent.width
                    text: multiPopup.maintitle
                    font.pixelSize: AppConstants.smallFontSize
                    font.bold: true
                    color: AppConstants.textColor
                    horizontalAlignment: TextEdit.AlignHCenter
                    verticalAlignment: TextEdit.AlignVCenter
                    readOnly: true
                }
                TextEdit {
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: AppConstants.smallTinyFontSize*2
                    Layout.preferredWidth: parent.width
                    text: multiPopup.subtitle
                    font.pixelSize: AppConstants.smallTinyFontSize
                    color: AppConstants.textColor
                    horizontalAlignment: TextEdit.AlignHCenter
                    verticalAlignment: TextEdit.AlignVCenter
                    readOnly: true
                }
                TextEdit {
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: AppConstants.smallTinyFontSize*2
                    Layout.preferredWidth: parent.width
                    text: "Time elapsed: "+progressPopupRoot.secondsElapsed+" s"
                    font.pixelSize: AppConstants.smallTinyFontSize
                    color: AppConstants.textColor
                    horizontalAlignment: TextEdit.AlignHCenter
                    verticalAlignment: TextEdit.AlignVCenter
                    readOnly: true
                }
                Rectangle {
                    color: "transparent"
                    Layout.preferredHeight: parent.height/6
                    Layout.preferredWidth: parent.width
                    Layout.alignment: Qt.AlignCenter
                    ProgressBar {
                        id: progressBar
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width*(2/3)
                        height: parent.height
                        from: 0
                        to: 100
                        value: currentProgress
                        property int rad: 3

                        background: Rectangle {
                            color: "darkgray"
                            radius: parent.rad
                            border.color: "black"
                            border.width: 2
                            opacity: 0.2
                        }
                        contentItem: Item {
                            clip: true
                            Rectangle {
                                id: bar
                                width: progressBar.visualPosition * parent.width
                                height: parent.height
                                radius: progressBar.rad
                                color: AppConstants.infoColor
                            }

                            Rectangle {
                                id: animatedRect
                                width: bar.width/10
                                height: parent.height-2
                                radius: progressBar.rad
                                color: AppConstants.infoColor
                                anchors.verticalCenter: bar.verticalCenter
                                x: -width

                                LinearGradient {
                                    anchors.fill: animatedRect
                                    start: Qt.point(0, 0)
                                    end: Qt.point(animatedRect.width, 0)
                                    source: animatedRect
                                    gradient: Gradient {
                                        GradientStop { position: 0; color: AppConstants.infoColor }
                                        GradientStop { position: 0.5; color: AppConstants.textColor }
                                        GradientStop { position: 1; color: AppConstants.infoColor }
                                    }
                                }

                                SequentialAnimation  {
                                    id: progressAnimation
                                    running: progressPopupRoot.animationsActive
                                    loops: Animation.Infinite
                                    PropertyAnimation { target: animatedRect; property: "x"; from: -animatedRect.width; to: bar.width; duration: 5000 }
                                }


                            }
                        }
                    } //!ProgressBar
                } // !Rectangle
                Rectangle {
                    //placeholder
                    Layout.preferredHeight: parent.height/12
                } //!Rectangle
            } // !ColumnLayout
            Connections {
                target: popupLoader
                onWindowChanged: {
                    if (window)
                    {
                        console.log("debug set size from window")
                        progressPopupRoot.width = window.width
                        progressPopupRoot.height = window.height
                        if (indeterminate)
                            progressPopupRoot.animationsActive = true
                    }
                }
            }
        } // !Rectangle
    } // !Component

    Component {
        id: catchPopup
        Rectangle {
            id: catchPopupRoot
            color: Qt.lighter( AppConstants.backgroundColor )
            opacity: maxOpacity
            radius: AppConstants.buttonRadius*2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                TextEdit {
                    Layout.preferredHeight: AppConstants.smallFontSize*2
                    Layout.preferredWidth: parent.width
                    text: "How was it catched?"
                    font.pixelSize: AppConstants.smallFontSize
                    font.bold: true
                    color: AppConstants.textColor
                    horizontalAlignment: TextEdit.AlignHCenter
                    verticalAlignment: TextEdit.AlignVCenter
                    readOnly: true
                }
                //
                QQC.Tumbler {
                    id: catchModeTumbler
                    Layout.preferredHeight: multiPopup.height * 1/6
                    Layout.preferredWidth: multiPopup.width * 3/4
                    Layout.alignment: Qt.AlignCenter
                    visibleItemCount: 3
                    model: fileHandler.currentModeIndexCatch

                    delegate: Text {
                            text: modelData
                            opacity: 1.0 - Math.abs(Tumbler.displacement) / (Tumbler.tumbler.visibleItemCount / 2)
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: AppConstants.smallFontSize
                            color: AppConstants.textColor
                        }


                    Rectangle {
                        anchors.horizontalCenter: catchModeTumbler.horizontalCenter
                        y: catchModeTumbler.height * 0.35
                        width: 60
                        height: 1
                        color: AppConstants.infoColor
                    }

                    Rectangle {
                        anchors.horizontalCenter: catchModeTumbler.horizontalCenter
                        y: catchModeTumbler.height * 0.65
                        width: 60
                        height: 1
                        color: AppConstants.infoColor
                    }

                }
                //
                TextEdit {
                    Layout.preferredHeight: AppConstants.smallFontSize*2
                    Layout.preferredWidth: parent.width
                    text: "Catch or Drop?"
                    font.pixelSize: AppConstants.smallFontSize
                    font.bold: true
                    color: AppConstants.textColor
                    horizontalAlignment: TextEdit.AlignHCenter
                    verticalAlignment: TextEdit.AlignVCenter
                    readOnly: true
                }
                AppButton {
                    id: catchButton
                    Layout.preferredHeight: AppConstants.fieldHeight
                    Layout.preferredWidth: parent.width*(4/5)
                    Layout.alignment: Qt.AlignCenter
                    pressedColor: AppConstants.infoColor
                    border.color: AppConstants.infoColor
                    opacity: 0.9
                    onClicked: {
                        popupConfirmed(1, catchModeTumbler.currentIndex)
                    }
                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: AppConstants.mediumFontSize
                        font.bold: true
                        color: AppConstants.infoColor
                        text: qsTr("CATCH")
                    }
                }
                AppButton {
                    id: dropButton
                    Layout.preferredHeight: AppConstants.fieldHeight
                    Layout.preferredWidth: parent.width*(4/5)
                    Layout.alignment: Qt.AlignCenter
                    blinkingColor: AppConstants.errorColor
                    pressedColor: AppConstants.errorColor
                    border.color: AppConstants.errorColor
                    opacity: 0.9
                    onClicked:
                    {

                        popupConfirmed(2, catchModeTumbler.currentIndex)
                    }
                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: AppConstants.mediumFontSize
                        font.bold: true
                        text: qsTr("DROP")
                        color: dropButton.enabled ? AppConstants.errorColor : AppConstants.disabledTextColor
                    }
                }
                AppButton {
                    id: flopButton
                    Layout.preferredHeight: dropButton.height*2/3
                    Layout.preferredWidth: dropButton.width*2/3
                    Layout.alignment: Qt.AlignCenter
                    blinkingColor: AppConstants.textColor
                    pressedColor: AppConstants.textColor
                    border.color: AppConstants.textColor
                    opacity: 0.9
                    onClicked:
                    {
                        popupConfirmed(3, catchModeTumbler.currentIndex)
                        multiPopup.visible = false
                    }
                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: AppConstants.mediumFontSize
                        font.bold: true
                        text: qsTr("FLOP")
                        color: flopButton.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
                    }
                }
            } // !ColumnLayout
        } // !Rectangle
    } // !Component

    Component {
        id: adapterPopup
        // is this unused? -> yes
        Rectangle {
            color: Qt.lighter( AppConstants.backgroundColor )
            opacity: maxOpacity
            radius: AppConstants.buttonRadius*2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10
                TextEdit {
                    Layout.preferredHeight: AppConstants.smallFontSize
                    Layout.preferredWidth: parent.width
                    text: "Available Bluetooth-Adapters:"
                    font.pixelSize: AppConstants.smallFontSize
                    font.bold: true
                    color: AppConstants.textColor
                    horizontalAlignment: TextEdit.AlignHCenter
                    verticalAlignment: TextEdit.AlignVCenter
                    readOnly: true
                }
                Rectangle {
                    Layout.preferredHeight: parent.height*(3/4)
                    Layout.preferredWidth: parent.width
                    color: Qt.lighter( AppConstants.buttonColor )
                    radius: 15
                    ListView {
                        id: delegateMe
                        anchors.fill: parent
                        // Add model which shares the found adapters by QBluetoothLocalDevice
                        // model:
                        // delegate:
                    }
                }
                RowLayout {
                    Layout.alignment: Qt.AlignCenter
                    AppButton {
                        Layout.preferredHeight: AppConstants.fieldHeight
                        Layout.preferredWidth: multiPopup.width*(1/3)
                        pressedColor: AppConstants.errorColor
                        onClicked: {
                            popupRejected()
                        }
                        Text {
                            anchors.centerIn: parent
                            font.pixelSize: AppConstants.mediumFontSize
                            // font.bold: true
                            text: qsTr("Close")
                            color:  AppConstants.textColor
                        }
                    }
                } // !RowLayout
            } // !ColumnLayout
        } // !Rectangle
    } // !Component



    Component {
        id: sessionPopup
        Rectangle {
            id: sessionPopupRoot
            color: Qt.lighter( AppConstants.backgroundColor )
            opacity: maxOpacity
            radius: AppConstants.buttonRadius*2
            property bool timesync: false
            property int textPadding: parent.width/20
            property int rowHeight: AppConstants.smallFontSize*2

            Connections {
                target: catchController
                onTimeSyncOfAllDevFinished:
                {
                    timesync = success
                }
            }

            TextEdit {
                id: sessionTitle
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: 10
                text: "Session settings"
                font.pixelSize: AppConstants.mediumFontSize
                font.bold: true
                color: AppConstants.textColor
                horizontalAlignment: TextEdit.AlignHCenter
                verticalAlignment: TextEdit.AlignVCenter
                readOnly: true
            }
            ColumnLayout {
                anchors.top: sessionTitle.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.topMargin: AppConstants.fieldMargin/2
                RowLayout {
                    id: nameSetting
                    Text {
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.preferredWidth: multiPopup.width/2
                        text: "Username:"
                        font.pixelSize: AppConstants.smallFontSize
                        color: AppConstants.textColor
                        verticalAlignment: TextEdit.AlignVCenter
                        horizontalAlignment: TextEdit.AlignRight
                        rightPadding: sessionPopupRoot.textPadding
                    }
                    Rectangle {
                        id: usernameInputContainer
                        Layout.preferredHeight: AppConstants.bigFontSize
                        Layout.preferredWidth: multiPopup.width * 2/5
                        Layout.alignment: Qt.AlignLeft
                        color: AppConstants.backgroundColor
                        border.width: 1
                        border.color: "black"
                        radius: 5
                        TextInput {
                            id: usernameInput
                            font.pixelSize: AppConstants.smallFontSize
                            color: AppConstants.textColor
                            height: parent.height
                            width: parent.width
                            layer.enabled: true
                            validator: RegExpValidator { regExp: /[a-zA-Z,]{1,12}/ }
                            focus: true
                            leftPadding: 5
                            rightPadding: 5
                        }
                    }
                } // !RowLayout
                RowLayout {
                    id: sdCardSetting
                    Text {
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.preferredWidth: multiPopup.width/2
                        text: "SD enabled:"
                        font.pixelSize: AppConstants.smallFontSize
                        color: AppConstants.textColor
                        verticalAlignment: TextEdit.AlignVCenter
                        horizontalAlignment: TextEdit.AlignRight
                        rightPadding: sessionPopupRoot.textPadding
                        leftPadding: sessionPopupRoot.textPadding

                        Image {
                            id: sdImage
                            source: "images/baseline_sd_storage_white_18dp.png"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            width: height*1.1
                            height: AppConstants.mediumFontSize
                            anchors.leftMargin: parent.width-parent.implicitWidth-15
                        }
                    }
                    Switch {
                        id: sdSwitch
                        checked: false
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.alignment: Qt.AlignCenter
                        indicator:
                            Rectangle {
                            implicitHeight: AppConstants.smallTinyFontSize
                            implicitWidth: 60
                            anchors.left: parent.left
                            anchors.leftMargin: AppConstants.tinyFontSize/2
                            x: sdSwitch.leftPadding
                            y: parent.height / 2 - height / 2
                            radius: height/2
                            color: sdSwitch.checked ? AppConstants.infoColor : "gray"
                            border.color: sdSwitch.checked ? Qt.lighter(AppConstants.infoColor) : AppConstants.backgroundColor
                            Rectangle {
                                x: sdSwitch.checked ? maxX : 0
                                anchors.verticalCenter: parent.verticalCenter
                                height: parent.height + parent.height * 0.2
                                width: height
                                radius: height/2
                                color: sdSwitch.down ? "darkgray" : AppConstants.textColor
                                property int maxX: parent.width - width
                            }
                        }
                    }
                } // !RowLayout
                RowLayout {
                    id: bluetoothSetting
                    Text {
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.preferredWidth: multiPopup.width/2
                        text: "BT enabled:"
                        font.pixelSize: AppConstants.smallFontSize
                        color: AppConstants.textColor
                        verticalAlignment: TextEdit.AlignVCenter
                        horizontalAlignment: TextEdit.AlignRight
                        rightPadding: sessionPopupRoot.textPadding
                        leftPadding: sessionPopupRoot.textPadding

                        Image {
                            id: btImage
                            source: "images/bt.png"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            height: AppConstants.mediumFontSize
                            anchors.leftMargin: parent.width-parent.implicitWidth-width/2
                            fillMode: Image.PreserveAspectFit
                        }
                    }
                    Switch {
                        id: btSwitch
                        checked: true
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.alignment: Qt.AlignCenter
                        indicator:
                            Rectangle {
                            implicitHeight: AppConstants.smallTinyFontSize
                            implicitWidth: 60
                            anchors.left: parent.left
                            anchors.leftMargin: AppConstants.tinyFontSize/2
                            x: btSwitch.leftPadding
                            y: parent.height / 2 - height / 2
                            radius: height/2
                            color: btSwitch.checked ? AppConstants.infoColor : "gray"
                            border.color: btSwitch.checked ? Qt.lighter(AppConstants.infoColor) : AppConstants.backgroundColor
                            Rectangle {
                                x: btSwitch.checked ? maxX : 0
                                anchors.verticalCenter: parent.verticalCenter
                                height: parent.height + parent.height * 0.2
                                width: height
                                radius: height/2
                                color: btSwitch.down ? "darkgray" : AppConstants.textColor
                                property int maxX: parent.width - width
                            }
                        }
                    }
                } // !RowLayout
                RowLayout {
                    id: timeSyncIndicator

                    Text {
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.preferredWidth: multiPopup.width/2
                        text: "Synchronization:"
                        font.pixelSize: AppConstants.smallFontSize
                        color: AppConstants.textColor
                        verticalAlignment: TextEdit.AlignVCenter
                        horizontalAlignment: TextEdit.AlignRight
                        rightPadding: sessionPopupRoot.textPadding
                    }
                    Rectangle {
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.preferredWidth: multiPopup.width/2
                        color: "transparent"
                        StatusIndicator {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: width/2
                            color: "green"
                            active: sessionPopupRoot.timesync
                        }
                    }
                } // !RowLayout
                RowLayout {
                    id: gdriveSetting
                    Text {
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.preferredWidth: multiPopup.width/2
                        text: "Google Drive:"
                        font.pixelSize: AppConstants.smallFontSize
                        color: AppConstants.textColor
                        verticalAlignment: TextEdit.AlignVCenter
                        horizontalAlignment: TextEdit.AlignRight
                        rightPadding: sessionPopupRoot.textPadding
                        leftPadding: sessionPopupRoot.textPadding

                        Image {
                            id: driveImage
                            source: "images/gdrive.png"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            width: height*1.1
                            height: AppConstants.mediumFontSize
                            anchors.leftMargin: parent.width-parent.implicitWidth-15
                        }
                    }

                    Switch {
                        id: googleSwitch
                        checked: false
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.alignment: Qt.AlignCenter
                        indicator:
                            Rectangle {
                            implicitHeight: AppConstants.smallTinyFontSize
                            implicitWidth: 60
                            anchors.left: parent.left
                            anchors.leftMargin: AppConstants.tinyFontSize/2
                            x: googleSwitch.leftPadding
                            y: parent.height / 2 - height / 2
                            radius: height/2
                            color: googleSwitch.checked ? AppConstants.infoColor : "gray"
                            border.color: googleSwitch.checked ? Qt.lighter(AppConstants.infoColor) : AppConstants.backgroundColor
                            Rectangle {
                                x: googleSwitch.checked ? maxX : 0
                                anchors.verticalCenter: parent.verticalCenter
                                height: parent.height + parent.height * 0.2
                                width: height
                                radius: height/2
                                color: googleSwitch.down ? "darkgray" : AppConstants.textColor
                                property int maxX: parent.width - width
                            }
                        }
                        onCheckedChanged: {
                            if (checked && networkManager.authorized != 2)
                            {
                                networkManager.authorize();
                            }
                        }
                        Image {
                            id: authImage
                            anchors.left: parent.right
                            anchors.leftMargin: AppConstants.fieldMargin
                            anchors.verticalCenter: parent.verticalCenter
                            source: if (networkManager.authorized === 1)
                                        "images/lock_toopen_w.png"
                                    else if (networkManager.authorized === 2)
                                        "images/lock_opened_w.png"
                                    else if (networkManager.authorized === 3)
                                        "images/lock_not_w.png"
                            width: height
                            height: AppConstants.mediumFontSize
                        }
                    }
                } // !RowLayout
                AppButton {
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: AppConstants.fieldHeight
                    Layout.preferredWidth: multiPopup.width*(2/3)
                    pressedColor: AppConstants.infoColor
                    enabled: buttonEnabled
                    onClicked: {
                        catchController.setLoggingMedia(sdSwitch.checked,btSwitch.checked)

                        fileHandler.setCurrDir(usernameInput.text,googleSwitch.checked)
                        networkManager.enabled = googleSwitch.checked

                        AppConstants.sessionPopupFinished = true;
                        multiPopup.visible = false
                    }
                    property bool buttonEnabled:
                        (usernameInput.acceptableInput && sessionPopupRoot.timesync &&
                         (sdSwitch.checked || btSwitch.checked) &&
                         (googleSwitch.checked && (networkManager.authorized === 2) || !googleSwitch.checked))
                        || devMode

                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: AppConstants.mediumFontSize
                        text: qsTr("Apply")
                        color: parent.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
                    }
                }
            } // !ColumnLayout
        } // !Rectangle
    } // !Component

    Component {
        id: passwordPopup
        Rectangle {
            id: passwordPopupRoot
            color: Qt.lighter( AppConstants.backgroundColor )
            opacity: maxOpacity
            radius: AppConstants.buttonRadius*2
            Connections {
                target: linuxInterface
                Component.onCompleted: {
                    multiPopup.modal = true
                }
            }
            ColumnLayout {
                anchors.fill: parent
                Text {
                    Layout.preferredHeight: AppConstants.fieldHeight
                    Layout.preferredWidth: multiPopup.width
                    text: "Linux detected.\nEnter admin password:"
                    font.pixelSize: AppConstants.mediumFontSize
                    color: AppConstants.textColor
                    verticalAlignment: TextEdit.AlignVCenter
                    horizontalAlignment: TextEdit.AlignHCenter
                    lineHeight: 1.4
                }
                Rectangle {
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: AppConstants.fieldHeight*2/3
                    Layout.preferredWidth: multiPopup.width * 0.8
                    color: AppConstants.backgroundColor
                    border.width: 1
                    border.color: "black"
                    radius: 5
                    TextInput {
                        id: passwordInput
                        anchors.fill: parent
                        echoMode: TextInput.Password
                        font.pixelSize: AppConstants.giganticFontSize
                        color: AppConstants.textColor
                        height: parent.height
                        width: parent.width
                        layer.enabled: true
                        focus: true
                        leftPadding: 5
                        rightPadding: 5
                    }
                }
                AppButton {
                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredHeight: AppConstants.fieldHeight
                    Layout.preferredWidth: multiPopup.width/2
                    pressedColor: AppConstants.infoColor
                    enabled: passwordInput.text != "" || devMode
                    onClicked: {
                        linuxInterface.password = passwordInput.text
                        // linuxInterface.setPassword(passwordInput.text)
                        multiPopup.visible = false
                        linuxInterface.writeAllSudoCommands()
                    }
                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: AppConstants.mediumFontSize
                        text: qsTr("Apply")
                        color: parent.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
                    }
                }

            }
        }
    }

    Component {
        id: satanPopup
        Rectangle {
            id: satanPopupRoot
            color: "black"
            opacity: 1
            property bool animationsActive: false
            property int step_duration: 5000
            Component.onCompleted: {
                satanMessage.visible = true
            }

            Image {
                id: satanBG1
                source: "images/flames_big.png"
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
            }

            Image {
                id: satanBG2
                source: "images/flames_small.png"
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
            }

            Image {
                id: satanHead
                source: "images/devil.png"
                fillMode: Image.PreserveAspectFit
                x: parent.width/2
                y: parent.height/2
                sourceSize.width: 100

                property int maxX: parent.width - satanHead.width
                property int maxY: parent.height - satanHead.height
                property point currentPos: Qt.point(x, y)
                property point targetPos: Qt.point(0, 0)
                property double randomNumX: 0
                property double randomNumY: 0

                signal borderHit

                onXChanged: {
                    if (satanPopupRoot.animationsActive && parent.width > this.width && parent.height > this.height)
                    {
                        if (x <= 0)
                        {
                            headMovementY.running = false
                            headMovementX.running = false
                            targetPos.x = maxX
                            currentPos.x = this.x
                            targetPos.y = targetPos.y + randomNumX
                            currentPos.y = this.y
                            borderHit()
                            headMovementX.start()
                            headMovementY.start()
                        }
                        else if (x >= maxX)
                        {
                            headMovementY.running = false
                            headMovementX.running = false
                            targetPos.x = 0
                            currentPos.x = this.x
                            targetPos.y = targetPos.y + randomNumX
                            currentPos.y = this.y
                            borderHit()
                            headMovementX.start()
                            headMovementY.start()
                        }
                    }
                }

                onYChanged: {
                    if (animationsActive && parent.width > this.width && parent.height > this.height)
                    {
                        if (y <= 0)
                        {
                            headMovementY.running = false
                            headMovementX.running = false
                            targetPos.y = maxY
                            currentPos.y = this.y
                            targetPos.x = targetPos.x + randomNumY
                            currentPos.x = this.x
                            borderHit()
                            headMovementY.start()
                            headMovementX.start()
                        }
                        else if (y >= maxY)
                        {
                            headMovementY.running = false
                            headMovementX.running = false
                            targetPos.y = 0
                            currentPos.y = this.y
                            targetPos.x = targetPos.x + randomNumY
                            currentPos.x = this.x
                            borderHit()
                            headMovementY.start()
                            headMovementX.start()
                        }
                    }
                }

                onBorderHit: {
                    if (!headMovementR.running)
                        headMovementR.start()
                    randomNumX = Math.random()*parent.width
                    randomNumY = Math.random()*parent.height
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        multiPopup.visible = false;
                    }
                }
            }

            Image {
                id: satanDevil
                source: "images/devil_full.png"
                fillMode: Image.PreserveAspectFit
                sourceSize.width: 300
                y: maxY

                property int maxX: parent.width - satanDevil.width
                property int maxY: parent.height - satanDevil.height
                property int currentX: 0
                property int targetX: maxX
                property int currentY: maxY
                property int targetY: maxY
                property int hoppihoppiY: 50

                signal hoppihoppi()

                onXChanged: {
                    if (satanPopupRoot.animationsActive && parent.width > this.width && parent.height > this.height)
                    {
                        var intX = parseInt(x, 10)
                        if (intX >= maxX )
                        {
                            bodyMovementX.stop()
                            currentX = x-1
                            targetX = 0
                            bodyMovementX.start()
                        }
                        else if (intX <= 0)
                        {
                            bodyMovementX.stop()
                            currentX = x+1
                            targetX = maxX
                            bodyMovementX.start()
                        }
                        else if (!bodyMovementY.running && intX >= maxX/2 - 1 && x <= maxX/2 + 1 )
                        {
                            hoppihoppi()
                        }
                    }
                }

                onHoppihoppi: {
                    bodyMovementY.stop()
                    currentY = y
                    targetY = maxY-hoppihoppiY
                    bodyMovementY.start()
                }
            }

            // Animation - Background
            SequentialAnimation  {
                id: bgAnimation1
                running: satanPopupRoot.animationsActive
                NumberAnimation { target: satanBG1; property: "height";
                    from: satanPopupRoot.height*0.97; to: satanPopupRoot.height; duration: satanPopupRoot.step_duration }
                NumberAnimation { target: satanBG1; property: "height";
                    from: satanPopupRoot.height; to: satanPopupRoot.height*0.97; duration: satanPopupRoot.step_duration }
                loops: Animation.Infinite
            }
            SequentialAnimation  {
                id: bgAnimation2
                running: satanPopupRoot.animationsActive
                NumberAnimation { target: satanBG2; property: "height";
                    from: satanPopupRoot.height*0.92; to: satanPopupRoot.height; duration: satanPopupRoot.step_duration*0.6 }
                NumberAnimation { target: satanBG2; property: "height";
                    from: satanPopupRoot.height; to: satanPopupRoot.height*0.92; duration: satanPopupRoot.step_duration*0.6 }
                loops: Animation.Infinite
            }
            // Animation - Head
            SequentialAnimation  {
                id: headMovementX
                running: satanPopupRoot.animationsActive
                NumberAnimation { target: satanHead; property: "x";
                    from: satanHead.currentPos.x; to: satanHead.targetPos.x;
                    duration: satanPopupRoot.step_duration/2 }
                loops: Animation.Infinite
            }
            SequentialAnimation  {
                id: headMovementY
                running: satanPopupRoot.animationsActive
                NumberAnimation { target: satanHead; property: "y";
                    from: satanHead.currentPos.y; to: satanHead.targetPos.y;
                    duration: satanPopupRoot.step_duration/2 }
                loops: Animation.Infinite
            }
            SequentialAnimation  {
                id: headMovementR
                running: false
                NumberAnimation { target: satanHead; property: "rotation"; from: 0; to: 360;
                    duration: satanPopupRoot.step_duration*0.2 }
            }
            // Animation - Body
            SequentialAnimation  {
                id: bodyMovementX
                running: satanPopupRoot.animationsActive
                NumberAnimation { target: satanDevil; property: "x";
                    from: satanDevil.currentX; to: satanDevil.targetX; duration: satanPopupRoot.step_duration }
            }
            SequentialAnimation  {
                id: bodyMovementY
                running: false
                NumberAnimation { target: satanDevil; property: "y";
                    from: satanDevil.currentY; to: satanDevil.targetY; duration: satanPopupRoot.step_duration*0.1 }
                NumberAnimation { target: satanDevil; property: "y";
                    from: satanDevil.targetY; to: satanDevil.currentY; duration: satanPopupRoot.step_duration*0.1 }
                NumberAnimation { target: satanDevil; property: "y";
                    from: satanDevil.currentY; to: satanDevil.targetY; duration: satanPopupRoot.step_duration*0.1 }
                NumberAnimation { target: satanDevil; property: "y";
                    from: satanDevil.targetY; to: satanDevil.currentY; duration: satanPopupRoot.step_duration*0.1 }
            }
            SequentialAnimation  {
                id: bodyMovementR
                running: satanPopupRoot.animationsActive
                NumberAnimation { target: satanDevil; property: "rotation"; from: -10; to: 10; duration: satanPopupRoot.step_duration*0.2 }
                NumberAnimation { target: satanDevil; property: "rotation"; from: 10; to: -10; duration: satanPopupRoot.step_duration*0.2 }
                loops: Animation.Infinite
            }

            Rectangle {
                id: satanMessage
                color: AppConstants.backgroundColor
                border.color: "black"
                border.width: 1
                radius: AppConstants.buttonRadius
                opacity: 1
                visible: false
                anchors.centerIn: parent
                width: parent.width-AppConstants.fieldHeight
                height: AppConstants.fieldHeight * 3
                clip: true

                property int maxCharsPerLine: 30
                property string errorMessage:splitLines(maintitle)

                function splitLines(err_mesg) {
                    var result = ""
                    for (var i=0; i<err_mesg.length; i+=satanMessage.maxCharsPerLine)
                    {
                        result = result + err_mesg.slice(i, i+satanMessage.maxCharsPerLine) + "\n"
                    }
                    return result
                }

                TextEdit {
                    id: errorText
                    anchors.centerIn: parent
                    font.pixelSize: AppConstants.smallFontSize
                    color: AppConstants.textColor
                    text: parent.errorMessage
                }
            }

            Connections {
                target: popupLoader
                onWindowChanged: {
                    if (window)
                    {
                        satanPopupRoot.width = window.width
                        satanPopupRoot.height = window.height
                        satanPopupRoot.animationsActive = true
                    }
                }
            }

        } // !Rectangle
    } // !Component

    // black magic blur
    background:
        Item {
        ShaderEffectSource {
            id: effectSource
            sourceItem: multiPopup.parent
            anchors.fill: parent
            sourceRect: Qt.rect(multiPopup.x, multiPopup.y,
                                multiPopup.width, multiPopup.height)
        }
        FastBlur {
            anchors.fill: effectSource
            source: effectSource
            radius: 50
        }
    }

} // !Popup
