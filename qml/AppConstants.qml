pragma Singleton
import QtQuick 2.9

Item {
    property int wHeight
    property int wWidth

    property bool sessionPopupFinished


    // Colors
    readonly property color backgroundColor: "#2d3037"
    readonly property color buttonColor: "#202227"
    readonly property color buttonPressedColor: "#6ccaf2"
    readonly property color disabledButtonColor: "#555555"
    readonly property color viewColor: "#202227"
    readonly property color delegate1Color: Qt.darker(viewColor, 1.2)
    readonly property color delegate2Color: Qt.lighter(viewColor, 1.2)
    readonly property color textColor: "#ffffff"
    readonly property color textDarkColor: "#232323"
    readonly property color disabledTextColor: "#777777"
    readonly property color sliderColor: "#6ccaf2"
    readonly property color errorColor: "#ba3f62"
    readonly property color infoColor: "#3fba62"

    // Font sizes
    property real microFontSize: hugeFontSize * 0.2
    property real tinyFontSize: hugeFontSize * 0.4
    property real smallTinyFontSize: hugeFontSize * 0.5
    property real smallFontSize: hugeFontSize * 0.6
    property real mediumFontSize: hugeFontSize * 0.7
    property real bigFontSize: hugeFontSize * 0.8
    property real largeFontSize: hugeFontSize * 0.9
    property real hugeFontSize: (wHeight) * 0.05
    property real giganticFontSize: (wHeight) * 0.06

    // Some other values
    property real fieldHeight: wHeight * 0.08
    property real fieldMargin: fieldHeight * 0.5
    property real buttonHeight: wHeight * 0.08
    property real buttonRadius: buttonHeight * 0.1

    // Some helper functions
    function widthForHeight(h, ss)
    {
        return h/ss.height * ss.width;
    }

    function heightForWidth(w, ss)
    {
        return w/ss.width * ss.height;
    }

}
