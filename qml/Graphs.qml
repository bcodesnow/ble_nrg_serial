import QtQuick 2.9

import GraphPainterCpp 1.0

AppPage {
    id: graphPage
    GridView {
        id: graphGrid
        anchors.fill: parent
        anchors.topMargin: ySpace
        anchors.leftMargin: xSpace
        // 2x5 arrangement
        cellHeight: graphGrid.height/5
        cellWidth: graphGrid.width/2
        interactive: false
        property int ySpace: 7
        property int xSpace: 7

        onVisibleChanged: {
            // demo
            //            fileHandler.demo("LEFT",1)
            //            fileHandler.demo("RIGHT",1)
            //            fileHandler.demo("LEFT",2)
            //            fileHandler.demo("RIGHT",2)
            //            fileHandler.demo("LEFT",3)
            //            fileHandler.demo("RIGHT",3)
            //            fileHandler.demo("LEFT",4)
            //            fileHandler.demo("RIGHT",4)
            //            fileHandler.demo("LEFT",5)
            //            fileHandler.demo("RIGHT",5)
        }

        model: fileHandler.paintDataList

        delegate:
            Rectangle {
            id: delegateRect
            objectName: "graph"
            property int index: model.index
            property string paintName: modelData.paintName
            color: "transparent"
            height: graphGrid.cellHeight-graphGrid.ySpace
            width: graphGrid.cellWidth-graphGrid.xSpace

            Rectangle {
                id: labelRect
                objectName: "label"
                width: parent.width
                height: label.font.pixelSize+10
                color: AppConstants.buttonColor
                Text {
                    id: label
                    anchors.centerIn: parent
                    text: delegateRect.paintName
                    font.pixelSize: AppConstants.tinyFontSize
                    color: AppConstants.textColor
                }
            }
            Rectangle {
                id: drawRect
                objectName: "paintArea"
                anchors.top: labelRect.bottom
                anchors.topMargin: 2
                width: parent.width
                height: parent.height - labelRect.height
                radius: 10
                color: AppConstants.buttonColor
                border.color: Qt.lighter(AppConstants.backgroundColor)
                GraphPainterCpp {
                    id: painter
                    objectName: "painterItem"
                    anchors.fill: drawRect
                    Component.onCompleted: {
                        if (modelData.paintName !== undefined)
                        {
                            painter.painterName = modelData.paintName
                          //  console.log("New GraphPainter instance:",modelData.paintName,"Index:",delegateRect.index,"Item:",graphGrid.getDelegateInstanceAt(delegateRect.index))
                        }
                    }
                } //! GraphPainterCpp
            } // !Rectangle
        } // !delegate


        Text {
            anchors.centerIn: parent
            text: "No data available."
            visible: !(graphGrid.count > 0)
            font.pixelSize: AppConstants.smallFontSize
            color: AppConstants.textColor
        }

        Connections {
            target: fileHandler
            onNewPaintData: {
                var painterItem
                for (var i=0;i<graphGrid.count;i++)
                {
                    if (graphGrid.itemAtIndex(i).paintName === dataname)
                    {
                        painterItem = graphGrid.itemAtIndex(i).children[1].children[0]
                        painterItem.fillPaintData(dataptr,dataname)
                    }
                }
            }
        } //! Connections
    } // !GridView

//    AppButton {
//        id: driveSyncButton
//        anchors.left: parent.left
//        anchors.right: parent.right
//        anchors.bottom: parent.bottom
//        anchors.topMargin: 5
//        anchors.bottomMargin: 5
//        anchors.leftMargin: driveSyncButton.margin
//        anchors.rightMargin: driveSyncButton.margin
//        height: 50
//        width: parent.width
//        radius: 10
//        property int margin: 30
//        Text {
//            anchors.verticalCenter: parent.verticalCenter
//            anchors.left: parent.left
//            anchors.leftMargin: driveSyncButton.margin
//            font.pixelSize: AppConstants.tinyFontSize*1.1
//            text: "Synchronize Data to Google Drive"
//            color: AppConstants.textColor
//        }
//        Image {
//            id: authImage
//            source: if (networkManager.authorized === 1)
//                        "images/lock_toopen_w.png"
//                    else if (networkManager.authorized === 2)
//                        "images/lock_opened_w.png"
//                    else if (networkManager.authorized === 3)
//                        "images/lock_not_w.png"
//            anchors.verticalCenter: parent.verticalCenter
//            anchors.right: driveImage.left
//            anchors.rightMargin: driveSyncButton.margin/4
//            height: if (source === "images/lock_toopen_w.png")
//                        driveSyncButton.height*0.7
//                    else driveSyncButton.height*0.6
//            width: height
//        }
//        Image {
//            id: driveImage
//            source: "images/gdrive.png"
//            anchors.right: parent.right
//            anchors.rightMargin: driveSyncButton.margin/2
//            anchors.verticalCenter: parent.verticalCenter
//            height: driveSyncButton.height
//            width: driveSyncButton.height
//        }
//        MouseArea {
//            anchors.fill: parent
//            onClicked: {
//                // Authorize using .json file
//                networkManager.synchronizeData();
//            }
//        }
//    } // !AppButton
}
