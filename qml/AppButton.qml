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
    property color blinkingColor: AppConstants.infoColor
    property bool blinkActive: false

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

    Timer {
        id: timer
        interval: 500
        repeat: true
        onTriggered:
        {
            if ( button.color == blinkingColor)
                button.color = baseColor
            else
                button.color = blinkingColor
        }
    }

    function startBlinking()
    {
        timer.start();
        blinkActive = true;
    }

    function stopBlinking()
    {
        timer.stop();
        button.color = baseColor;
        blinkActive = false;
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onPressed: checkColor()
        onReleased: checkColor()
        onClicked: {
            checkColor()
            button.clicked()
            if (blinkActive)
                stopBlinking();
        }
    }
}
