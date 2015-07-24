#ifndef MANIPULATOR2D_TRANSLATION_H
#define MANIPULATOR2D_TRANSLATION_H

#include "SofaQtQuickGUI.h"
#include "Manipulator.h"

#include <QObject>
#include <QVector3D>

namespace sofa
{

namespace qtquick
{

class SOFA_SOFAQTQUICKGUI_API Manipulator2D_Translation : public Manipulator
{
    Q_OBJECT

public:
    explicit Manipulator2D_Translation(QObject* parent = 0);
    ~Manipulator2D_Translation();

public:
    Q_PROPERTY(QString axis READ axis WRITE setAxis NOTIFY axisChanged)

public:
    QString axis() const {return myAxis;}
    void setAxis(QString newAxis);

signals:
    void axisChanged(QString newAxis);

public slots:
    virtual void draw(const Viewer& viewer) const;
    virtual void pick(const Viewer& viewer) const;

private:
    void internalDraw(const Viewer& viewer, bool isPicking = false) const;

private:
    QString myAxis;

};

}

}

#endif // MANIPULATOR2D_TRANSLATION_H
