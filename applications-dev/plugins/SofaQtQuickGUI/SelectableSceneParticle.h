#ifndef SELECTABLE_SCENE_COMPONENT_PARTICLE_H
#define SELECTABLE_SCENE_COMPONENT_PARTICLE_H

#include "SofaQtQuickGUI.h"
#include "SelectableSceneComponent.h"

namespace sofa
{

namespace qtquick
{

/// QtQuick wrapper for a selectable object representing a SceneComponent particle (a dof)
class SOFA_SOFAQTQUICKGUI_API SelectableSceneParticle : public SelectableSceneComponent
{
    Q_OBJECT

public:
    SelectableSceneParticle(const SceneComponent& sceneComponent, int particleIndex);
    ~SelectableSceneParticle();

public:
    Q_PROPERTY(int particleIndex READ particleIndex)

public:
    int particleIndex() const {return myParticleIndex;}

private:
    int myParticleIndex;

};

}

}

#endif // SELECTABLE_SCENE_COMPONENT_PARTICLE_H
