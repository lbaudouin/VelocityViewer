#include "plot.h"

static boost::thread thread_;
static boost::mutex mutex_;
static PlotObject *object_ = 0;

Plot::Plot()
{
    int v_argc = 1;
    char *v_argv[] = {(char *) ""};

    boost::mutex::scoped_lock lock(mutex_);
    boost::condition_variable condition;

    if(!qApp){
      thread_ = boost::thread(bind(start, &v_argc, v_argv, &condition));
      while(!qApp){
        condition.wait(lock);
      }
    }
    if(!object_){
      object_ = new PlotObject(condition);
      while(!object_->ready()) {
        condition.wait(lock);
      }
    }
}

Plot::~Plot(){
    thread_.join();
}

void Plot::start(int *argc, char **argv, boost::condition_variable *condition)
{
  boost::mutex::scoped_lock lock(mutex_);
  QApplication app(*argc, argv);
  app.setApplicationName("VelocityProfile");
  condition->notify_one();
  lock.unlock();
  app.exec();
}

void Plot::load(QString xPathFilename, QString yPathFilename, QString curvatureFilename, QString velocityFilename, bool loop)
{
    object_->load(xPathFilename,yPathFilename,curvatureFilename,velocityFilename,loop);
}

void Plot::setRobotPositionVelocityError(int index, double x, double y, double abscissa, double velocity, double error, bool tracePosition, bool traceVelocity, bool center)
{
    object_->setRobotPositionVelocityError(index,x,y,abscissa,velocity,error,tracePosition,traceVelocity,center);
}

void Plot::setRobotPosition(int index, double x, double y, bool trace)
{
    object_->setRobotPosition(index,x,y,trace);
}

void Plot::setRobotVelocity(int index, double abscissa, double velocity, bool trace, bool center)
{
    object_->setRobotVelocity(index,abscissa,velocity,trace,center);
}

void Plot::setRobotError(int index, double value)
{
    object_->setRobotError(index,value);
}




PlotObject::PlotObject(boost::condition_variable &condition) : m_cond(condition), m_ready(false)
{
    moveToThread(qApp->thread());
    setParent(qApp);
    qApp->postEvent(this, new QEvent(QEvent::User));
}

void PlotObject::customEvent(QEvent *)
{
    m_window = new MainWindow;
    m_window->setWindowTitle(qApp->applicationName());
    connect(this,SIGNAL(loadSignal(QString,QString,QString,QString,bool)),m_window,SLOT(load(QString,QString,QString,QString,bool)),Qt::BlockingQueuedConnection);
    connect(this,SIGNAL(setRobotPositionVelocityErrorSignal(int,double,double,double,double,double,bool,bool,bool)),m_window,SLOT(setRobotPositionVelocityError(int,double,double,double,double,double,bool,bool,bool)),Qt::QueuedConnection);
    connect(this,SIGNAL(setRobotPositionSignal(int,double,double,bool)),m_window,SLOT(setRobotPosition(int,double,double,bool)),Qt::QueuedConnection);
    connect(this,SIGNAL(setRobotVelocitySignal(int,double,double,bool,bool)),m_window,SLOT(setRobotVelocity(int,double,double,bool,bool)),Qt::QueuedConnection);
    connect(this,SIGNAL(setRobotErrorSignal(int,double)),m_window,SLOT(setRobotError(int,double)),Qt::QueuedConnection);
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(finalize()));
    m_ready = true;
    m_timer = startTimer(100);
    m_cond.notify_one();
}

void PlotObject::timerEvent(QTimerEvent *)
{
    killTimer(m_timer);
    m_window->show();
}

void PlotObject::finalize()
{
    m_window->deleteLater();
}
