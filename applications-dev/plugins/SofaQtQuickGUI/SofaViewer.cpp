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

#include "SofaViewer.h"
#include "SofaScene.h"
#include "Manipulator.h"

#include <sofa/simulation/common/Node.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/core/visual/DrawToolGL.h>
#include <SofaOpenglVisual/OglModel.h>
#include <sofa/helper/cast.h>

#include <QtQuick/qquickwindow.h>
#include <QQmlEngine>
#include <QQmlContext>
#include <QRunnable>
#include <QEventLoop>
//#include <QOpenGLContext>
//#include <QOpenGLPaintDevice>
//#include <QPaintEngine>
//#include <QPainter>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QSGTransformNode>
#include <QSGSimpleTextureNode>
#include <QVector>
#include <QVector4D>
#include <QTime>
#include <QPair>
#include <QThread>
#include <QString>
#include <QDir>
#include <qqml.h>
#include <qmath.h>
#include <limits>

#include <iomanip>
#include <sstream>

namespace sofa
{

namespace qtquick
{

using namespace sofa::simulation;

SofaViewer::SofaViewer(QQuickItem* parent) : QQuickFramebufferObject(parent),
    myFBO(nullptr),
    mySofaScene(nullptr),
    myCamera(nullptr),
    myRoots(),
    myBackgroundColor("#00404040"),
    myBackgroundImageSource(),
    myBackgroundImage(),
    myWireframe(false),
    myCulling(true),
    myBlending(false),
    myAntialiasingSamples(2),
    myMirroredHorizontally(false),
    myMirroredVertically(false),
    myDrawManipulators(true),
    myDrawNormals(false),
    myNormalsDrawLength(1.0f)
{
    setFlag(QQuickItem::ItemHasContents);

    connect(this, &SofaViewer::backgroundImageSourceChanged, this, &SofaViewer::handleBackgroundImageSourceChanged);
    connect(this, SIGNAL(antialiasingSamplesChanged(int)), this, SLOT(update()));
}

SofaViewer::~SofaViewer()
{
	/*sofa::core::visual::VisualParams* _vparams = sofa::core::visual::VisualParams::defaultInstance();
	if(_vparams && _vparams->drawTool())
	{
		delete _vparams->drawTool();
		_vparams->drawTool() = 0;
	}*/

    clearRoots();
}

void SofaViewer::setSofaScene(SofaScene* newSofaScene)
{
    if(newSofaScene == mySofaScene)
		return;

    mySofaScene = newSofaScene;

    sofaSceneChanged(newSofaScene);
}

void SofaViewer::setCamera(Camera* newCamera)
{
	if(newCamera == myCamera)
		return;

	myCamera = newCamera;

	cameraChanged(newCamera);
}

static void appendRoot(QQmlListProperty<SofaComponent> *property, SofaComponent *value)
{
    if(value)
        static_cast<QList<SofaComponent*>*>(property->data)->append(new SofaComponent(*value));
}

static SofaComponent* atRoot(QQmlListProperty<SofaComponent> *property, int index)
{
    return static_cast<QList<SofaComponent*>*>(property->data)->at(index);
}

static void clearRoot(QQmlListProperty<SofaComponent> *property)
{
    QList<SofaComponent*>& roots = *static_cast<QList<SofaComponent*>*>(property->data);

    for(SofaComponent* sofaComponent : roots)
        delete sofaComponent;

    roots.clear();
}

static int countRoot(QQmlListProperty<SofaComponent> *property)
{
    return static_cast<QList<SofaComponent*>*>(property->data)->size();
}

QList<SofaComponent*> SofaViewer::roots() const
{
    return myRoots;
}

QQmlListProperty<SofaComponent> SofaViewer::rootsListProperty()
{
    return QQmlListProperty<SofaComponent>(this, &myRoots, appendRoot, countRoot, atRoot, clearRoot);
}

void SofaViewer::clearRoots()
{
    myRoots.clear();
}

void SofaViewer::setBackgroundColor(QColor newBackgroundColor)
{
    if(newBackgroundColor == myBackgroundColor)
        return;

    myBackgroundColor = newBackgroundColor;

    backgroundColorChanged(newBackgroundColor);
}

void SofaViewer::setBackgroundImageSource(QUrl newBackgroundImageSource)
{
    if(newBackgroundImageSource == myBackgroundImageSource)
        return;

    myBackgroundImageSource = newBackgroundImageSource;

    backgroundImageSourceChanged(newBackgroundImageSource);
}

void SofaViewer::setWireframe(bool newWireframe)
{
    if(newWireframe == myWireframe)
        return;

    myWireframe = newWireframe;

    wireframeChanged(newWireframe);
}

void SofaViewer::setCulling(bool newCulling)
{
    if(newCulling == myCulling)
        return;

    myCulling = newCulling;

    cullingChanged(newCulling);
}

void SofaViewer::setBlending(bool newBlending)
{
    if(newBlending == myBlending)
        return;

    myBlending = newBlending;

    blendingChanged(newBlending);
}

void SofaViewer::setAntialiasingSamples(int newAntialiasingSamples)
{
    if(newAntialiasingSamples == myAntialiasingSamples)
        return;

    myAntialiasingSamples = newAntialiasingSamples;

    antialiasingSamplesChanged(newAntialiasingSamples);
}

void SofaViewer::setMirroredHorizontally(bool newMirroredHorizontally)
{
    if(newMirroredHorizontally == myMirroredHorizontally)
        return;

    myMirroredHorizontally = newMirroredHorizontally;

    mirroredHorizontallyChanged(newMirroredHorizontally);
}

void SofaViewer::setMirroredVertically(bool newMirroredVertically)
{
    if(newMirroredVertically == myMirroredVertically)
        return;

    myMirroredVertically = newMirroredVertically;

    mirroredVerticallyChanged(newMirroredVertically);
}

void SofaViewer::setDrawManipulators(bool newDrawManipulators)
{
    if(newDrawManipulators == myDrawManipulators)
        return;

    myDrawManipulators = newDrawManipulators;

    drawManipulatorsChanged(newDrawManipulators);
}

void SofaViewer::setDrawNormals(bool newDrawNormals)
{
    if(newDrawNormals == myDrawNormals)
        return;

    myDrawNormals = newDrawNormals;

    drawNormalsChanged(newDrawNormals);
}

void SofaViewer::setNormalsDrawLength(float newNormalsDrawLength)
{
    if(newNormalsDrawLength == myNormalsDrawLength)
        return;

    myNormalsDrawLength = newNormalsDrawLength;

    normalsDrawLengthChanged(newNormalsDrawLength);
}

double SofaViewer::computeDepth(const QVector3D& wsPosition) const
{
    if(!myCamera)
        return 1.0;

    return myCamera->computeDepth(wsPosition) * 0.5 + 0.5;
}

QVector3D SofaViewer::mapFromWorld(const QVector3D& wsPoint) const
{
	if(!myCamera)
		return QVector3D();

    QVector4D nsPosition = (myCamera->projection() * myCamera->view() * QVector4D(wsPoint, 1.0));
	nsPosition /= nsPosition.w();

    if(mirroredHorizontally())
        nsPosition.setX(-nsPosition.x());

    if(mirroredVertically())
        nsPosition.setY(-nsPosition.y());

	return QVector3D((nsPosition.x() * 0.5 + 0.5) * qCeil(width()) + 0.5, qCeil(height()) - (nsPosition.y() * 0.5 + 0.5) * qCeil(height()) + 0.5, (nsPosition.z() * 0.5 + 0.5));
}

QVector3D SofaViewer::mapToWorld(const QPointF& ssPoint, double z) const
{
	if(!myCamera)
		return QVector3D();

    QVector3D nsPosition = QVector3D(ssPoint.x() / (double) qCeil(width()) * 2.0 - 1.0, (1.0 - ssPoint.y() / (double) qCeil(height())) * 2.0 - 1.0, z * 2.0 - 1.0);
    if(mirroredHorizontally())
        nsPosition.setX(-nsPosition.x());

    if(mirroredVertically())
        nsPosition.setY(-nsPosition.y());

	QVector4D vsPosition = myCamera->projection().inverted() * QVector4D(nsPosition, 1.0);
	vsPosition /= vsPosition.w();

	return (myCamera->model() * vsPosition).toVector3D();
}


bool SofaViewer::intersectRayWithPlane(const QVector3D& rayOrigin, const QVector3D& rayDirection, const QVector3D& planeOrigin, const QVector3D& planeNormal, QVector3D& intersectionPoint) const
{
    if(0.0 == QVector3D::dotProduct(rayDirection, planeNormal)) // ray and plane are orthogonal
        return false;

    QVector3D normalizedPlaneNormal = planeNormal.normalized();

    double d = QVector3D(0.0, 0.0, 0.0).distanceToPlane(planeOrigin, normalizedPlaneNormal);

    double nDotOrigin = QVector3D::dotProduct(normalizedPlaneNormal, rayOrigin);

    QVector3D normalizedRayDirection = rayDirection.normalized();
    double nDotDir = QVector3D::dotProduct(normalizedPlaneNormal, normalizedRayDirection);

    intersectionPoint = QVector3D(rayOrigin + ((-(nDotOrigin + d) / nDotDir) * normalizedRayDirection));

    // line intersection but not ray intersection (the intersection happened 'behind' the ray)
    if(QVector3D::dotProduct(rayDirection, intersectionPoint - rayOrigin) < 0.0)
        return false;

    return true;
}

QVector3D SofaViewer::projectOnLine(const QPointF& ssPoint, const QVector3D& lineOrigin, const QVector3D& lineDirection) const
{
    if(window())
    {
        QVector3D wsOrigin = mapToWorld(ssPoint, 0.0);
        QVector3D wsDirection = mapToWorld(ssPoint, 1.0) - wsOrigin;

        QVector3D normalizedLineDirection = lineDirection.normalized();
        QVector3D planAxis = QVector3D::normal(normalizedLineDirection, wsDirection);
        QVector3D planNormal = QVector3D::normal(normalizedLineDirection, planAxis);

        QVector3D intersectionPoint;
        if(intersectRayWithPlane(wsOrigin, wsDirection, lineOrigin, planNormal, intersectionPoint))
            return lineOrigin + normalizedLineDirection * QVector3D::dotProduct(normalizedLineDirection, intersectionPoint - lineOrigin);
    }

    return QVector3D(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
}

QVector3D SofaViewer::projectOnPlane(const QPointF& ssPoint, const QVector3D& planeOrigin, const QVector3D& planeNormal) const
{
    if(window())
    {
        QVector3D wsOrigin = mapToWorld(ssPoint, 0.0);
        QVector3D wsDirection = mapToWorld(ssPoint, 1.0) - wsOrigin;

        QVector3D intersectionPoint;
        if(intersectRayWithPlane(wsOrigin, wsDirection, planeOrigin, planeNormal, intersectionPoint))
            return intersectionPoint;
    }

    return QVector3D(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
}

using sofa::simulation::Node;
using sofa::component::visualmodel::OglModel;

class PickUsingRasterizationWorker : public QRunnable
{
public:
    PickUsingRasterizationWorker(const SofaViewer* viewer, const QPointF& ssPoint, const QStringList& tags, Selectable*& selectable, bool& finished) :
        myViewer(viewer),
        mySSPoint(ssPoint),
		myTags(tags),
        mySelectable(selectable),
        myFinished(finished)
    {

    }

    void run()
    {
        QRect rect = myViewer->nativeRect();

        QSize size = rect.size();
        if(size.isEmpty())
            return;

        Camera* camera = myViewer->camera();
        if(!camera)
            return;

        SofaScene* sofaScene = myViewer->sofaScene();
        if(!sofaScene)
            return;

        glDisable(GL_BLEND);
        glDisable(GL_LIGHTING);

        glDisable(GL_CULL_FACE);

        glViewport(0.0, 0.0, size.width(), size.height());

        camera->setAspectRatio(size.width() / (double) size.height());

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadMatrixf(camera->projection().constData());

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(camera->view().constData());

        if(myViewer->wireframe())
            glPolygonMode(GL_FRONT_AND_BACK ,GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);

        if(myViewer->culling())
            glEnable(GL_CULL_FACE);

        mySelectable = sofaScene->pickObject(*myViewer, mySSPoint, myTags, myViewer->roots());

        if(myViewer->wireframe())
            glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        myFinished = true;
    }

private:
    const SofaViewer*   myViewer;
    QPointF             mySSPoint;
	QStringList			myTags;
    Selectable*&        mySelectable;
    bool&               myFinished;

};

QVector4D SofaViewer::projectOnGeometry(const QPointF& ssPoint) const
{
    return projectOnGeometryWithTags(ssPoint, QStringList());
}

QVector4D SofaViewer::projectOnGeometryWithTags(const QPointF& ssPoint, const QStringList& tags) const
{
	if(!window() || !window()->isActive())
		return QVector4D();

	bool finished = false;
	Selectable* selectable = nullptr;

	PickUsingRasterizationWorker* worker = new PickUsingRasterizationWorker(this, ssPoint, tags, selectable, finished);
	window()->scheduleRenderJob(worker, QQuickWindow::AfterSynchronizingStage);
	window()->update();

	// TODO: add a timeout
	while(!finished)
		qApp->processEvents(QEventLoop::WaitForMoreEvents);

	if(selectable)
		return QVector4D(selectable->position(), 1.0f);

	return QVector4D(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), 0.0f);
}

SelectableSofaParticle* SofaViewer::pickParticle(const QPointF& ssPoint) const
{
    return pickParticleWithTags(ssPoint, QStringList());
}

SelectableSofaParticle* SofaViewer::pickParticleWithTags(const QPointF& ssPoint, const QStringList& tags) const
{
	QVector3D nearPosition = mapToWorld(ssPoint, 0.0);
	QVector3D farPosition  = mapToWorld(ssPoint, 1.0);

	QVector3D origin = nearPosition;
	QVector3D direction = (farPosition - nearPosition).normalized();

	double distanceToRay = mySofaScene->radius() / 76.0;
	double distanceToRayGrowth = 0.001;

	return mySofaScene->pickParticle(origin, direction, distanceToRay, distanceToRayGrowth, tags, roots());
}

Selectable* SofaViewer::pickObject(const QPointF& ssPoint)
{
    return pickObjectWithTags(ssPoint, QStringList());
}

Selectable* SofaViewer::pickObjectWithTags(const QPointF& ssPoint, const QStringList& tags)
{
	Selectable* selectable = nullptr;

	if(!window() || !window()->isActive())
		return selectable;

	bool finished = false;

	PickUsingRasterizationWorker* worker = new PickUsingRasterizationWorker(this, ssPoint, tags, selectable, finished);
	window()->scheduleRenderJob(worker, QQuickWindow::AfterSynchronizingStage);
	window()->update();

	// TODO: add a timeout
	while(!finished)
		qApp->processEvents(QEventLoop::WaitForMoreEvents);

	return selectable;
}

QPair<QVector3D, QVector3D> SofaViewer::boundingBox() const
{
    QVector3D min, max;
    mySofaScene->computeBoundingBox(min, max);

    return QPair<QVector3D, QVector3D>(min, max);
}

QVector3D SofaViewer::boundingBoxMin() const
{
    QVector3D min, max;
    mySofaScene->computeBoundingBox(min, max);

    return min;
}

QVector3D SofaViewer::boundingBoxMax() const
{
    QVector3D min, max;
    mySofaScene->computeBoundingBox(min, max);

    return max;
}

void SofaViewer::handleBackgroundImageSourceChanged(QUrl newBackgroundImageSource)
{
    QString path = newBackgroundImageSource.toEncoded();
    if(path.isEmpty())
        path = newBackgroundImageSource.toLocalFile();

    myBackgroundImage = QImage(path.replace("qrc:", ":"));
}

void SofaViewer::saveScreenshot(const QString& path)
{    
    QDir dir = QFileInfo(path).dir();
    if(!dir.exists())
        dir.mkpath(".");

    if(!myFBO->toImage().save(path))
        qWarning() << "Screenshot could not be saved to" << path;
}

class ScreenshotWorker : public QRunnable
{
public:
	ScreenshotWorker(const SofaViewer* viewer, const QString& path, int width, int height) :
		myViewer(viewer),
		myPath(path),
		myWidth(width),
		myHeight(height)
	{

	}

	void run()
	{
		QSize size(myWidth, myHeight);
		if(size.isEmpty())
			return;

		QOpenGLFramebufferObjectFormat format;
		format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
		format.setSamples(myViewer->antialiasingSamples());

		glViewport(0.0, 0.0, myWidth, myHeight);

		QOpenGLFramebufferObject fbo(myWidth, myHeight, format);
		fbo.bind();

		myViewer->internalRender(myWidth, myHeight);

		fbo.release();

		if(!fbo.toImage().save(myPath))
			qWarning() << "Screenshot could not be saved to" << myPath;
	}

private:
	const SofaViewer*	myViewer;
	QString				myPath;
	int					myWidth;
	int					myHeight;

};

void SofaViewer::saveScreenshotWithResolution(const QString& path, int width, int height)
{
	if(!window())
		return;

	QDir dir = QFileInfo(path).dir();
    if(!dir.exists())
        dir.mkpath(".");

	ScreenshotWorker* worker = new ScreenshotWorker(this, path, width, height);
	window()->scheduleRenderJob(worker, QQuickWindow::AfterSynchronizingStage);
	window()->update();
}

QSize SofaViewer::nativeSize() const
{
    return nativeRect().size();
}

QRect SofaViewer::nativeRect() const
{
	if(!window())
		return QRect();

    QPointF realPos = mapToScene(QPointF(0.0, qCeil(height())));
    realPos.setX(realPos.x() * window()->devicePixelRatio());
    realPos.setY((window()->height() - realPos.y()) * window()->devicePixelRatio());  // OpenGL has its Y coordinate inverted compared to Qt

    QPoint pos(qFloor(realPos.x()), qFloor(realPos.y()));
    QSize size((qCeil(width()) + qCeil(pos.x() - realPos.x())) * window()->devicePixelRatio(), (qCeil((height()) + qCeil(pos.y() - realPos.y())) * window()->devicePixelRatio()));
	
	return QRect(pos, size);
}

QRect SofaViewer::qtRect() const
{
    if(!window())
        return QRect();

    QPointF realPos = mapToScene(QPointF(0.0, qCeil(height())));
    realPos.setX( realPos.x() * window()->devicePixelRatio());
    realPos.setY( realPos.y() * window()->devicePixelRatio());

    QPoint pos(qFloor(realPos.x()), qFloor(realPos.y()));
    QSize size((qCeil(width()) + qCeil(pos.x() - realPos.x())) * window()->devicePixelRatio(), (qCeil((height()) + qCeil(pos.y() - realPos.y())) * window()->devicePixelRatio()));

    return QRect(pos, size);
}

QPointF SofaViewer::mapToNative(const QPointF& ssPoint) const
{
    QPointF ssNativePoint(ssPoint);

    if(mirroredHorizontally())
        ssNativePoint.setX(width() - ssPoint.x());

    if(!mirroredVertically())
        ssNativePoint.setY(height() - ssPoint.y());

    ssNativePoint.setX(ssNativePoint.x() * window()->devicePixelRatio());
    ssNativePoint.setY(ssNativePoint.y() * window()->devicePixelRatio());

    return ssNativePoint;
}

void SofaViewer::viewAll()
{
    if(!myCamera || !mySofaScene || !mySofaScene->isReady())
		return;

	QVector3D min, max;
    mySofaScene->computeBoundingBox(min, max);

    myCamera->fit(min, max);
}

QSGNode* SofaViewer::updatePaintNode(QSGNode* inOutNode, UpdatePaintNodeData* inOutData)
{
    if(!inOutNode)
    {
        inOutNode = QQuickFramebufferObject::updatePaintNode(inOutNode, inOutData);
        QSGSimpleTextureNode* textureNode = static_cast<QSGSimpleTextureNode*>(inOutNode);
        if(textureNode)
        {
            QSGSimpleTextureNode::TextureCoordinatesTransformMode mirroredHorizontallyFlag = mirroredHorizontally() ? QSGSimpleTextureNode::MirrorHorizontally : QSGSimpleTextureNode::NoTransform;
            QSGSimpleTextureNode::TextureCoordinatesTransformMode mirroredVerticallyFlag = mirroredVertically() ? QSGSimpleTextureNode::NoTransform : QSGSimpleTextureNode::MirrorVertically;

            textureNode->setTextureCoordinatesTransform(mirroredHorizontallyFlag | mirroredVerticallyFlag);
        }

        return inOutNode;
    }
    return QQuickFramebufferObject::updatePaintNode(inOutNode, inOutData);
}

void SofaViewer::internalRender(int width, int height) const
{
	QSize size(width, height);
	if(size.isEmpty())
		return;

	//    if(!myBackgroundImage.isNull())
	//    {
	//        // TODO: warning: disable lights, but why ?
	//        QOpenGLPaintDevice device(size);
	//        QPainter painter(&device);
	//        painter.drawImage(size.width() - myBackgroundImage.width(), size.height() - myBackgroundImage.height(), myBackgroundImage);
	//    }

	glClearColor(myBackgroundColor.redF(), myBackgroundColor.greenF(), myBackgroundColor.blueF(), myBackgroundColor.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(!myCamera)
		return;

	// set default lights
	{
		glEnable(GL_LIGHT0);
		{
			float lightPosition[] = { 0.5f,  0.5f, 1.0f, 0.0f};
			float lightAmbient [] = { 0.0f,  0.0f, 0.0f, 0.0f};
			float lightDiffuse [] = { 1.0f,  1.0f, 1.0f, 1.0f};
			float lightSpecular[] = { 0.0f,  0.0f, 0.0f, 0.0f};

			glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
			glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
			glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
			glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
		}

		glEnable(GL_LIGHT1);
		{
			float lightPosition[] = { -1.0f, -1.0f,-1.0f, 0.0f};
			float lightAmbient [] = {  0.0f,  0.0f, 0.0f, 0.0f};
			float lightDiffuse [] = { 0.25f, 0.25f, 0.5f, 0.0f};
			float lightSpecular[] = {  0.0f,  0.0f, 0.0f, 0.0f};

			glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
			glLightfv(GL_LIGHT1, GL_AMBIENT,  lightAmbient);
			glLightfv(GL_LIGHT1, GL_DIFFUSE,  lightDiffuse);
			glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
		}
	}

	glEnable(GL_LIGHTING);

	if(blending())
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	if(mySofaScene && mySofaScene->isReady())
	{
		glDisable(GL_CULL_FACE);

		myCamera->setAspectRatio(width / (double) height);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixf(myCamera->projection().constData());

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadMatrixf(myCamera->view().constData());

		if(wireframe())
			glPolygonMode(GL_FRONT_AND_BACK ,GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);

		if(culling())
			glEnable(GL_CULL_FACE);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);

		// qt does not release its shader program and we do not use one so we have to release the current bound program
		glUseProgram(0);

		// prepare the sofa visual params
		sofa::core::visual::VisualParams* _vparams = sofa::core::visual::VisualParams::defaultInstance();
		if(_vparams)
		{
			if(!_vparams->drawTool())
			{
				_vparams->drawTool() = new sofa::core::visual::DrawToolGL();
				_vparams->setSupported(sofa::core::visual::API_OpenGL);
			}

			GLint _viewport[4];
			GLdouble _mvmatrix[16], _projmatrix[16];

			glGetIntegerv (GL_VIEWPORT, _viewport);
			glGetDoublev  (GL_MODELVIEW_MATRIX, _mvmatrix);
			glGetDoublev  (GL_PROJECTION_MATRIX, _projmatrix);

			_vparams->viewport() = sofa::helper::fixed_array<int, 4>(_viewport[0], _viewport[1], _viewport[2], _viewport[3]);
			_vparams->sceneBBox() = mySofaScene->sofaSimulation()->GetRoot()->f_bbox.getValue();
			_vparams->setProjectionMatrix(_projmatrix);
			_vparams->setModelViewMatrix(_mvmatrix);
		}

		// draw the SofaScene
		{
			preDraw();
			mySofaScene->draw(*this, roots());
			postDraw();
		}

		if(wireframe())
			glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	if(blending())
		glDisable(GL_BLEND);
}

SofaViewer::SofaRenderer::SofaRenderer(SofaViewer* viewer) : QQuickFramebufferObject::Renderer(),
    myViewer(viewer),
    myAntialiasingSamples(0)
{

}

QOpenGLFramebufferObject* SofaViewer::SofaRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(myAntialiasingSamples);

    myViewer->myFBO = new QOpenGLFramebufferObject(size, format);
    return myViewer->myFBO;
}

void SofaViewer::SofaRenderer::synchronize(QQuickFramebufferObject* quickFramebufferObject)
{
    SofaViewer* viewer = qobject_cast<SofaViewer*>(quickFramebufferObject);
    if(!viewer)
        return;

    if(myAntialiasingSamples != viewer->myAntialiasingSamples)
    {
        myAntialiasingSamples = viewer->myAntialiasingSamples;
        invalidateFramebufferObject();
    }
}

void SofaViewer::SofaRenderer::render()
{
    update();

    if(!myViewer || !myViewer->isVisible())
        return;

	myViewer->internalRender(myViewer->width(), myViewer->height());
}

}

}
