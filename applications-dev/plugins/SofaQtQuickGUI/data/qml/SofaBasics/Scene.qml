import QtQuick 2.0
import QtQuick.Controls 1.0
import Qt.labs.settings 1.0
import PickingInteractor 1.0
import Scene 1.0
import "qrc:/SofaCommon/SofaSettingsScript.js" as SofaSettingsScript
import "qrc:/SofaCommon/SofaToolsScript.js" as SofaToolsScript

Scene {
    id: root

    asynchronous: true
    header: ""
    source: ""
    sourceQML: ""
    property string statusMessage: ""

    onStatusChanged: {
		if(listModel)
			listModel.selectedId = -1;

        clearManipulators();

        var path = source.toString().replace("///", "/").replace("file:", "");
        switch(status) {
        case Scene.Loading:
            statusMessage = 'Loading "' + path + '" please wait';
            break;
        case Scene.Error:
            statusMessage = 'Scene "' + path + '" issued an error during loading';
            break;
        case Scene.Ready:
            statusMessage = 'Scene "' + path + '" loaded successfully';
            SofaSettingsScript.Recent.add(path);
            break;
        }
    }

    property var listModel: SceneListModel {id : listModel}
    property bool listModelDirty: true

    onStepEnd: {
        if(root.play)
            listModelDirty = true;
        else if(listModel)
                listModel.update();
    }

    onReseted: if(listModel) listModel.update();

    property var listModelUpdateTimer: Timer {
        running: root.play && root.listModel ? true : false
        repeat: true
        interval: 200
        onTriggered: {
            if(root.listModelDirty) {
                root.listModel.update()
                root.listModelDirty = false;
            }
        }
    }

    // convenience
    readonly property bool ready: status === Scene.Ready

    // allow us to interact with the python script controller
    property var pythonInteractor: PythonInteractor {scene: root}

    // allow us to interact with the scene physics
    property var pickingInteractor: PickingInteractor {
        stiffness: 100

        onPickingChanged: SofaToolsScript.Tools.overrideCursorShape = picking ? Qt.BlankCursor : 0
    }

    function keyPressed(event) {
        if(event.modifiers & Qt.ShiftModifier)
            onKeyPressed(event.key);
    }

    function keyReleased(event) {
        //if(event.modifiers & Qt.ShiftModifier)
            onKeyReleased(event.key);
    }

	property var resetAction: Action {
        text: "&Reset"
        shortcut: "Ctrl+Alt+R"
        onTriggered: root.reset();
        tooltip: "Reset the simulation"
    }

    function dataValue(dataName) {
        if(arguments.length == 1) {
            return onDataValue(dataName);
        }

        console.debug("ERROR: Scene - using dataValue with an invalid number of arguments:", arguments.length);
    }

    function setDataValue(dataName) {
        if(arguments.length > 1){
            var packedArguments = [];
            for(var i = 1; i < arguments.length; i++)
                packedArguments.push(arguments[i]);

            return onSetDataValue(dataName, packedArguments);
        }

        console.debug("ERROR: Scene - using setDataValue with an invalid number of arguments:", arguments.length);
    }

    ///// SELECTED COMPONENTS

    function selectedComponent() {
        if(0 === root.selectedComponents.length)
            return null;

        return root.selectedComponents[0];
    }

    function setSelectedComponent(selectedComponent) {
        var selectedComponents = [];
        selectedComponents.push(selectedComponent);
        root.selectedComponents = selectedComponents;
    }

    function clearselectedComponents() {
        root.selectedComponents = [];
    }

    ///// MANIPULATOR

    function addManipulator(manipulator) {
        var manipulators = [];
        for(var i = 0; i < root.manipulators.length; ++i)
            manipulators.push(root.manipulators[i]);

        manipulators.push(manipulator);
        root.manipulators = manipulators;

        // if the added manipulator is a compound also add its children manipulators
        if(manipulator.manipulators && 0 !== manipulator.manipulators.length)
            for(var i = 0; i < manipulator.manipulators.length; ++i)
                addManipulator(manipulator.manipulators[i]);
    }

    function removeManipulator(manipulator) {
        var manipulators = [];
        for(var i = 0; i < root.manipulators; ++i)
            if(manipulator !== root.manipulators[i])
                manipulators.push(root.manipulators[i]);

        root.manipulators = manipulators;
    }

    function clearManipulators() {
        root.manipulators = [];
    }

    ///// SELECTED MANIPULATORS

    function selectedManipulator() {
        if(0 === root.selectedManipulators.length)
            return null;

        return root.selectedManipulators[0];
    }

    function setSelectedManipulator(selectedManipulator) {
        var selectedManipulators = [];
        selectedManipulators.push(selectedManipulator);
        root.selectedManipulators = selectedManipulators;
    }

    function clearSelectedManipulators() {
        root.selectedManipulators = [];
    }
}
