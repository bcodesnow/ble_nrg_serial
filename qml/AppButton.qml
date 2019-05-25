import QtQuick 2.5
import "."

Rectangle {
    id: button
    color: baseColor
    onEnabledChanged: checkColor()
    radius: AppConstants.buttonRadius

    property color baseColor: AppConstants.buttonColor
    property color pressedColor: AppConstants.buttonPressedColor
    property color disabledColor: AppConstants.disabledButtonColor

    signal clicked()

    function checkColor()
    {
        if (!button.enabled) {
            button.color = disabledColor
        } else {
            if (mouseArea.containsPress)
                button.color = pressedColor
            else
                button.color = baseColor
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onPressed: checkColor()
        onReleased: checkColor()
        onClicked: {
            checkColor()
            button.clicked()
        }
    }
}
