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
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import SofaBasics 1.0
import SofaApplication 1.0
import SofaInteractors 1.0
import SofaViewer 1.0
import SofaScene 1.0

SofaViewer {
    id: root

    clip: true
    backgroundColor: "#FF404040"
    backgroundImageSource: "qrc:/icon/sofaLogoAlpha.png"
    mirroredHorizontally: false
    mirroredVertically: false
    wireframe: false
    culling: true
    blending: false
    antialiasingSamples: 2
    sofaScene: SofaApplication.sofaScene
    property bool defaultCameraOrthographic: false

    property alias interactor: interactorLoader.item

    Component.onCompleted: {
        SofaApplication.addSofaViewer(root)

        recreateCamera();
    }

    Component.onDestruction: {
        SofaApplication.removeSofaViewer(root)
    }

	Action{
		shortcut: "F5"
		onTriggered: root.viewAll()
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        width: 100
        height: width
        running: sofaScene ? sofaScene.status === SofaScene.Loading : false;
    }

    Label {
        anchors.fill: parent
        visible: sofaScene ? sofaScene.status === SofaScene.Error : false
        color: "red"
        wrapMode: Text.WordWrap
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        text: sofaScene ? "Error during sofa scene loading\n" + sofaScene.source.toString().replace("///", "/").replace("file:", "") : "No sofa scene object"
    }

    Component {
        id: cameraComponent

        Camera {
        }
    }

    property bool keepCamera: false
    function recreateCamera() {
        if(camera && !keepCamera) {
            camera.destroy();
            camera = null;
        }

        if(!camera) {
            camera = cameraComponent.createObject(root, {orthographic: defaultCameraOrthographic} );

            viewAll();
        }
    }

    Connections {
        target: root.sofaScene
        onStatusChanged: {
            if(SofaScene.Ready === root.sofaScene.status)
                root.recreateCamera();
        }
    }

    function formatDateForScreenshot() {
        var today = new Date();
        var day = today.getDate();
        var month = today.getMonth();
        var year = today.getFullYear();

        var hour = today.getHours();
        var min = today.getMinutes() + hour * 60;
        var sec = today.getSeconds() + min * 60;
        var msec = today.getMilliseconds() + sec * 1000;

        return year + "-" + month + "-" + day + "_" + msec;
    }

    function takeScreenshot() {
        root.saveScreenshot("Captured/Screen/" + formatDateForScreenshot + ".png");
    }

    property bool saveVideo: false
    onSaveVideoChanged: {
        videoFrameNumber = 0;
        videoName = formatDateForScreenshot();
    }

    property int videoFrameNumber: 0
    property string videoName: "movie"
    Connections {
        target: root.sofaScene && root.saveVideo ? root.sofaScene : null
        onStepEnd: root.saveScreenshot("Captured/Movie/" + videoName + "/" + (root.videoFrameNumber++) + ".png");
    }

    Image {
        id: handIcon
        source: "qrc:/icon/hand.png"
        visible: sofaScene ? sofaScene.particleInteractor.interacting : false
        antialiasing: true

        Connections {
            target: sofaScene ? sofaScene.particleInteractor : null
            onInteractorPositionChanged: {
                var position = root.mapFromWorld(sofaScene.particleInteractor.interactorPosition)
                if(position.z > 0.0 && position.z < 1.0) {
                    handIcon.x = position.x - 6;
                    handIcon.y = position.y - 2;
                }
            }
        }
    }

    property Component interactorComponent: SofaApplication.interactorComponent

    property alias mouseArea: mouseArea
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        enabled: sofaScene && sofaScene.ready

        property alias interactor: interactorLoader.item
        Loader {
            id: interactorLoader
            sourceComponent: root.interactorComponent
//            source: "qrc:/SofaInteractors/UserInteractor_MoveCamera.qml"
            onLoaded: {
                var interactor = item;
                interactor.init();
            }
        }

        onClicked: {
            SofaApplication.setFocusedSofaViewer(root);

            if(interactor)
                interactor.mouseClicked(mouse, root);
        }

        onDoubleClicked: {
            SofaApplication.setFocusedSofaViewer(root);

            if(interactor)
                interactor.mouseDoubleClicked(mouse, root);
        }

        onPressed: {
            SofaApplication.setFocusedSofaViewer(root);

            if(interactor)
                interactor.mousePressed(mouse, root);
        }

        onReleased: {
            if(interactor)
                interactor.mouseReleased(mouse, root);
        }

        onWheel: {
            SofaApplication.setFocusedSofaViewer(root);

            if(interactor)
                interactor.mouseWheel(wheel, root);

            wheel.accepted = true;
        }

        onPositionChanged: {
            if(interactor)
                interactor.mouseMoved(mouse, root);
        }

        Keys.onPressed: {
            if(event.isAutoRepeat) {
                event.accepted = true;
                return;
            }

            if(sofaScene)
                sofaScene.keyPressed(event);

            if(interactor)
                interactor.keyPressed(event, root);

            event.accepted = true;
        }

        Keys.onReleased: {
            if(event.isAutoRepeat) {
                event.accepted = true;
                return;
            }

            if(sofaScene)
                sofaScene.keyReleased(event);

            if(interactor)
                interactor.keyReleased(event, root);

            event.accepted = true;
        }
    }

    readonly property alias crosshairGizmo: crosshairGizmo
    Item {
        id: crosshairGizmo
        anchors.centerIn: parent
        visible: false

        function show() {
            popAnimation.complete();
            visible = true;
        }

        function hide() {
            popAnimation.complete();
            visible = false;
        }

        function pop() {
            popAnimation.restart();
        }

        SequentialAnimation {
            id: popAnimation

            ScriptAction    {script: {crosshairGizmo.visible = true;}}
            NumberAnimation {target:  crosshairGizmo; properties: "opacity"; from: 1.0; to: 0.0; duration: 2000;}
            ScriptAction    {script: {crosshairGizmo.visible = false; crosshairGizmo.opacity = crosshairGizmo.defaultOpacity;}}
        }

        readonly property real defaultOpacity: 0.75
        opacity: defaultOpacity
        property color color: "red"
        property real size: Math.min(root.width, root.height) / 20.0
        property real thickness: 1

        Rectangle {
            anchors.centerIn: parent
            color: crosshairGizmo.color
            width: crosshairGizmo.size
            height: crosshairGizmo.thickness
        }

        Rectangle {
            anchors.centerIn: parent
            color: crosshairGizmo.color
            width: crosshairGizmo.thickness
            height: crosshairGizmo.size
        }
    }

    /*Item {
        id: circleGizmo
        anchors.centerIn: parent
        visible: false

        opacity: 0.75
        property color color: "red"
        property real size: Math.min(root.width, root.height) / 2.0
        property real thickness: 1

        Rectangle {
            anchors.centerIn: parent
            color: "transparent"
            border.color: circleGizmo.color
            border.width: circleGizmo.thickness
            width: circleGizmo.size
            height: width
            radius: width / 2.0
        }
    }*/

    Rectangle {
        id: toolPanel
        color: "lightgrey"
        anchors.top: toolPanelSwitch.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.topMargin: -6
        anchors.bottomMargin: 20
        anchors.rightMargin: -radius
        width: 250
        radius: 5
        visible: false
        opacity: 0.9

        // avoid mouse event propagation through the toolpanel to the sofa viewer
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons
            onWheel: wheel.accepted = true
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: toolPanel.radius / 2
            anchors.rightMargin: anchors.margins - toolPanel.anchors.rightMargin
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: "SofaViewer parameters"
                font.bold: true
                color: "darkblue"
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Flickable {
                    anchors.fill: parent
                    contentHeight: panelColumn.implicitHeight

                    Column {
                        id: panelColumn
                        anchors.fill: parent
                        spacing: 5

                        GroupBox {
                            id: visualPanel
                            implicitWidth: parent.width
                            title: "Visual"

                            GridLayout {
                                anchors.fill: parent
                                columnSpacing: 0
                                rowSpacing: 2
                                columns: 2

                                Label {
                                    Layout.fillWidth: true
                                    text: "Wireframe"
                                }

                                Switch {
                                    id: wireframeSwitch
                                    Layout.alignment: Qt.AlignCenter
                                    Component.onCompleted: checked = root.wireframe
                                    onCheckedChanged: root.wireframe = checked

                                    ToolTip {
                                        anchors.fill: parent
                                        description: "Draw in wireframe mode"
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: "Culling"
                                }

                                Switch {
                                    id: cullingSwitch
                                    Layout.alignment: Qt.AlignCenter
                                    Component.onCompleted: checked = root.culling
                                    onCheckedChanged: root.culling = checked

                                    ToolTip {
                                        anchors.fill: parent
                                        description: "Enable culling"
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: "Blending"
                                }

                                Switch {
                                    id: blendingSwitch
                                    Layout.alignment: Qt.AlignCenter
                                    Component.onCompleted: checked = root.blending
                                    onCheckedChanged: root.blending = checked

                                    ToolTip {
                                        anchors.fill: parent
                                        description: "Enable blending"
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: "Normals"
                                }

                                Switch {
                                    id: normalsSwitch
                                    Layout.alignment: Qt.AlignCenter
                                    Component.onCompleted: checked = root.blending
                                    onCheckedChanged: root.drawNormals = checked

                                    ToolTip {
                                        anchors.fill: parent
                                        description: "Display normals"
                                    }
                                }

                                RowLayout {
                                    id: normalsLayout
                                    Layout.alignment: Qt.AlignCenter
                                    Layout.columnSpan: 2
                                    visible: normalsSwitch.checked

                                    Slider {
                                        id: normalsSlider
                                        Layout.fillWidth: true

                                        Component.onCompleted: {
                                            value = Math.sqrt(root.normalsDrawLength);
                                            minimumValue = value * 0.1;
                                            maximumValue = value * 2.0;
                                            stepSize = minimumValue;
                                        }

                                        onValueChanged: {
                                            root.normalsDrawLength = value * value;
                                        }
                                    }

                                    TextField {
                                        Layout.preferredWidth: 32
                                        readOnly: true
                                        text: normalsSlider.value.toFixed(1);
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: "Antialiasing"
                                }

                                Switch {
                                    id: antialiasingSwitch
                                    Layout.alignment: Qt.AlignCenter
                                    Component.onCompleted: checked = (0 !== root.antialiasingSamples)
                                    
                                    onCheckedChanged: {
                                        if(checked) {
                                            antialiasingSlider.uploadValue(antialiasingSlider.minimumValue);
                                            antialiasingLayout.visible = true;
                                        } else {
                                            antialiasingLayout.visible = false;
                                            antialiasingSlider.uploadValue(0);
                                        }
                                    }

                                    ToolTip {
                                        anchors.fill: parent
                                        description: "Enable / Disable Antialiasing\n\nNote : You must resize your window before the changes will take effect"
                                    }
                                }

                                RowLayout {
                                    id: antialiasingLayout
                                    Layout.alignment: Qt.AlignCenter
                                    Layout.columnSpan: 2

                                    Slider {
                                        id: antialiasingSlider
                                        Layout.fillWidth: true
                                        Component.onCompleted: downloadValue();
                                        onValueChanged: if(visible) uploadValue(value);

                                        stepSize: 1
                                        minimumValue: 1
                                        maximumValue: 4

                                        function downloadValue() {
                                            value = Math.min((root.antialiasingSamples >= 1 ? Math.log(root.antialiasingSamples) / Math.log(2.0) : minimumValue), maximumValue);
                                        }

                                        function uploadValue(newValue) {
                                            if(undefined === newValue)
                                                newValue = value;

                                            root.antialiasingSamples = (newValue >= 1 ? Math.round(Math.pow(2.0, newValue)) : 0);
                                        }

                                        Connections {
                                            target: root
                                            onAntialiasingSamplesChanged: antialiasingSlider.downloadValue();
                                        }

                                        ToolTip {
                                            anchors.fill: parent
                                            description: "Change the number of samples used for antialiasing\n\nNote : You must resize your window before the changes will take effect"
                                        }
                                    }

                                    TextField {
                                        Layout.preferredWidth: 32
                                        readOnly: true
                                        text: root.antialiasingSamples;
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: "Background"
                                }

                                Rectangle {
                                    Layout.preferredWidth: wireframeSwitch.implicitWidth
                                    Layout.preferredHeight: wireframeSwitch.implicitHeight
                                    Layout.alignment: Qt.AlignCenter
                                    color: "darkgrey"
                                    radius: 2

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: backgroundColorPicker.open()
                                    }

                                    ColorDialog {
                                        id: backgroundColorPicker
                                        title: "Please choose a background color"
                                        showAlphaChannel: true

                                        property color previousColor
                                        Component.onCompleted: {
                                            previousColor = root.backgroundColor;
                                            color = previousColor;
                                        }

                                        onColorChanged: root.backgroundColor = color

                                        onAccepted: previousColor = color
                                        onRejected: color = previousColor
                                    }

                                    Rectangle {
                                        anchors.fill: parent
                                        anchors.margins: 2
                                        color: Qt.rgba(root.backgroundColor.r, root.backgroundColor.g, root.backgroundColor.b, 1.0)

                                        ToolTip {
                                            anchors.fill: parent
                                            description: "Background color"
                                        }
                                    }
                                }
                            }
                        }

                        GroupBox {
                            id: savePanel
                            implicitWidth: parent.width

                            title: "Save"

                            GridLayout {
                                anchors.fill: parent
                                columnSpacing: 0
                                rowSpacing: 2
                                columns: 2

                                Item {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: screenshotButton.implicitHeight

                                    Button {
                                        id: screenshotButton
                                        anchors.fill: parent
                                        text: "Screenshot"
                                        checked: false
                                        checkable: false

                                        onClicked: root.takeScreenshot();

                                        ToolTip {
                                            anchors.fill: parent
                                            description: "Save screenshot in Captured/Screen/"
                                        }
                                    }
                                }

                                Item {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: movieButton.implicitHeight

                                    Button {
                                        id: movieButton
                                        anchors.fill: parent
                                        text: "Movie"
                                        checked: false
                                        checkable: true

                                        onClicked: root.saveVideo = checked

                                        ToolTip {
                                            anchors.fill: parent
                                            description: "Save video in Captured/Movie/"
                                        }
                                    }
                                }
                            }
                        }

                        GroupBox {
                            id: cameraPanel
                            implicitWidth: parent.width

                            title: "Camera"

                            Column {
                                anchors.fill: parent
                                spacing: 0

                                GroupBox {
                                    implicitWidth: parent.width
                                    title: "Mode"
                                    flat: true

                                    RowLayout {
                                        anchors.fill: parent
                                        spacing: 0

                                        Button {
                                            id: orthoButton
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: parent.width

                                            text: "Orthographic"
                                            checkable: true
                                            checked: root.defaultCameraOrthographic
                                            onCheckedChanged: root.camera.orthographic = checked
                                            onClicked: {
                                                checked = true;
                                                perspectiveButton.checked = false;
                                            }

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Orthographic Mode"
                                            }
                                        }

                                        Button {
                                            id: perspectiveButton
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: parent.width

                                            text: "Perspective"
                                            checkable: true
                                            checked: !root.defaultCameraOrthographic
                                            onCheckedChanged: if(root.camera) root.camera.orthographic = !checked
                                            onClicked: {
                                                checked = true;
                                                orthoButton.checked = false;
                                            }

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Perspective Mode"
                                            }
                                        }
                                    }
                                }


                                //                                Label {
                                //                                    Layout.fillWidth: true
                                //                                    text: "Logo"
                                //                                }

                                //                                RowLayout {
                                //                                    Layout.fillWidth: true
                                //                                    spacing: 0

                                //                                    TextField {
                                //                                        id: logoTextField
                                //                                        Layout.fillWidth: true
                                //                                        Component.onCompleted: text = root.backgroundImageSource
                                //                                        onAccepted: root.backgroundImageSource = text
                                //                                    }

                                //                                    Button {
                                //                                        Layout.preferredWidth: 22
                                //                                        Layout.preferredHeight: Layout.preferredWidth
                                //                                        iconSource: "qrc:/icon/open.png"

                                //                                        onClicked: openLogoDialog.open()

                                //                                        FileDialog {
                                //                                            id: openLogoDialog
                                //                                            title: "Please choose a logo"
                                //                                            selectFolder: true
                                //                                            selectMultiple: false
                                //                                            selectExisting: true
                                //                                            property var resultTextField
                                //                                            onAccepted: {
                                //                                                logoTextField.text = Qt.resolvedUrl(fileUrl)
                                //                                                logoTextField.accepted();
                                //                                            }
                                //                                        }
                                //                                    }
                                //                                }

                                GroupBox {
                                    implicitWidth: parent.width
                                    title: "View"
                                    flat: true

                                    GridLayout {
                                        anchors.fill: parent
                                        columns: 2
                                        rowSpacing: 0
                                        columnSpacing: 0

                                        Button {
                                            Layout.fillWidth: true
                                            Layout.columnSpan: 2
                                            text: "Fit"

                                            onClicked: if(camera) camera.fit(root.boundingBoxMin(), root.boundingBoxMax())

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Fit in view"
                                            }
                                        }

                                        Button {
                                            Layout.fillWidth: true
                                            text: "-X"

                                            onClicked: if(camera) camera.viewFromLeft()

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Align view along the negative X Axis"
                                            }
                                        }

                                        Button {
                                            Layout.fillWidth: true
                                            text: "+X"

                                            onClicked: if(camera) camera.viewFromRight()

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Align view along the positive X Axis"
                                            }
                                        }

                                        Button {
                                            Layout.fillWidth: true
                                            text: "-Y"

                                            onClicked: if(camera) camera.viewFromTop()

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Align view along the negative Y Axis"
                                            }
                                        }

                                        Button {
                                            Layout.fillWidth: true
                                            text: "+Y"

                                            onClicked: if(camera) camera.viewFromBottom()

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Align view along the positive Y Axis"
                                            }
                                        }

                                        Button {
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: parent.width
                                            text: "-Z"

                                            onClicked: if(camera) camera.viewFromFront()

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Align view along the negative Z Axis"
                                            }
                                        }

                                        Button {
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: parent.width
                                            text: "+Z"

                                            onClicked: if(camera) camera.viewFromBack()

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Align view along the positive Z Axis"
                                            }
                                        }

                                        Button {
                                            Layout.fillWidth: true
                                            Layout.columnSpan: 2
                                            text: "Isometric"

                                            onClicked: if(camera) camera.viewIsometric()

                                            ToolTip {
                                                anchors.fill: parent
                                                description: "Isometric View"
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Image {
        id: toolPanelSwitch
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 26
        anchors.rightMargin: 3
        source: toolPanel.visible ? "qrc:/icon/minus.png" : "qrc:/icon/plus.png"
        width: 12
        height: width

        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true
            onClicked: toolPanel.visible = !toolPanel.visible
        }
    }
}
