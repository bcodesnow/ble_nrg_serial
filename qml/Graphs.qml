import QtQuick 2.12
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
                radius: AppConstants.buttonRadius
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
                radius: AppConstants.buttonRadius
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

        Connections {
            target: fileHandler
            onPaintDataListChanged: {
                console.log("!! - !! -- onPaintDataListChanged")
                var painterItem
                for (var i=0;i<fileHandler.paintDataList.length;i++)
                {
                    painterItem = graphGrid.itemAtIndex(i).children[1].children[0]
                    painterItem.graphData = fileHandler.paintDataList[i];
                }
            }

//            onUpdateAllPainters: {
//                var painterItem
//                for (var i=0;i<datalist.length;i++)                {
//                    painterItem = graphGrid.itemAtIndex(i).children[1].children[0]
//                    painterItem.graphData = datalist[i]
//                }
//            }
        } //! Connections
    } // !GridView
}
