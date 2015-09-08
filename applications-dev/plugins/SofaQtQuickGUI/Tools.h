#ifndef TOOLS_H
#define TOOLS_H

#include "SofaQtQuickGUI.h"
#include <QQmlApplicationEngine>
#include <QApplication>
#include <QQuickWindow>
#include <QQuickItem>
#include <QSettings>
#include <QPluginLoader>

class QOpenGLDebugLogger;

namespace sofa
{

namespace qtquick
{

class SOFA_SOFAQTQUICKGUI_API Tools : public QObject
{
    Q_OBJECT

public:
    Tools(QObject* parent = 0);
    ~Tools();

public:
    Q_PROPERTY(int overrideCursorShape READ overrideCursorShape WRITE setOverrideCursorShape NOTIFY overrideCursorShapeChanged)

public:
    int overrideCursorShape() const {return QApplication::overrideCursor() ? QApplication::overrideCursor()->shape() : Qt::ArrowCursor;}
    void setOverrideCursorShape(int newCursorShape);

signals:
    void overrideCursorShapeChanged();
    void windowChanged();

public:
    Q_INVOKABLE QQuickWindow* window(QQuickItem* item) const;
	Q_INVOKABLE void trimCache(QObject* object = 0);
	Q_INVOKABLE void clearSettingGroup(const QString& group);

public:
    static void SetOpenGLDebugContext();    // must be call before the window has been shown
    static void UseOpenGLDebugLogger();     // must be call after a valid opengl debug context has been made current

    static void UseDefaultSofaPath();
    static void UseDefaultSettingsAtFirstLaunch(const QString& defaultSettingsPath = QString());
    static void CopySettings(const QSettings& src, QSettings& dst);

    /// Centralized common settings for most sofaqtquick applications.
    /// Every applications will be updated when modifying this code.
    /// To be called in the main function.
    static bool DefaultMain(QApplication& app, QQmlApplicationEngine& applicationEngine , const QString &mainScript);

};

}

}

#endif // TOOLS_H
