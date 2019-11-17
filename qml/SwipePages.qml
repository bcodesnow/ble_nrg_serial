import QtQuick 2.9
import QtQuick.Controls 2.2
import "."

Item {
    id: swipePages
    anchors.fill: parent
    opacity: 0.0

    property bool devMode: false

    Behavior on opacity { NumberAnimation { duration: 500 } }

    function init()
    {
        opacity = 1.0
        view.currentIndex = 0;
        // if (QML_OS_LINUX) passwordInputPopup.visible = true
    }

    Keys.onReleased: {
        switch (event.key) {
        case Qt.Key_Escape:
        case Qt.Key_Back:
            if (view.currentIndex > 0)
            {
                view.currentIndex = view.currentIndex-1;
                event.accepted = true
            }
            else
            {
                Qt.quit()
            }
            break;
        default: break;
        }
    }

    //    Component.onCompleted: {
    //        passwordInputPopup.visible = true
    //    }

    // global popups
    MultiPopup {
        id: passwordInputPopup
        currentPopupType: MultiPopupType.type_password
    }
    //    MultiPopup {
    //        // unused
    //        id: btAdapterPopup
    //        popupType: 3
    //    }
    MultiPopup {
        id: sessionSettingsPopup
        currentPopupType: MultiPopupType.type_session
    }
    MultiPopup {
        id: showTheDevilPopup
        currentPopupType: MultiPopupType.type_satan
        Component.onCompleted: {
            // todo: start popup animations at this point,
            //       not on loader windowChanged() signal
            // global property doesnt work for the bg-flames ...
        }

        Connections {
            target: catchController
            onInvokeQmlError: {
                showTheDevilPopup.maintitle = error
                showTheDevilPopup.visible = true
                console.log("Woops...",error)
            }
        }
    }

    BluetoothAlarmDialog {
        id: btAlarmDialog
        anchors.fill: parent
        visible: !connectionHandler.alive
    }

    SwipeView {
        id: view
        visible: connectionHandler.alive
        currentIndex: 0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom

        onCurrentIndexChanged: {
            if (currentIndex > 2 && !devMode)
                decrementCurrentIndex()
        }

        Item {
            id: connect
            Connect {
                id: connectPage
            }
        }
        Item {
            id: secondPage
            Catch {
                id: catchPage
                onDevicesConnectedChanged: {
                    if (devicesConnected)
                    {
                        sessionSettingsPopup.visible = true
                    }
                }
            }
        }
        Item {
            id: thirdPage
            Graphs {
                id: graphPage
            }
        }
        Item {
            id: terminalPage
            visible: devMode
            Terminal {
                id: terminalToQmlPage
            }
        }
    }

    TitleBar {
        id: titleBar
        visible: connectionHandler.alive
        currentIndex: view.currentIndex
        property string combination
        property string solution: "001122"
        onTitleClicked:
        {
            view.setCurrentIndex(index);

            combination += currentIndex
            if (combination.length > solution.length)
                combination = combination.substring(1)
            if (combination == solution && !devMode)
            {
                devMode = true
                console.log("Developer mode unlocked")
            }
        }

    }
}

