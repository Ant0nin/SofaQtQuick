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

pragma Singleton
import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import Qt.labs.settings 1.0
import Qt.labs.folderlistmodel 2.1
import SofaApplicationSingleton 1.0
import "qrc:/SofaCommon/SofaCommonScript.js" as SofaCommonScript

SofaApplication {
    id: root

////////////////////////////////////////////////// PRIVATE
    property QtObject d: QtObject {
        id: d

        property var sceneListModel: null

        property var focusedViewer: null
        property var viewers: []

    }

////////////////////////////////////////////////// SCENE

    property var scene: null

//    // create a sceneListModel only if needed
//    function sceneListModel() {
//        if(null === d.sceneListModel) {
//            console.log("+");
//            d.sceneListModel = SofaCommonScript.InstanciateComponent(SceneListModel, root, {"scene": root.scene});
//            console.log("-");
//        }

//        return d.sceneListModel;
//    }

////////////////////////////////////////////////// TOOLBAR

    property var toolBar: null

////////////////////////////////////////////////// STATUSBAR

    property var statusBar: null

////////////////////////////////////////////////// VIEWER

    readonly property alias focusedViewer: d.focusedViewer
    readonly property alias viewers: d.viewers

    function setFocusedViewer(viewer) {
        d.focusedViewer = viewer;
    }

    function addViewer(viewer) {
        d.viewers.push(viewer)
    }

    function removeViewer(viewer) {
        var index = d.viewers.indexOf(viewer);
        if(-1 === index)
            return;

        d.viewers.splice(index, 1);
    }

////////////////////////////////////////////////// SETTINGS

    //property UISettings uiSettings: UISettings {}
    //property RecentSettings uiSettings: RecentSettings {}

////////////////////////////////////////////////// INTERACTOR

    property string defaultInteractorName: "ParticleInteractor"
    readonly property string interactorName: {
        if(interactorComponent)
            for(var key in interactorComponentMap)
                if(interactorComponentMap.hasOwnProperty(key) && interactorComponent === interactorComponentMap[key])
                    return key;

        return "";
    }

    property Component interactorComponent: null
    property var interactorComponentMap: null

    property var interactorFolderListModel: FolderListModel {
        id: interactorFolderListModel
        nameFilters: ["*.qml"]
        showDirs: false
        showFiles: true
        sortField: FolderListModel.Name

        Component.onCompleted: refresh();
        onCountChanged: update();

        property var refreshOnSceneLoaded: Connections {
            target: root.scene
            onLoaded: interactorFolderListModel.refresh();
        }

        function refresh() {
            folder = "";
            folder = "qrc:/SofaInteractors";
        }

        function update() {
            if(root.interactorComponentMap)
                for(var key in root.interactorComponentMap)
                    if(root.interactorComponentMap.hasOwnProperty(key))
                        root.interactorComponentMap[key].destroy();

            var interactorComponentMap = [];
            for(var i = 0; i < count; ++i)
            {
                var fileBaseName = get(i, "fileBaseName");
                var filePath = get(i, "filePath").toString();
                if(-1 !== folder.toString().indexOf("qrc:"))
                    filePath = "qrc" + filePath;

                var name = fileBaseName.slice(fileBaseName.indexOf("_") + 1);
                var interactorComponent = Qt.createComponent(filePath);
                interactorComponentMap[name] = interactorComponent;
            }

            if(null === root.interactorComponent)
                if(interactorComponentMap.hasOwnProperty(root.defaultInteractorName))
                    root.interactorComponent = interactorComponentMap[root.defaultInteractorName];

            if(null === root.interactorComponent)
                for(var key in interactorComponentMap)
                    if(interactorComponentMap.hasOwnProperty(key)) {
                        root.interactorComponent = interactorComponentMap[key];
                        break;
                    }

            root.interactorComponentMap = interactorComponentMap;
        }
    }

////////////////////////////////////////////////// SCREENSHOT

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
        root.saveScreenshot("Captured/Screen/" + formatDateForScreenshot() + ".png");
    }

//////////////////////////////////////////////////

}
