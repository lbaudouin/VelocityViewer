#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QMessageBox>

#include <QGraphicsLineItem>

#include "qcustomplot.h"

#include "velocitydata.h"

#include "colordialog.h"

#include <iostream>
#include <fstream>

#include <QTimer>
#include <QSettings>

#include <QDebug>

class MinMaxRange
{
public :
    MinMaxRange() : m_init(false) {}
    MinMaxRange(double value) : m_init(true) {m_min = m_max = value;}

    void clear() { m_init = false; }

    bool addValue(double value){
        if(!m_init){
            m_min = m_max = value;
            m_init = true;
            return true;
        }

        if(value<m_min){
            m_min = value;
            return true;
        }
        if(value>m_max){
            m_max = value;
            return true;
        }
        return false;
    }
    double min() const {return m_min;}
    double max() const {return m_max;}
    void range(double &min, double &max) const { min = m_min; max = m_max; }

private:
    bool m_init;
    double m_min, m_max;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void drawPath();
    void drawPavin();
    void drawPavinElements(QString filebase, int max, QPen pen, QBrush brush = QBrush());

    QList<QPointF> loadPavinElement(QString filename);

    inline double frand(double min, double max)
    {
        if(min==max) return min;
        if(min>max) std::swap(min,max);
        double f = (double)rand() / RAND_MAX;
        return min + f * (max - min);
    }

private:
    Ui::MainWindow *ui;

    VelocityData vp;
    bool m_init;
    bool m_loop;
    
    bool autoReplot;

    //Ranges
    MinMaxRange curvatureRange, velocityRange, longitudinalErrorRange, lateralErrorRange;

    //Robots
    QMap<int,QGraphicsEllipseItem*> robotEllipseItem;
    QMap<int,QGraphicsLineItem*> robotLineItem;
    QMap<int,QGraphicsPathItem*> robotTraceItem;

    //Bar graphs
    QMap<int,QCPBars*> longitudinalBarGraphs;
    QMap<int,QCPBars*> longitudinalBarGraphs2;
    QMap<int,QCPBars*> lateralBarGraphs;
    QMap<int,QCPBars*> lateralBarGraphs2;
    QMap<int,QCPItemText*> longitudinalErrorValue;
    QMap<int,QCPItemText*> lateralErrorValue;

    //Subplot
    QCPAxisRect *upRect,*bottomRect;

    QMap<int,QCPGraph*> positionGraph;
    QMap<int,QCPGraph*> errorGraph;
    QMap<int,QCPGraph*> errorCurveGraph;

    //Default colors
    QList<QColor> m_colors;

    //Simulation
    QTimer *timer;
    double simulatedTime;
    int screenshotIndex;

private slots:

  void setupPlot();
  
  void zoomIn();
  void zoomOut();
  void zoomFit();
  void zoom(int);
  
  void positionFit();
  void positionChanged(int);
  void xAxisChanged(QCPRange range);

public slots:
  bool load(QString xPathFilename, QString yPathFilename, QString curvatureFilename, QString velocityFilename, bool loop = false);

  void setRobotPositionVelocityError(int index, double x, double y, double abscissa, double velocity, double longitudinalError, double ratioLongitudinal, double lateralError, double ratioLateral, bool tracePosition = false, bool traceVelocity = false, bool center = false);
  void setRobotPosition(int index, double x, double y, bool trace = false);
  void setRobotVelocity(int index, double abscissa, double velocity, bool trace = false, bool center = false);
  void setRobotLongitudinalError(int index, double value, double ratio);
  void setRobotLateralError(int index, double value, double ratio);
  void setRobotError(int index, double longitudinal, double ratioLongitudinal, double lateral, double ratioLateral);

  void updatePlots();

  void settings();
  void simulate();
};

#endif // MAINWINDOW_H
