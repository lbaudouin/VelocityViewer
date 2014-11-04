#ifndef VELOCITYDATA_H
#define VELOCITYDATA_H

#include <QObject>

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QPointF>

#include <QMessageBox>
#include <QDebug>

#include <assert.h>

#include "math.h"

struct Curve;
struct ModeInfo;
struct CurveInfo;

class VelocityData
{
public:
    VelocityData(QWidget *parent = 0);
    enum VelocityMode { MaxVel = 0, LonAcc, LatAcc, NoMode };

    bool loadCurvature(const QString &curvatureFilename);
    bool loadVelocityProfile(const QString &velocityFilename);
    bool loadPath(const QString &xFilename, const QString &yFilename);

    bool isValid();

    QPointF path(double abscissa) const;
    double curvature(double abscissa) const;
    double velocity(double abscissa) const;
    QPointF path(int curveIndex, double localAbscissa) const;
    double curvature(int curveIndex, double localAbscissa) const;
    double velocity(int curveIndex, double localAbscissa) const;

    int findCurveIndex(double abscissa) const;
    VelocityMode mode(double abscissa) const;
    QList<ModeInfo> modes() const;
    int curveInfo(double abscissa) const;
    QList<CurveInfo> curveInfos() const;

    double length() const;

    QList<QPointF> startPoints() const;

    QPair<int,double> findClosestPoint(double x, double y) const;


    // CurveIndex-CurveLocalAbsciss
    QPair<int,double> toLocalAbscissa(double abscissa) const;
    double toGlobalAbscissa(int curveIndex, double localAbscissa) const;

protected:
    double evalPoly(const QList<double> &poly, const double &s) const;


private:
    QList< QList<double> > m_xPath, m_yPath, m_curvature;
    QList< Curve > m_velocityCurves;

    QWidget *m_parent;

};

struct ModeInfo
{
    VelocityData::VelocityMode mode;
    double s0,s1;
};

struct CurveInfo
{
    int index;
    double s0,s1;
};

struct Curve
{
    int index;
    QList< VelocityData::VelocityMode > mode;
    QList< double > u0, u1, s0, s1;
    QList< QList<double> > values;
};

#endif // VELOCITYDATA_H
