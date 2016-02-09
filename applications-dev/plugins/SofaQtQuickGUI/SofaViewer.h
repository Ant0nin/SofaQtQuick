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

#ifndef SOFAVIEWER_H
#define SOFAVIEWER_H

#include "SofaQtQuickGUI.h"
#include "Camera.h"
#include "SelectableSofaParticle.h"

#include <QtQuick/QQuickFramebufferObject>
#include <QVector3D>
#include <QVector4D>
#include <QImage>
#include <QColor>
#include <QList>

#ifdef SOFA_HAVE_PNG
#include <sofa/helper/io/ImagePNG.h>
#else
#include <sofa/helper/io/ImageBMP.h>
#endif

namespace sofa
{

namespace qtquick
{

class SofaRenderer;
class SofaComponent;
class SofaScene;
class Camera;
class Manipulator;

class PickUsingRasterizationWorker;
class ScreenshotWorker;

/// \class Display a Sofa Scene in a QQuickFramebufferObject
/// \note Coordinate prefix meaning:
/// ws  => world space
/// vs  => view space
/// cs  => clip space
/// ndc => ndc space
/// ss  => screen space (window space)
class SOFA_SOFAQTQUICKGUI_API SofaViewer : public QQuickFramebufferObject
{
    Q_OBJECT

    friend class PickUsingRasterizationWorker;
	friend class ScreenshotWorker;
    friend class SofaRenderer;

public:
    explicit SofaViewer(QQuickItem* parent = 0);
    ~SofaViewer();

public:
    Q_PROPERTY(sofa::qtquick::SofaScene* sofaScene READ sofaScene WRITE setSofaScene NOTIFY sofaSceneChanged)
    Q_PROPERTY(sofa::qtquick::Camera* camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(QQmlListProperty<sofa::qtquick::SofaComponent> roots READ rootsListProperty)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QUrl backgroundImageSource READ backgroundImageSource WRITE setBackgroundImageSource NOTIFY backgroundImageSourceChanged)
    Q_PROPERTY(bool wireframe READ wireframe WRITE setWireframe NOTIFY wireframeChanged)
    Q_PROPERTY(bool culling READ culling WRITE setCulling NOTIFY cullingChanged)
    Q_PROPERTY(bool blending READ blending WRITE setBlending NOTIFY blendingChanged)
    Q_PROPERTY(int antialiasingSamples READ antialiasingSamples WRITE setAntialiasingSamples NOTIFY antialiasingSamplesChanged)
    Q_PROPERTY(bool mirroredHorizontally READ mirroredHorizontally WRITE setMirroredHorizontally NOTIFY mirroredHorizontallyChanged)
    Q_PROPERTY(bool mirroredVertically READ mirroredVertically WRITE setMirroredVertically NOTIFY mirroredVerticallyChanged)
    Q_PROPERTY(bool drawManipulators READ drawManipulators WRITE setDrawManipulators NOTIFY drawManipulatorsChanged)
    Q_PROPERTY(bool drawNormals READ drawNormals WRITE setDrawNormals NOTIFY drawNormalsChanged)
    Q_PROPERTY(float normalsDrawLength READ normalsDrawLength WRITE setNormalsDrawLength NOTIFY normalsDrawLengthChanged)

public:
    Renderer* createRenderer() const {return new SofaRenderer(const_cast<SofaViewer*>(this));}

    SofaScene* sofaScene() const        {return mySofaScene;}
    void setSofaScene(SofaScene* newScene);

    Camera* camera() const      {return myCamera;}
    void setCamera(Camera* newCamera);

    QList<SofaComponent*> roots() const;
    QQmlListProperty<SofaComponent> rootsListProperty();
    void clearRoots();

    QColor backgroundColor() const	{return myBackgroundColor;}
    void setBackgroundColor(QColor newBackgroundColor);

    QUrl backgroundImageSource() const	{return myBackgroundImageSource;}
    void setBackgroundImageSource(QUrl newBackgroundImageSource);

    bool wireframe() const      {return myWireframe;}
    void setWireframe(bool newWireframe);

    bool culling() const        {return myCulling;}
    void setCulling(bool newCulling);

    bool blending() const        {return myBlending;}
    void setBlending(bool newBlending);

    int antialiasingSamples() const        {return myAntialiasingSamples;}
    void setAntialiasingSamples(int newAntialiasingSamples);

    bool mirroredHorizontally() const        {return myMirroredHorizontally;}
    void setMirroredHorizontally(bool newMirroredHorizontally);

    bool mirroredVertically() const        {return myMirroredVertically;}
    void setMirroredVertically(bool newMirroredVertically);

    bool drawManipulators() const {return myDrawManipulators;}
    void setDrawManipulators(bool newDrawManipulators);

    bool drawNormals() const {return myDrawNormals;}
    void setDrawNormals(bool newDrawNormals);

    float normalsDrawLength() const {return myNormalsDrawLength;}
    void setNormalsDrawLength(float newNormalsDrawLength);

    /// @return depth in screen space
    Q_INVOKABLE double computeDepth(const QVector3D& wsPosition) const;

    Q_INVOKABLE QVector3D mapFromWorld(const QVector3D& wsPoint) const;
    Q_INVOKABLE QVector3D mapToWorld(const QPointF& ssPoint, double z) const;

    /// @brief map screen coordinates to opengl coordinates
    QPointF mapToNative(const QPointF& ssPoint) const;

    QSize nativeSize() const;

    bool intersectRayWithPlane(const QVector3D& rayOrigin, const QVector3D& rayDirection, const QVector3D& planeOrigin, const QVector3D& planeNormal, QVector3D& intersectionPoint) const;

    Q_INVOKABLE QVector3D projectOnLine(const QPointF& ssPoint, const QVector3D& lineOrigin, const QVector3D& lineDirection) const;
    Q_INVOKABLE QVector3D projectOnPlane(const QPointF& ssPoint, const QVector3D& planeOrigin, const QVector3D& planeNormal) const;
    Q_INVOKABLE QVector4D projectOnGeometry(const QPointF& ssPoint) const;    // .w == 0 => background hit ; .w == 1 => geometry hit

    Q_INVOKABLE sofa::qtquick::SelectableSofaParticle*    pickParticle(const QPointF& ssPoint) const;
    Q_INVOKABLE sofa::qtquick::Selectable*                 pickObject(const QPointF& ssPoint);

    Q_INVOKABLE QPair<QVector3D, QVector3D> boundingBox() const;
    Q_INVOKABLE QVector3D boundingBoxMin() const;
    Q_INVOKABLE QVector3D boundingBoxMax() const;

    Q_INVOKABLE void saveScreenshot(const QString& path);
	Q_INVOKABLE void saveScreenshotWithResolution(const QString& path, int width, int height);

signals:
    void sofaSceneChanged(sofa::qtquick::SofaScene* newScene);
    void rootsChanged(QList<sofa::qtquick::SofaComponent> newRoots);
    void cameraChanged(sofa::qtquick::Camera* newCamera);
    void backgroundColorChanged(QColor newBackgroundColor);
    void backgroundImageSourceChanged(QUrl newBackgroundImageSource);
    void wireframeChanged(bool newWireframe);
    void cullingChanged(bool newCulling);
    void blendingChanged(bool newBlending);
    void antialiasingSamplesChanged(int newAntialiasingSamples);
    void mirroredHorizontallyChanged(bool newMirroredHorizontally);
    void mirroredVerticallyChanged(bool newMirroredVertically);
    void drawManipulatorsChanged(bool newDrawManipulators);
    void drawNormalsChanged(bool newDrawNormals);
    void normalsDrawLengthChanged(float newNormalsDrawLength);

    void preDraw() const;
    void postDraw() const;

public slots:
    void viewAll();

protected:
    QSGNode* updatePaintNode(QSGNode* inOutNode, UpdatePaintNodeData* inOutData);
	void internalRender(int width, int height) const;

private:
    QRect nativeRect() const;
    QRect qtRect() const;

private slots:
    void handleBackgroundImageSourceChanged(QUrl newBackgroundImageSource);

private:
    class SofaRenderer : public QQuickFramebufferObject::Renderer
    {
    public:
        SofaRenderer(SofaViewer* viewer);

    protected:
        QOpenGLFramebufferObject *createFramebufferObject(const QSize &size);
        void synchronize(QQuickFramebufferObject* quickFramebufferObject);
        void render();

    private:
        // TODO: not safe at all when we will use multithreaded rendering, use synchronize() instead
        SofaViewer* myViewer;
        int     myAntialiasingSamples;

    };

private:
    QOpenGLFramebufferObject*   myFBO;
    SofaScene*                  mySofaScene;
	Camera*						myCamera;
    QList<SofaComponent*>       myRoots;
    QColor                      myBackgroundColor;
    QUrl                        myBackgroundImageSource;
    QImage                      myBackgroundImage;
    bool                        myWireframe;
    bool                        myCulling;
    bool                        myBlending;
    int                         myAntialiasingSamples;
    bool                        myMirroredHorizontally;
    bool                        myMirroredVertically;
    bool                        myDrawManipulators;
    bool                        myDrawNormals;
    float                       myNormalsDrawLength;

};

}

}

#endif // SOFAVIEWER_H
