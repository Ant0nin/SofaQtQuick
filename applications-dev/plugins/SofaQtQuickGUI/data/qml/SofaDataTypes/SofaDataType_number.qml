/*
Copyright 2015, Anatoscope

This file is part of sofaqtquick.

sofaqtquick is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sofaqtquick is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with sofaqtquick. If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import QtQuick.Controls 1.3

TextField {
    id: root


    property var dataObject: null

    enabled: !dataObject.readOnly
    property int decimals: dataObject.properties["decimals"]

    horizontalAlignment : TextInput.AlignRight

    Component.onCompleted: download();

    onActiveFocusChanged: function(){
        if(activeFocus){
            horizontalAlignment = TextInput.AlignLeft
            root.text = Number(dataObject.value).toFixed(10) ;
            cursorPosition = 0
            selectAll()
        }else{
            horizontalAlignment = TextInput.AlignRight
            upload()
            download()
            cursorPosition = 10
        }
    }

    onCursorPositionChanged: function(){
    }

    onAccepted: function(){
        root.focus=false
    }

    /*
    MouseArea
    {
        anchors.fill: parent

        propagateComposedEvents: true

        onClicked: mouse.accepted = false;
        onPressed: mouse.accepted = false;
        onReleased: mouse.accepted = false;
        onDoubleClicked: mouse.accepted = false;
        onPositionChanged: mouse.accepted = false;
        onPressAndHold: mouse.accepted = false;
    }*/

    property var intValidator: IntValidator {
        Component.onCompleted: {
            var min = dataObject.properties["min"];
            if(undefined !== min)
                bottom = min;
        }
    }

    property var doubleValidator: DoubleValidator {
        decimals: root.decimals
    }


    validator: decimals > 0 ? doubleValidator : intValidator

    Connections {
        target: dataObject
        onUpdated: root.download();
    }

    /***************************************************************
     The formatting rules is inspired by Blender.
     - the dot '.' is at a fixed position so that row of numbers are aligned
     - there is 5 decimal number at right
     - there is at least 4 decimal number at right
    ***************************************************************/
    function download() {
        root.text = Number(dataObject.value).toFixed(5) ;
        cursorPosition = 0;
    }

    function upload() {
        //var oldValue = Number(dataObject.value).toFixed(decimals);
        //var newValue = root.text
        //if(oldValue !== newValue)
        dataObject.value = root.text;
    }
}
