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
            if (blinkActive)
            {
                if ( button.color == blinkingColor)
                    button.color = baseColor
                else
                    button.color = blinkingColor
            }
        }
    }

    function startSlowBlinking()
    {
        blinkActive = true;
        timer.stop();
        timer.interval = 750;
        timer.start();
    }

    function startFastBlinking()
    {
        blinkActive = true;
        timer.stop();
        timer.interval = 175;
        timer.start();
    }

    function stopBlinking()
    {
        blinkActive = false;
        timer.stop();
        button.color = baseColor;
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
