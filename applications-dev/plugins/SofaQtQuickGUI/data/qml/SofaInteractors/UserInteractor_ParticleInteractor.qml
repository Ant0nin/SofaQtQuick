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
import SofaBasics 1.0
import SceneComponent 1.0
import "qrc:/SofaCommon/SofaToolsScript.js" as SofaToolsScript

UserInteractor_MoveCamera {
    id: root

    property var selectedManipulator: null
    property var selectedComponent: null

    function init() {

        moveCamera_init();

        addMousePressedMapping(Qt.LeftButton, function(mouse, viewer) {
            selectedManipulator = scene.selectedManipulator;
            selectedComponent = scene.selectedComponent;

            var sceneComponentParticle = viewer.pickParticle(Qt.point(mouse.x + 0.5, mouse.y + 0.5));
            if(sceneComponentParticle) {
                scene.particleInteractor.start(sceneComponentParticle.sceneComponent, sceneComponentParticle.particleIndex);

                setMouseMovedMapping(function(mouse, viewer) {
                    var z = viewer.computeDepth(scene.particleInteractor.particlePosition());
                    var position = viewer.mapToWorld(Qt.point(mouse.x + 0.5, mouse.y + 0.5), z);
                    scene.particleInteractor.update(position);
                });
            }
        });

        addMouseReleasedMapping(Qt.LeftButton, function(mouse, viewer) {
            if(scene.particleInteractor)
                scene.particleInteractor.release();

            if(selectedManipulator && selectedManipulator.mouseReleased)
                selectedManipulator.mouseReleased(mouse, viewer);

            scene.selectedManipulator = null;

            setMouseMovedMapping(null);
        });

    }
}
