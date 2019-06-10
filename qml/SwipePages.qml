import QtQuick 2.0
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

