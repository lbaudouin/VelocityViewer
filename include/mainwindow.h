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
#include <QDir>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  #include <QDesktopWidget>
#else
  #include <QPixmap>
#endif

#include <QDebug>

class MinMaxRange
{
public :
    MinMaxRange() : m_init(false) {}
    MinMaxRange(double value) : m_init(true) {m_min = m_max = value;}

    virtual void clear() { m_init = false; }

    virtual bool addValue(double value){
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
    double min() const {if(m_min>0) return 0.0; else return m_min;}
    double max() const {if(m_max<0) return 0.0; else return m_max;}
    void range(double &vMin, double &vMax) const { vMin = min(); vMax = max(); }

protected:
    bool m_init;
    double m_min, m_max;
};

class AutoMinMaxRange : public MinMaxRange
{
public:
    AutoMinMaxRange(int maxVal = 1000) : MinMaxRange(), m_maxVal(maxVal) { if(maxVal<=0) m_maxVal = 1000; }
    AutoMinMaxRange(double value, int maxVal = 1000) : MinMaxRange(), m_maxVal(maxVal) { addValue(value); if(maxVal<=0) m_maxVal = 1000; }
    
    void setMaximumValue(int maxVal) { if(maxVal<=0) m_maxVal = 1000; else m_maxVal = maxVal; }
    
    void clear() { m_list.clear(); }
  
    bool addValue(double value){
      
	double p_min = m_min;
	double p_max = m_max;
	
	m_list.push_back(value);
	while(m_list.size()>m_maxVal){
	  m_list.pop_front();
	}
	
	m_min = m_max = m_list.at(0);
	for(int i=1; i<m_list.size(); i++){
	  double v = m_list.at(i);
	  if(v>m_max) m_max = v;
	  if(v<m_min) m_min = v;
	}

	if(!m_init){
	  return true;
	}else{
	  if(m_min<p_min || m_max>p_max)
	    return true;
	}
	
	return false;
    }
    
private:
    QList<double> m_list;
    int m_maxVal;
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
    
    bool m_autoReplot;
    
    bool m_changed;

    //Ranges
    MinMaxRange curvatureRange, velocityRange;
    AutoMinMaxRange longitudinalErrorRange, lateralErrorRange;
    
    //Robots
    QMap<int,QGraphicsEllipseItem*> robotEllipseItem;
    QMap<int,QGraphicsLineItem*> robotLineItem;
    QMap<int,QGraphicsPathItem*> robotTraceItem;

    //Bar graphs
    QMap<int,QCPBars*> longitudinalBarGraphs1;
    QMap<int,QCPBars*> longitudinalBarGraphs2;
    QMap<int,QCPBars*> longitudinalBarGraphs3;
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

  void setRobotPositionVelocityError(int index, double x, double y, double abscissa, double velocity, double longitudinalErrorLeader, double longitudinalErrorPreceding, double ratio, double lateralError, bool tracePosition = false, bool traceVelocity = false, bool center = false);
  void setRobotPosition(int index, double x, double y, bool trace = false, bool tracePositionError = false);
  void setRobotVelocity(int index, double abscissa, double velocity, bool trace = false, bool center = false);
  void setRobotLongitudinalError(int index, double longitudinalErrorLeader, double longitudinalErrorPreceding, double ratio);
  void setRobotLateralError(int index, double value, double ratio);
  void setRobotError(int index, double longitudinalErrorLeader, double longitudinalErrorPreceding, double ratio, double lateral);

  void clearPlots();
  void updatePlots();

  void settings();
  void simulate();
};

#endif // MAINWINDOW_H
