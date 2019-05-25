/***************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtBluetooth module of the Qt Toolkit.
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

import QtQuick 2.5
import Shared 1.0

AppPage {

    errorMessage: deviceFinder.error
    infoMessage: deviceFinder.info

    Rectangle {
        id: viewContainer
        anchors.top: parent.top
        anchors.bottom: connectButton.top
        // only BlueZ platform has address type selection
        // connectionHandler.requiresAddressType ? addressTypeButton.top : searchButton.top
        anchors.topMargin: AppConstants.fieldMargin + messageHeight
        anchors.bottomMargin: AppConstants.fieldMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - AppConstants.fieldMargin*2
        color: AppConstants.viewColor
        radius: AppConstants.buttonRadius


        Text {
            id: title
            width: parent.width
            height: AppConstants.fieldHeight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: AppConstants.textColor
            font.pixelSize: AppConstants.mediumFontSize
            text: qsTr("FOUND DEVICES")

            BottomLine {
                height: 1;
                width: parent.width
                color: "#898989"
            }
        }


        ListView {
            id: devices
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: title.bottom
            model: deviceFinder.devices
            clip: true
            spacing: 3


            delegate: Rectangle {
                id: box
                height:AppConstants.fieldHeight * 1.2
                width: parent.width
                color: {
                    if (modelData.deviceFlags & 0x01)
                    {
                        AppConstants.infoColor
                    }
                    else
                    {
                        index % 2 === 0 ? AppConstants.delegate1Color : AppConstants.delegate2Color
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (modelData.deviceFlags & 0x01)
                        {
                            deviceFinder.removeDeviceFromSelection(index);
                            console.log("Device deSelected");
                        }
                        else
                        {
                            deviceFinder.addDeviceToSelection(index);
                            console.log("Device Selected");
                        }
                        //deviceFinder.connectToService(modelData.deviceAddress);
                    }
                }

                Text {
                    id: device
                    font.pixelSize: AppConstants.smallFontSize
                    text: modelData.deviceName
                    anchors.top: parent.top
                    anchors.topMargin: parent.height * 0.1
                    anchors.leftMargin: parent.height * 0.1
                    anchors.left: parent.left
                    color: AppConstants.textColor
                }

                Text {
                    id: deviceAddress
                    font.pixelSize: AppConstants.smallFontSize
                    text: modelData.deviceAddress
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: parent.height * 0.1
                    anchors.rightMargin: parent.height * 0.1
                    anchors.right: parent.right
                    color: Qt.darker(AppConstants.textColor)
                }
            }
        }
    }

        AppButton {
            id: connectButton
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: searchButton.top
            anchors.bottomMargin: AppConstants.fieldMargin*0.5
            width: viewContainer.width
            height: AppConstants.fieldHeight
            visible: true //connectionHandler.requiresAddressType // only required on BlueZ
            state: "disconnected"
            onClicked:
            {
                //state == "disconnected" ? state = "disconnected" : state = "connected"
                if (state === "disconnected")
                {
                    state = "connected"
                    deviceFinder.connectToMultipleServices();
                    app.showPage("Terminal.qml")
                }
                else
                {
                    state = "disconnected"
                    // handle termination gracefully
                }
            }
            states: [
                State {
                    name: "disconnected"
                    PropertyChanges { target: addressTypeText; text: qsTr("CONNECT") }
                    //PropertyChanges { target: deviceHandler; addressType: AddressType.PublicAddress }
                },
                State {
                    name: "connected"
                    PropertyChanges { target: addressTypeText; text: qsTr("TERMINATE") }
                    //PropertyChanges { target: deviceHandler; addressType: AddressType.RandomAddress }
                }
            ]

            Text {
                id: addressTypeText
                anchors.centerIn: parent
                font.pixelSize: AppConstants.tinyFontSize
                color: AppConstants.textColor
            }
        }

    AppButton {
        id: searchButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: AppConstants.fieldMargin
        width: viewContainer.width
        height: AppConstants.fieldHeight
        enabled: !deviceFinder.scanning
        onClicked: deviceFinder.startSearch()

        Text {
            anchors.centerIn: parent
            font.pixelSize: AppConstants.tinyFontSize
            text: qsTr("START SEARCH")
            color: searchButton.enabled ? AppConstants.textColor : AppConstants.disabledTextColor
        }
    }
}
