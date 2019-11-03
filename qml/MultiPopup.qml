import QtQuick 2.12
import QtQuick.Controls 2.12 //as QQC2
//import QtQuick.Controls 1.4 as QQC1
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4

Popup {
    id: multiPopup
    anchors.centerIn: parent
    closePolicy: Popup.NoAutoClose
    focus: visible
    dim: modal
    padding: 0.0
    clip: true
    width:
        switch(popupType) {
        case type_SATAN:
            parent.width
            break;
        default:
            parent.width - 30
        }
    height:
        switch(popupType) {
        case type_progress:
            parent.height / 2
            break;
        case type_catch:
            parent.height / 2
            break;
        case type_adapter:
            parent.height * 3/5
            break;
        case type_session:
            parent.height * 4/5
            break;
        case type_SATAN:
            parent.height
            break;
        }

    property int popupType: 0
    property string maintitle: "Main Title"
    property string subtitle: "Sub Title"
    property int currentProgress: 100
    property bool indeterminate: false
    property double maxOpacity: 0.90

    readonly property int type_progress: 1
    readonly property int type_catch: 2
    readonly property int type_adapter: 3
    readonly property int type_session: 4
    readonly property int type_SATAN: 666

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: maxOpacity; duration: 500}
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: maxOpacity; to: 0.0; duration: 500}
    }
    Loader {
        anchors.fill: parent
        sourceComponent:
            switch(popupType) {
            case type_progress:
                progressPopup
                break;
            case type_catch:
                catchPopup
                break;
            case type_adapter:
                adapterPopup
                break;
            case type_session:
                sessionPopup
                break;
            case type_SATAN:
                satanPopup
                break;
            }
    }

    Component {
        id: progressPopup
        Rectangle {
            color: Qt.lighter( AppConstants.backgroundColor )
            opacity: maxOpacity
            radius: AppConstants.buttonRadius*2
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
                        height: 30
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
                            Rectangle {
                                id: bar
                                width: progressBar.visualPosition * parent.width
                                height: parent.height
                                radius: progressBar.rad
                                color: AppConstants.infoColor
                            }
                            // fancy stuff
                            LinearGradient {
                                anchors.fill: bar
                                start: Qt.point(0, 0)
                                end: Qt.point(bar.width, 0)
                                source: bar
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: "#17a81a" }
                                    GradientStop { id: grad; position: 0.5; color: Qt.lighter("#17a81a", 2) }
                                    GradientStop { position: 1.0; color: "#17a81a" }
                                }
                            }
                            LinearGradient {
                                anchors.fill: bar
                                start: Qt.point(0, 0)
                                end: Qt.point(0, bar.height)
                                source: bar
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: Qt.rgba(0,0,0,0) }
                                    GradientStop { position: 0.5; color: Qt.rgba(1,1,1,0.3) }
                                    GradientStop { position: 1.0; color: Qt.rgba(0,0,0,0.05) }
                                }
                            }
                            SequentialAnimation  {
                                PropertyAnimation { target: grad; property: "position"; from: 0.2; to: 0; duration: 2000 }
                                PropertyAnimation { target: grad; property: "position"; from: 0; to: 0.2; duration: 2000 }
                                PropertyAnimation { target: grad; property: "position"; from: 0.2; to: 0.8; duration: 3000 }
                                PropertyAnimation { target: grad; property: "position"; from: 0.8; to: 1; duration: 2000 }
                                PropertyAnimation { target: grad; property: "position"; from: 1; to: 0.8; duration: 2000 }
                                PropertyAnimation { target: grad; property: "position"; from: 0.8; to: 0.2; duration: 3000 }
                                running: indeterminate
                                loops: Animation.Infinite
                            }
                        }
                    } //!ProgressBar
                } // !Rectangle
            } // !ColumnLayout
        } // !Rectangle
    } // !Component

    Component {
        id: catchPopup
        Rectangle {
            color: Qt.lighter( AppConstants.backgroundColor, 1.2 )//Qt.darker("white")
            opacity: maxOpacity
            radius: AppConstants.buttonRadius*2
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
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
                    //   enabled: parent.catchConfirmationNeeded
                    pressedColor: AppConstants.infoColor
                    border.color: AppConstants.infoColor
                    opacity: 0.9
                    onClicked: {
                        console.log("Catch: downloading")
                        catchController.startDownloadFromAllDevices()
                        fileHandler.sendCatchSuccessFromQML(true)
                        multiPopup.visible = false
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
                        console.log("Drop: downloading")
                        catchController.startDownloadFromAllDevices()
                        fileHandler.sendCatchSuccessFromQML(false)
                        multiPopup.visible = false
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
                        console.log("Flop: sending stop")
                        catchController.sendStopToAllDevices() // is this right?
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
            onVisibleChanged: {
                if (visible && !timesync)
                {
                    if (catchController.devicesConnected) // prevent crash
                    {
                        console.log("Starting time sync")
                        catchController.startTimesyncAllDevices()
                    }
                    else console.log("Time sync failed")
                }
            }

            Connections {
                target: catchController
                onTimeSyncOfAllDevFinished:
                {
                    console.log("Time sync success")
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
                            validator: RegExpValidator { regExp: /[a-zA-Z,]{1,12}/ }
                            focus: true
                            leftPadding: 5
                            rightPadding: 5
                        }
                    }
                } // !RowLayout
                RowLayout {
                    id: catchModeSetting
                    Text {
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.preferredWidth: multiPopup.width/2
                        text: "Catch mode:"
                        font.pixelSize: AppConstants.smallFontSize
                        color: AppConstants.textColor
                        verticalAlignment: TextEdit.AlignVCenter
                        horizontalAlignment: TextEdit.AlignRight
                        rightPadding: sessionPopupRoot.textPadding
                    }
                    ComboBox {
                        id: catchModeCB
                        Layout.preferredHeight: sessionPopupRoot.rowHeight
                        Layout.preferredWidth: multiPopup.width * 2/5
                        Layout.alignment: Qt.AlignLeft
                        font.pixelSize: AppConstants.smallTinyFontSize
                        model: ["Standing","Running","Jumping","One hand"]
                        delegate: ItemDelegate {
                            contentItem: Text {
                                text: modelData
                                color: hovered?AppConstants.infoColor:AppConstants.textColor
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignLeft
                                font.pixelSize: AppConstants.smallTinyFontSize
                                rightPadding: catchModeCB.width
                            }
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
                        onCheckedChanged: {
                            if (checked && btSwitch.checked)
                                btSwitch.checked = false
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
                        onCheckedChanged: {
                            if (checked && sdSwitch.checked)
                                sdSwitch.checked = false
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
                            // todo:
                            if (checked && networkManager.authorized === 1)
                            {
                                // authorize to google and enable auto data upload
                                networkManager.synchronizeData();
                                console.log("Enabled google")
                            }
                            else if (!checked && networkManager.authorized === 2)
                            {
                                // disable auto data upload
                                // maybe unauthorize but not neccessary
                                console.log("Disabled google")
                            }
                            else if (checked && networkManager.authorized === 3)
                            {
                                // reconnect after error
                                console.log("Enabled google after error")
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
                        console.log("Start a lot of shit ")
                        // 1. create folder name by usernameInput.text

                        // 2. add catch mode to log file by catchModeCB.currentText

                        // 3. enable sd

                        // 4. enable bluetooth data transfer

                        // 5. can only be accepted when time sync success

                        // 6. can only be accepted when google is off, or is authorized to google

                    }
                    property bool buttonEnabled:
                        usernameInput.acceptableInput && sessionPopupRoot.timesync &&
                        (sdSwitch.checked || btSwitch.checked) &&
                        (googleSwitch.enabled && networkManager.authorized === 2 || !googleSwitch.enabled)
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
        id: satanPopup
        Rectangle {
            id: bg
            color: "black"
            opacity: 1
            radius: AppConstants.buttonRadius*2

            Image {
                id: satanBG
                source: "images/flames_big.png"
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                property double heightRate: this.height/parent.height
                onHeightRateChanged: {
                    this.height = heightRate*parent.height
                }
            }

            Image {
                id: satanBG2
                source: "images/flames_small.png"
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                property double heightRate: this.height/parent.height
                onHeightRateChanged: {
                    this.height = heightRate*parent.height
                }
            }

            Image {
                id: satanDevil
                source: "images/devil_full.png"
                fillMode: Image.PreserveAspectFit
                sourceSize.width: 300
                property int maxX: parent.width - satanDevil.width
                property int maxY: parent.height - satanDevil.height
                property double xRate: this.x/maxX
                property double yRate: this.y/maxY
                onXRateChanged: {
                    this.x = xRate*maxX
                }
                onYRateChanged: {
                    this.y = yRate*maxY
                }
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

                onYChanged: {
                    if (parent.width > this.width && parent.height > this.height)
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

                onXChanged: {
                    if (parent.width > this.width && parent.height > this.height)
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
                onBorderHit: {
                    if (!headMovementR.running)
                        headMovementR.start()
                    randomNumX = Math.random()*parent.width
                    randomNumY = Math.random()*parent.height
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        popupAccepted()
                    }
                }
            }

            // Animations
            SequentialAnimation  {
                id: headMovementY
                NumberAnimation { target: satanHead; property: "y";
                    from: satanHead.currentPos.y; to: satanHead.targetPos.y;
                    duration: satanAnimations.step_duration/2 }
                loops: Animation.Infinite
                running: true
            }
            SequentialAnimation  {
                id: headMovementX
                NumberAnimation { target: satanHead; property: "x";
                    from: satanHead.currentPos.x; to: satanHead.targetPos.x;
                    duration: satanAnimations.step_duration/2 }
                loops: Animation.Infinite
                running: true
            }
            SequentialAnimation  {
                id: headMovementR
                NumberAnimation { target: satanHead; property: "rotation"; from: 0; to: 360; duration: satanAnimations.step_duration*0.1 }
                running: false
            }

            ParallelAnimation {
                id: satanAnimations
                running: true
                property int step_duration: 10000
                loops: Animation.Infinite
                SequentialAnimation  {
                    NumberAnimation { target: satanBG; property: "heightRate";
                        from: 0.97; to: 1; duration: satanAnimations.step_duration }
                    NumberAnimation { target: satanBG; property: "heightRate";
                        from: 1; to: 0.97; duration: satanAnimations.step_duration }
                    loops: Animation.Infinite
                }
                SequentialAnimation  {
                    NumberAnimation { target: satanBG2; property: "heightRate";
                        from: 0.92; to: 1; duration: satanAnimations.step_duration*0.6 }
                    NumberAnimation { target: satanBG2; property: "heightRate";
                        from: 1; to: 0.92; duration: satanAnimations.step_duration*0.6 }
                    loops: Animation.Infinite
                }
                SequentialAnimation  {
                    id: xMovement
                    NumberAnimation { target: satanDevil; property: "xRate";
                        from: 0; to: 1; duration: satanAnimations.step_duration }
                    NumberAnimation { target: satanDevil; property: "xRate";
                        from: 1; to: 0; duration: satanAnimations.step_duration }
                    loops: Animation.Infinite
                }
                SequentialAnimation  {
                    id: yMovement
                    NumberAnimation { target: satanDevil; property: "yRate";
                        from: 1; to: 1; duration: satanAnimations.step_duration*0.4 }
                    NumberAnimation { target: satanDevil; property: "yRate";
                        from: 1; to: 0.9; duration: satanAnimations.step_duration*0.05 }
                    NumberAnimation { target: satanDevil; property: "yRate";
                        from: 0.9; to: 1; duration: satanAnimations.step_duration*0.05 }
                    NumberAnimation { target: satanDevil; property: "yRate";
                        from: 1; to: 0.9; duration: satanAnimations.step_duration*0.05 }
                    NumberAnimation { target: satanDevil; property: "yRate";
                        from: 0.9; to: 1; duration: satanAnimations.step_duration*0.05 }
                    NumberAnimation { target: satanDevil; property: "yRate";
                        from: 1; to: 1; duration: satanAnimations.step_duration*0.4 }
                    loops: Animation.Infinite
                }
                SequentialAnimation  {
                    id: rotationMovement
                    NumberAnimation { target: satanDevil; property: "rotation"; from: -10; to: 10; duration: satanAnimations.step_duration*0.2 }
                    NumberAnimation { target: satanDevil; property: "rotation"; from: 10; to: -10; duration: satanAnimations.step_duration*0.2 }
                    loops: Animation.Infinite
                }
            } // !ParallelAnimation
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

    // currently unused ...
    function insertNewLine(text, maxchars)
    {
        var output
        var index = maxchars - 1;
        var iterations = text.length / maxchars
        for (var i;i<iterations;i++)
        {
            output.concat( text.substr(index*i, index*(i+1)) + "\n" )
            //  text.substr(index*i, index*(i+1)) + "\n" + text.substr(maxchars*(i+1))
        }
        return output;
    }


    // Example connections
    //    Connections {
    //        target: deviceHandler_0
    //        onShowProgressMessage: {
    //            console.log("Popup 0:",mainText,subText,percent,flag)
    //            if (!visible) open()
    //            maintitle = mainText
    //            subtitle = subText
    //            currentProgress = percent
    //            popupType = flag
    //        }
    //        onProgressFinished: {
    //            close()
    //            currentProgress = 0
    //        }
    //    }
    //    Connections {
    //        target: deviceHandler_1
    //        onShowProgressMessage: {
    //            console.log("Popup 1:",mainText,subText,percent,flag)
    //            if (!visible) open()
    //            maintitle = mainText
    //            subtitle = subText
    //            progress = percent
    //        }
    //        onProgressFinished: {
    //            close()
    //            progress = 0
    //        }
    //    }



} // !Popup
