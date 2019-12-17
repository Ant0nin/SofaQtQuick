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

#include <SofaQtQuickGUI/SofaApplication.h>
#include <iostream>

int main(int argc, char **argv)
{
    // IMPORTANT NOTE: this function MUST be call before QApplication creation in order to be able to load a SofaScene containing calls to OpenGL functions (e.g. containing OglModel)
    sofa::qtquick::SofaApplication::Initialization();

    QApplication app(argc, argv);
    QQmlApplicationEngine applicationEngine;

    // application specific settings
    app.setOrganizationName("Sofa Consortium");
    app.setApplicationName("runSofa2");
    app.setApplicationVersion("v1.0");

    // Add import search path
//    applicationEngine.addImportPath("./qml_modules");

    std::cout << "== Qt: importPathList (QML stuff):" << std::endl;
    for(auto& path : applicationEngine.importPathList()) {
        std::cout << path.toStdString() << std::endl;
    }

//    applicationEngine.addPluginPath("/home/abernard/Workspace/sofa/build_master_debug/lib"); // TODO: do this using a CMakeLists.txt if possible, or use a relative path

    std::cout << "== Qt: pluginPathList (compiled C++):" << std::endl;
    for(auto& path : applicationEngine.pluginPathList()) {
        std::cout << path.toStdString() << std::endl;
    }

    // common settings for most sofaqtquick applications
    if(!sofa::qtquick::SofaApplication::DefaultMain(app, applicationEngine, "qrc:/qml/Main.qml"))
        return -1;

    return app.exec();
}
