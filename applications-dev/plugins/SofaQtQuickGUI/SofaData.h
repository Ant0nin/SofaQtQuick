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

#ifndef SOFA_DATA_H
#define SOFA_DATA_H

#include "SofaQtQuickGUI.h"

#include <sofa/simulation/common/Simulation.h>
#include <QVariantMap>

class QTimer;
class QOpenGLShaderProgram;

namespace sofa
{

namespace qtquick
{

class SofaScene;
class SofaComponent;

/// QtQuick wrapper for a sofa data (i.e baseData), allowing us to share a component data in a QML context
class SOFA_SOFAQTQUICKGUI_API SofaData : public QObject
{
    Q_OBJECT

public:
    SofaData(const SofaComponent* sofaComponent, const sofa::core::objectmodel::BaseData* data);
    SofaData(const SofaScene* sofaScene, const sofa::core::objectmodel::Base* base, const sofa::core::objectmodel::BaseData* data);
//    SceneData(const SceneData& sceneData);

    Q_INVOKABLE QVariantMap object() const;

    Q_INVOKABLE QVariant value();
    Q_INVOKABLE bool setValue(const QVariant& value);
    Q_INVOKABLE bool setLink(const QString& path);

    sofa::core::objectmodel::BaseData* data();
    const sofa::core::objectmodel::BaseData* data() const;

private:
    const SofaScene*                                    mySofaScene;
    mutable const sofa::core::objectmodel::Base*        myBase;
    mutable const sofa::core::objectmodel::BaseData*    myData;

};

}

}

#endif // SOFA_DATA_H
