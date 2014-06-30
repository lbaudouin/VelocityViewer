#ifndef PLOT_H
#define PLOT_H

#include <QApplication>
#include "mainwindow.h"

#include <boost/thread/thread.hpp>

class Plot
{
public:
    Plot();
    ~Plot();

    void load(QString xPathFilename, QString yPathFilename, QString curvatureFilename, QString velocityFilename, bool loop = false);

    void setRobotPositionVelocityError(int index, double x, double y, double abscissa, double velocity, double longitudinalError, double lateralError, bool tracePosition = false, bool traceVelocity = false, bool center = false);
    void setRobotPosition(int index, double x, double y, bool trace = false);
    void setRobotVelocity(int index, double abscissa, double velocity, bool trace = false, bool center = false);
    void setRobotError(int index, double longitudinal, double lateral);
    void setRobotLongitudinalError(int index, double longitudinalError);
    void setRobotLateralError(int index, double lateralError);

protected:
    static void start(int *argc, char **argv, boost::condition_variable *condition);


private:
};

class PlotObject : public QObject
{
    Q_OBJECT
public:
    explicit PlotObject(boost::condition_variable &cond);

    inline bool ready() { return m_ready; }

    inline MainWindow *window() { return m_window; }

protected:
    void timerEvent(QTimerEvent *);
    void customEvent(QEvent *);

private:
    MainWindow *m_window;
    boost::condition_variable &m_cond;
    bool m_ready;
    int m_timer;

signals:
    void loadSignal(QString xPathFilename, QString yPathFilename, QString curvatureFilename, QString velocityFilename, bool loop);
    void setRobotPositionVelocityErrorSignal(int index, double x, double y, double abscissa, double velocity, double longitudinalError, double lateralError, bool tracePosition, bool traceVelocity, bool center);
    void setRobotPositionSignal(int index, double x, double y, bool trace);
    void setRobotVelocitySignal(int index, double abscissa, double velocity, bool trace, bool center);
    void setRobotErrorSignal(int index, double longitudinalError, double lateralError);
    void setRobotLongitudinalErrorSignal(int index, double longitudinalError);
    void setRobotLateralErrorSignal(int index, double lateralError);
    
public slots:
    void load(QString xPathFilename, QString yPathFilename, QString curvatureFilename, QString velocityFilename, bool loop)
    {
        emit loadSignal(xPathFilename,yPathFilename,curvatureFilename,velocityFilename,loop);
    }
    void setRobotPositionVelocityError(int index, double x, double y, double abscissa, double velocity, double longitudinalError, double lateralError, bool tracePosition, bool traceVelocity, bool center)
    {
        emit setRobotPositionVelocityErrorSignal(index,x,y,abscissa,velocity,longitudinalError,lateralError,tracePosition,traceVelocity,center);
    }
    void setRobotPosition(int index, double x, double y, bool trace)
    {
        emit setRobotPositionSignal(index,x,y,trace);
    }
    void setRobotVelocity(int index, double abscissa, double velocity, bool trace, bool center)
    {
        emit setRobotVelocitySignal(index,abscissa,velocity,trace,center);
    }
    void setRobotError(int index, double longitudinalError, double lateralError)
    {
        emit setRobotErrorSignal(index,longitudinalError,lateralError);
    }
    void setRobotLongitudinalError(int index, double longitudinalError)
    {
        emit setRobotLongitudinalErrorSignal(index,longitudinalError);
    }
    void setRobotLateralError(int index, double lateralError)
    {
        emit setRobotLateralErrorSignal(index,lateralError);
    }
    

    void finalize();

};

#endif // PLOT_H
