import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.1
import Qt.labs.settings 1.0
import SofaBasics 1.0
import Scene 1.0
import PythonConsole 1.0
import "qrc:/SofaCommon/SofaSettingsScript.js" as SofaSettingsScript

Rectangle {
    id: root
    clip: true
    color: "lightgrey"

    property var scene

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TextArea {
            id: consoleTextArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            readOnly: true

            onTextChanged: cursorPosition = Math.max(0, text.length - 1)

            Connections {
                target: scene
                onAboutToUnload: consoleTextArea.text = ""
            }

            PythonConsole {
                onTextAdded: consoleTextArea.text += text
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 24

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 32
                spacing: 0

                TextField {
                    id: scriptTextField
                    Layout.fillWidth: true
                    onAccepted: run();

                    property int indexHistory: 0
                    property int maxHistory: 100
                    property var commandHistory: ["", ""]
                    property bool lock: false

                    onTextChanged: {
                        if(0 === indexHistory || !lock)
                            commandHistory[0] = text;

                        if(!lock)
                            indexHistory = 0;
                    }

                    Keys.onUpPressed: {
                        if(commandHistory.length - 1 === indexHistory)
                            return;

                        indexHistory = Math.min(++indexHistory, commandHistory.length - 1);

                        lock = true;
                        text = commandHistory[indexHistory];
                        lock = false;
                    }

                    Keys.onDownPressed: {
                        if(0 === indexHistory)
                            return;

                        indexHistory = Math.max(--indexHistory, 0);
                        lock = true;
                        text = commandHistory[indexHistory];
                        lock = false;
                    }

                    function run() {
                        if(0 === text.length)
                            return;

                        scene.pythonInteractor.run(text);

                        if(0 !== text.localeCompare(commandHistory[1])) {
                            commandHistory[0] = text;
                            commandHistory.splice(0, 0, "");
                            if(commandHistory.length > maxHistory)
                                commandHistory.length = maxHistory;
                        }

                        indexHistory = 0;
                        text = "";
                    }
                }

                Button {
                    text: "Run"
                    onClicked: scriptTextField.run();
                }
            }
        }
    }
}
