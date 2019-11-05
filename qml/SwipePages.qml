import QtQuick 2.9
import QtQuick.Controls 2.2
import "."

Item {
    id: swipePages
    anchors.fill: parent
    opacity: 0.0

    Behavior on opacity { NumberAnimation { duration: 500 } }

    function init()
    {
        opacity = 1.0
        view.currentIndex = 0;
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

    // global popups
    MultiPopup {
        // todo: fill list with adapters from c++ (only showing, no interaction)
        id: btAdapterPopup
        popupType: 3
    }
    MultiPopup {
        id: sessionSettingsPopup
        popupType: 4
    }
    MultiPopup {
        id: showTheDevilPopup
        popupType: 666
        Connections {
            target: catchController
            onInvokeQmlError: {
                showTheDevilPopup.visible = true
                console.log("Woops...")
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
            Terminal {
                id: terminalToQmlPage
            }
        }
    }

    TitleBar {
        id: titleBar
        visible: connectionHandler.alive
        currentIndex: view.currentIndex
        onTitleClicked: view.setCurrentIndex(index);

    }
}

