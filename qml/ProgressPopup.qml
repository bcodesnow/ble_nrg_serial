import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

// Label


Popup {
    id: progressPopup
    anchors.centerIn: parent
    width: parent.width //- 30
    height: parent.height / 3
    closePolicy: Popup.NoAutoClose
    focus: visible
    modal: false
    dim: false

    background: Rectangle {
        id: bg
        color: "white"
        smooth: true

        //        ShaderEffectSource {
        //            id: effectSource

        //            sourceItem: bg
        //            anchors.centerIn: bg
        //            width: bg.width
        //            height: bg.height
        //            sourceRect: Qt.rect(x, y, width, height)
        //        }

        //        FastBlur {
        //                anchors.fill: effectSource
        //                source: effectSource
        //                radius: 100
        //            }
    }




    property string maintitle: "Balint is having a shit"
    property string subtitle: "Progress is as follows:"
    property int progress: 0
    property double maxOpacity: 0.97

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: maxOpacity }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: maxOpacity; to: 0.0 }
    }

    Rectangle {
        id: container
        anchors.fill: parent
        color: Qt.lighter( AppConstants.backgroundColor )//Qt.darker("white")
        opacity: maxOpacity
        radius: AppConstants.buttonRadius*2

        ShaderEffectSource {
            id: effectSource
            sourceItem: container
            anchors.centerIn: container
            width: container.width
            height: container.height
            sourceRect: Qt.rect(x, y, width, height)
        }

        FastBlur {
            anchors.fill: effectSource
            source: effectSource
            radius: 100
        }


        ColumnLayout {

            anchors.fill: parent
            Label {
                id: mainTitle
                text: maintitle
                width: parent.width
                height: parent.height/4

                color: AppConstants.textColor
            }
            Label {
                id: subTitle
                text: subtitle
                width: parent.width
                height: parent.height/4
                color: AppConstants.textColor
            }
            ProgressBar {
                id: progressBar
                width: parent.width
                height: parent.height/2
                from: 0
                to: 100
                value: progress
            }
        }
    }

    Connections {
        target: deviceHandler_0
        onShowProgressMessage: {
            console.log("Popup 0:",mainText,subText,percent,flag)
            if (!visible) open()
            maintitle = mainText
            subtitle = subText
            progress = percent
        }
        onProgressFinished: {
            close()
            progress = 0
        }
    }

    Connections {
        target: deviceHandler_1
        onShowProgressMessage: {
            console.log("Popup 1:",mainText,subText,percent,flag)
            if (!visible) open()
            maintitle = mainText
            subtitle = subText
            progress = percent
        }
        onProgressFinished: {
            close()
            progress = 0
        }
    }



    // background: parent // Needed?
}
