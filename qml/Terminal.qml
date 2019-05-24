/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [Imports]
import QtQuick.Window 2.3

//! [Imports]
import QtQuick 2.9
import QtQuick.Controls 2.2
import "."

// Orientation 1 Portrait, Orientation 2 Landscape

//ApplicationWindow {
//    id: rootWindow
//    visible: true
//    width: Screen.width //640
//    height: Screen.height //480
//    title: qsTr("BLE UART Terminal")
Item
{

    Switch {
        anchors.top: ( Screen.orientation === Qt.PortraitOrientation  ) ? terminalBackground.bottom : parent.top
        anchors.left: ( Screen.orientation === Qt.PortraitOrientation  ) ? parent.left : terminalBackground.right
        anchors.margins: 5
        height: 32
        width: 64
        text: "AutoScroll OFF"
        onCheckedChanged:
        {
            terminalListView.autoScrollEnabled = !terminalListView.autoScrollEnabled
        }
    }

    Connections {
        id: connectTerminalMsgSignal
        target: terminalToQml
        onMessageArrived:
        {
            terminalModel.append({ "str": str, "clr": clr, "fmt": fmt});
        }
    }

    ListModel {
        id: terminalModel
        Component.onCompleted:
        {
            terminalToQml.isActive = true;
        }
    }

    Rectangle
    {
        id: terminalBackground
        property int marginsLeftRight: 32
        anchors.left: parent.left
        anchors.leftMargin: marginsLeftRight
        anchors.top: parent.top
        anchors.topMargin: marginsLeftRight
        width: ( Screen.orientation === Qt.PortraitOrientation  ) ? ( parent.width - marginsLeftRight*2 ) : ( ( parent.width / 2 ) - marginsLeftRight*2 )
        height: ( Screen.orientation === Qt.PortraitOrientation  ) ? ( parent.height / 2 ) * 0.95 :  parent.height * 0.95
        radius: parent.height / 32
        color: "gray"
        gradient: Gradient {
            GradientStop { position: 0.0; color: "gray" }
            GradientStop { position: 1.0; color: "black" }
        }


        Rectangle
        {
            width: parent.width * 0.90
            height: parent.height * 0.90
            color: "gray"
            anchors.centerIn: parent

            ListView {
                id: terminalListView
                property bool autoScrollEnabled: true
                flickableDirection: Flickable.VerticalFlick
                flickDeceleration: 50
                boundsBehavior: Flickable.StopAtBounds
                width: parent.width
                height: parent.height * 0.7
                anchors.left: parent.left
                anchors.top: parent.top
                spacing: parent.height / 64
                cacheBuffer: 200
                model: terminalModel
                delegate:
                    Text {
                    color: clr
                    width: parent.width
                    font.pixelSize: 10
                    text: str
                    font.bold: fmt ? true : false
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }
                onCountChanged: {
                    if (autoScrollEnabled)
                        terminalListView.currentIndex = count - 1
                }

                ScrollBar.vertical: ScrollBar {}
            }

            Rectangle
            {
                id: terminalInputBlock
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                color: "whitesmoke"
                height: parent.height * 0.2
                width: parent.width
                radius: height / 16

                TextEdit
                {
                    id: txtInput
                    anchors.top: parent.top
                    anchors.left: parent.left
                    width: parent.width * 0.75
                    height: parent.height
                }
                RoundButton
                {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    width: parent.width * 0.2
                    height: parent.height
                    radius: terminalInputBlock.radius
                    text: "Send"
                    onClicked:
                    {
                        terminalToQml.messageFromQml(txtInput.text, "yellow", 1);
                        txtInput.clear();
                    }
                }
            }
        }
    }
}
