#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), vp(this), m_init(false), m_loop(false), autoReplot(false),
    upRect(0), bottomRect(0), timer(0)
{
    ui->setupUi(this);

    //Create graphics scene
    ui->graphicsView->setScene(new QGraphicsScene);

    //Draw pavin elements
    drawPavin();
    
    //Set up settings
    QSettings settings("VelocityViewer","VelocityViewer");

    this->restoreGeometry(settings.value("ui/geometry").toByteArray());
    this->restoreState(settings.value("ui/state").toByteArray());

    ui->mainSplitter->restoreState(settings.value("splitter/mainSplitterState").toByteArray());
    ui->secondSplitter->restoreState(settings.value("splitter/secondSplitterState").toByteArray());
    ui->graphicsView->setTransform(settings.value("viewport/transform").value<QTransform>());
    ui->zoomSlider->setValue(settings.value("zoom/value",0).toInt());
    
    ui->positionScrollBar->setRange(-1000,1000);

    //Connect actions
    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(close()));
    connect(ui->actionSimulate,SIGNAL(triggered()),this,SLOT(simulate()));
    connect(ui->actionSettings,SIGNAL(triggered()),this,SLOT(settings()));
    
    //Connect zoom
    connect(ui->zoomIn,SIGNAL(clicked()),this,SLOT(zoomIn()));
    connect(ui->zoomOut,SIGNAL(clicked()),this,SLOT(zoomOut()));
    connect(ui->zoomFitBest,SIGNAL(clicked()),this,SLOT(zoomFit()));
    connect(ui->zoomSlider,SIGNAL(valueChanged(int)),this,SLOT(zoom(int)));
    
    //Connect
    connect(ui->positionFitBest,SIGNAL(clicked()),this,SLOT(positionFit()));
    connect(ui->positionScrollBar,SIGNAL(sliderMoved(int)),this,SLOT(positionChanged(int)));
    connect(ui->autoCheckBox,SIGNAL(clicked(bool)),ui->positionScrollBar,SLOT(setDisabled(bool)));
    connect(ui->autoCheckBox,SIGNAL(clicked(bool)),ui->positionFitBest,SLOT(setDisabled(bool)));
    
    //Define colors
    m_colors.clear();
    int size = settings.beginReadArray("colors");
    for (int i = 0; i < size; ++i) {
	settings.setArrayIndex(i);
	m_colors << settings.value("color").value<QColor>();
    }
    settings.endArray();
    if(m_colors.empty())
      m_colors << QColor(150,0,160) << QColor(180,80,0) << QColor(0,170,180) << QColor(Qt::yellow) << QColor(Qt::black) << QColor(Qt::red) << QColor(Qt::green) << QColor(Qt::blue);

    longitudinalErrorRange.addValue(0);
    lateralErrorRange.addValue(0);
    
}

MainWindow::~MainWindow()
{
    if(timer)
        timer->stop();

    QSettings settings("VelocityViewer","VelocityViewer");
    settings.setValue("ui/geometry",this->saveGeometry());
    settings.setValue("ui/state",this->saveState());
    settings.setValue("splitter/mainSplitterState",ui->mainSplitter->saveState());
    settings.setValue("splitter/secondSplitterState",ui->secondSplitter->saveState());
    settings.setValue("viewport/transform",ui->graphicsView->transform());
    settings.setValue("zoom/value",ui->zoomSlider->value());
    
    settings.beginWriteArray("colors");
    for(int i=0;i<m_colors.size();i++){
      settings.setArrayIndex(i);
      settings.setValue("color", m_colors.at(i));
    }
    settings.endArray();

    delete ui;
}

bool MainWindow::load(QString xPathFilename, QString yPathFilename, QString curvatureFilename, QString velocityFilename, bool loop)
{
    //Load data;
    if(!vp.loadPath(xPathFilename,yPathFilename))
        return false;
    if(!vp.loadCurvature(curvatureFilename))
        return false;
    if(!vp.loadVelocityProfile(velocityFilename))
        return false;

    m_loop = loop;

    if(!vp.isValid()){
        qWarning() << "Invalid dataset: lengths are not egual";
        return false;
    }

    m_init = true;

    //Create plots
    setupPlot();

    //Draw path on graph
    drawPath();

    return true;
}

QList<QPointF> MainWindow::loadPavinElement(QString filename)
{
    QList<QPointF> list;
    QPointF offset(660000,84700);

    //Open file
    QFile file(filename);
    if(!file.exists()){
        //QMessageBox::critical(m_parent,QString("Error"),QString("File '%1' doesn't exists").arg(curvatureFilename));
        return list;
    }
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        //QMessageBox::critical(m_parent,QString("Error"),QString("Can't read '%1'").arg(curvatureFilename));
        return list;
    }

    QTextStream stream(&file);
    QString line;
    do{
        line = stream.readLine();
        if(line.isEmpty()) break;
        QTextStream lineStream(&line);

        double x,y;

        lineStream >> x >> y;

        //Remove offset, inverse y axis to display, apply factor
        QPointF pt = 10.0* (QPointF(x,y) - offset);
        pt.ry() *= -1.0;
        list.push_back( pt );

    }while(!line.isEmpty());


    return list;
}


void MainWindow::drawPavinElements(QString filebase, int max, QPen pen, QBrush brush)
{
    for(int i=1;i<=max;i++){
        QList<QPointF> points = loadPavinElement( filebase.arg(i) );
        if(points.isEmpty())
            continue;

        QPainterPath path(points[0]);
        for(int k=1;k<points.size();k++)
            path.lineTo(points[k]);

        ui->graphicsView->scene()->addPath(path,pen,brush);
    }
}

void MainWindow::drawPavin()
{
#if 0 //If color else gray
    drawPavinElements(":/pavin/contour_route_%1",     8,  QPen(Qt::black,2));
    drawPavinElements(":/pavin/grillage_%1",          5,  QPen(Qt::red,2));
    drawPavinElements(":/pavin/hall_%1",              1,  QPen(Qt::darkYellow,2),     QBrush(QColor(255,255,200)));
    drawPavinElements(":/pavin/ligne_blanche_%1",     33, QPen(QColor(200,200,200)),  QBrush(QColor(225,225,225)));
    drawPavinElements(":/pavin/parking_%1",           2,  QPen(QColor(200,200,200)),  QBrush(QColor(225,225,225)));
    drawPavinElements(":/pavin/passage_pieton_%1",    17, QPen(QColor(200,200,230)),  QBrush(QColor(225,225,255)));
    drawPavinElements(":/pavin/porte_%1",             4,  QPen(Qt::green,2),          QBrush(QColor(200,225,200)));
    drawPavinElements(":/pavin/rond_point_%1",        1,  QPen(Qt::magenta,2),        QBrush(QColor(255,200,200)));
    drawPavinElements(":/pavin/separation_stop_%1",   1,  QPen(Qt::red,2),            QBrush(QColor(255,200,200)));
    drawPavinElements(":/pavin/stop_%1",              1,  QPen(Qt::red,2),            QBrush(QColor(255,200,200)));
    drawPavinElements(":/pavin/tour_bloc_%1",         6,  QPen(Qt::darkBlue,2),       QBrush(QColor(200,200,255)));
    drawPavinElements(":/pavin/trottoir_%1",          5,  QPen(Qt::magenta,2),        QBrush(QColor(255,200,200)));
    drawPavinElements(":/pavin/verdure_%1",           2,  QPen(Qt::darkGreen,2),      QBrush(QColor(200,255,200)));
    drawPavinElements(":/pavin/bloc_%1",              9,  QPen(Qt::gray,2),           QBrush(QColor(200,200,200)));
#else
    drawPavinElements(":/pavin/contour_route_%1",     8,  QPen(Qt::gray,2));
    drawPavinElements(":/pavin/grillage_%1",          5,  QPen(Qt::gray,2));
    drawPavinElements(":/pavin/hall_%1",              1,  QPen(Qt::gray,2),     QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/ligne_blanche_%1",     33, QPen(Qt::gray),       QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/parking_%1",           2,  QPen(Qt::gray),       QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/passage_pieton_%1",    17, QPen(Qt::gray),       QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/porte_%1",             4,  QPen(Qt::gray,2),     QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/rond_point_%1",        1,  QPen(Qt::gray,2),     QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/separation_stop_%1",   1,  QPen(Qt::gray,2),     QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/stop_%1",              1,  QPen(Qt::gray,2),     QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/tour_bloc_%1",         6,  QPen(Qt::gray,2),     QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/trottoir_%1",          5,  QPen(Qt::gray,2),     QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/verdure_%1",           2,  QPen(Qt::gray,2),     QBrush(Qt::lightGray));
    drawPavinElements(":/pavin/bloc_%1",              9,  QPen(Qt::gray,2),     QBrush(Qt::lightGray));
#endif
}

void MainWindow::drawPath()
{
    if(!m_init) return;

    QList<QPointF> startPoints = vp.startPoints();
    QPainterPath path;

    if(startPoints.size()>0){
        QPointF pos = 10.0 * startPoints[0];
        //Inverse y axis to display
        pos.ry() *= -1.0;
        path.moveTo( pos);
    }

    double smax = vp.length();
    double step = 0.01;
    for(double s=step;s<smax;s+=step){
        QPointF pos = 10 * vp.path(s);
        //Inverse y axis to display
        pos.ry() *= -1.0;
        path.lineTo( pos );
    }

    ui->graphicsView->scene()->addPath(path,QPen(Qt::green,2));

    for(int i=0;i<startPoints.size();i++){
        if(i==0){
            QPointF p1 = 10.0 * startPoints[0];
            QPointF p2 = 10.0 * vp.path(0.001);
            //Inverse y axis to display
            p1.ry() *= -1.0;
            p2.ry() *= -1.0;

            QPointF v = (p2-p1)/(QLineF(p1,p2).length());
            QPointF n(-v.y(),v.x());

            double size = 5;
            QPolygonF arrow;
            arrow << p1 << p1 + size*n - size*v << p1 + 2.0*size*v;
            arrow << p1 - size*n - size*v << p1;

            ui->graphicsView->scene()->addPolygon(arrow,QPen(Qt::blue),QBrush(Qt::blue));
        }else{
            QGraphicsEllipseItem *item = ui->graphicsView->scene()->addEllipse(-3,-3,6,6,QPen(Qt::blue),QBrush(Qt::blue));
            QPointF pos = 10.0 * startPoints[i];
            //Inverse y axis to display
            pos.ry() *= -1.0;
            item->setPos( pos );
        }
    }
    
    //ui->graphicsView->scene()->addRect(path.boundingRect());
    //ui->graphicsView->fitInView(path.boundingRect(),Qt::KeepAspectRatio);
}

void MainWindow::setupPlot()
{
    //Clear main plot
    ui->mainPlot->clearGraphs();
    ui->mainPlot->clearItems();
    ui->mainPlot->plotLayout()->clear();
    ui->mainPlot->setAutoAddPlottableToLegend(false);

    //Read maximun abscissa
    double smax = vp.length();

    //Create subplot
    upRect = new QCPAxisRect(ui->mainPlot);
    bottomRect = new QCPAxisRect(ui->mainPlot);
    
    connect(upRect->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

    //Define subplot
    upRect->setupFullAxesBox(true);
    bottomRect->setupFullAxesBox(true);
    upRect->axis(QCPAxis::atBottom)->grid()->setPen(Qt::NoPen);
    bottomRect->axis(QCPAxis::atBottom)->grid()->setPen(Qt::NoPen);

    QCPAxis *upperBlockAxis = upRect->addAxis(QCPAxis::atLeft);
    upperBlockAxis->setVisible(false);
    upperBlockAxis->setRange(0,1);

    QCPAxis *lowerBlockAxis = bottomRect->addAxis(QCPAxis::atLeft);
    lowerBlockAxis->setVisible(false);
    lowerBlockAxis->setRange(0,1);

    //Add subplot to main plot
    ui->mainPlot->plotLayout()->addElement(0,0,upRect);
    ui->mainPlot->plotLayout()->addElement(1,0,bottomRect);

    //Create and attach legen to upper subplot
    QCPLegend *legend1 = new QCPLegend;
    upRect->insetLayout()->addElement(legend1, Qt::AlignRight|Qt::AlignTop);
    legend1->setLayer("legend");
    legend1->setVisible(true);
    QFont font = legend1->font();
    font.setPointSize(8);
    legend1->setFont(font);
    legend1->setRowSpacing(-5);
    legend1->setBrush(QBrush(QColor(255,255,255,200)));

    //Create and attach legen to lower subplot
    QCPLegend *legend2 = new QCPLegend;
    bottomRect->insetLayout()->addElement(legend2, Qt::AlignRight|Qt::AlignTop);
    legend2->setLayer("legend");
    legend2->setVisible(true);
    legend2->setFont(font);
    legend2->setRowSpacing(-5);
    legend2->setBrush(QBrush(QColor(255,255,255,200)));

    //Align subplots
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->mainPlot);
    upRect->setMarginGroup(QCP::msLeft, marginGroup);
    bottomRect->setMarginGroup(QCP::msLeft, marginGroup);
    //upRect->setMarginGroup(QCP::msRight, marginGroup);
    //bottomRect->setMarginGroup(QCP::msRight, marginGroup);

    //Retreive informations
    QList<ModeInfo> modes = vp.modes();
    QList<CurveInfo> infos = vp.curveInfos();

    //Draw begin and end
    for(int k=-1;k<=1;k++){
        if(!m_loop && k!=0)
            continue;

        //Upper gaprh
        {
            QCPItemLine *line = new QCPItemLine(ui->mainPlot);
            QPen pen(Qt::darkGray,2);
            pen.setStyle(Qt::DotLine);
            line->setPen(pen);
            line->start->setAxes(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft,1));
            line->end->setAxes(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft,1));
            line->start->setCoords(k*smax,0);
            line->end->setCoords(k*smax,1);
            line->setClipAxisRect(upRect);
        }
        //Lower graph
        {
            QCPItemLine *line = new QCPItemLine(ui->mainPlot);
            QPen pen(Qt::darkGray,2);
            pen.setStyle(Qt::DotLine);
            line->setPen(pen);
            line->start->setAxes(bottomRect->axis(QCPAxis::atBottom), bottomRect->axis(QCPAxis::atLeft,1));
            line->end->setAxes(bottomRect->axis(QCPAxis::atBottom), bottomRect->axis(QCPAxis::atLeft,1));
            line->start->setCoords(k*smax,0);
            line->end->setCoords(k*smax,1);
            line->setClipAxisRect(bottomRect);
        }
    }


    //Plot upper curves
    bool oddCurveLegendAdded = false;
    bool evenCurveLegendAdded = false;

    for(int k=-1;k<=1;k++){
        if(!m_loop && k!=0)
            continue;

        for(int i=0;i<infos.size();i++){

            ui->mainPlot->addGraph(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft));
            if(infos[i].index%2){
                ui->mainPlot->graph()->setPen(QPen(Qt::green,3));
                ui->mainPlot->graph()->setName("odd index curves");
                if(!oddCurveLegendAdded){
                    legend1->addItem( new QCPPlottableLegendItem(legend1,ui->mainPlot->graph()));
                    oddCurveLegendAdded = true;
                }
            }else{
                ui->mainPlot->graph()->setPen(QPen(Qt::black,3));
                ui->mainPlot->graph()->setName("even index curves");
                if(!evenCurveLegendAdded){
                    legend1->addItem( new QCPPlottableLegendItem(legend1,ui->mainPlot->graph()));
                    evenCurveLegendAdded = true;
                }
            }

            double step = 0.1;
            QVector<double> x,y;
            for(double s=infos[i].s0;s<infos[i].s1;s+=step){
                double value = vp.curvature(s);
                x << s + k*smax;
                y << value;
                curvatureRange.addValue(value);
            }

            //Add first value of next curve
            double value = vp.curvature(infos[(i+1)%infos.size()].s0);
            x << infos[i].s1 + k*smax;
            y << value;
            curvatureRange.addValue(value);

            ui->mainPlot->graph()->setData(x, y);
        }
    }
    upRect->axis(QCPAxis::atLeft)->setRange(curvatureRange.min() - 0.1*fabs(curvatureRange.min()) - 0.1*(curvatureRange.max()-curvatureRange.min())/0.9,
                                                curvatureRange.max() + 0.1*fabs(curvatureRange.max()));


    //Plot lower curves
    bool legendMaxVelAdded = false;
    bool legendLatAccAdded = false;
    bool legendLonAccAdded = false;

    for(int k=-1;k<=1;k++){
        if(!m_loop && k!=0)
            continue;

        for(int i=0;i<modes.size();i++){

            ui->mainPlot->addGraph(bottomRect->axis(QCPAxis::atBottom), bottomRect->axis(QCPAxis::atLeft));
            VelocityData::VelocityMode currentMode = modes[i].mode;
            switch(currentMode){
                case VelocityData::MaxVel:{
                    ui->mainPlot->graph()->setPen(QPen(Qt::blue,3));
                    ui->mainPlot->graph()->setName("MaxVel");
                    if(!legendMaxVelAdded){
                        legend2->addItem( new QCPPlottableLegendItem(legend2,ui->mainPlot->graph()));
                        legendMaxVelAdded = true;
                    }
                    break;
                }
                case VelocityData::LatAcc:{
                    ui->mainPlot->graph()->setPen(QPen(Qt::black,3));
                    ui->mainPlot->graph()->setName("LatAcc");
                    if(!legendLatAccAdded){
                        legend2->addItem( new QCPPlottableLegendItem(legend2,ui->mainPlot->graph()));
                        legendLatAccAdded = true;
                    }
                    break;
                }
                case VelocityData::LonAcc:{
                    ui->mainPlot->graph()->setPen(QPen(Qt::red,3));
                    ui->mainPlot->graph()->setName("LonAcc");
                    if(!legendLonAccAdded){
                        legend2->addItem( new QCPPlottableLegendItem(legend2,ui->mainPlot->graph()));
                        legendLonAccAdded = true;
                    }
                    break;
                }
                default:
                    ui->mainPlot->graph()->setPen(QPen(Qt::magenta,2));
            }

            QVector<double> x,y;
            double step = 0.1;

            for(double s=modes[i].s0;s<modes[i].s1;s+=step){
                double value = vp.velocity(s);
                x << s + k*smax;
                y << value;
                velocityRange.addValue(value);
            }

            //Add first value of next curve
            double value = vp.velocity(modes[(i+1)%modes.size()].s0);
            x << modes[i].s1 + k*smax;
            y << value;
            velocityRange.addValue(value);

            ui->mainPlot->graph()->setData(x, y);
        }
    }

    //Add 10% over and under the curve, and 10% of total more under for items
    bottomRect->axis(QCPAxis::atLeft)->setRange(velocityRange.min() - 0.1*fabs(velocityRange.min()) - 0.1*(velocityRange.max()-velocityRange.min())/0.9,
                                                velocityRange.max() + 0.1*fabs(velocityRange.max()));

    //Add color block on lower subplot
    for(int k=-1;k<=1;k++){
        if(!m_loop && k!=0)
            continue;

        for(int i=0;i<modes.size();i++){
            QCPItemRect *rect = new QCPItemRect(ui->mainPlot);
            //rect->topLeft->setType(QCPItemPosition::ptAxisRectRatio );
            //rect->bottomRight->setType(QCPItemPosition::ptAxisRectRatio );
            rect->topLeft->setAxes(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft,1));
            rect->bottomRight->setAxes(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft,1));
            rect->topLeft->setAxisRect(upRect);
            rect->bottomRight->setAxisRect(upRect);

            VelocityData::VelocityMode mode = modes[i].mode;

            QColor color;
            switch(mode){
                case VelocityData::MaxVel: color = QColor(Qt::blue); break;
                case VelocityData::LonAcc: color = QColor(Qt::red); break;
                case VelocityData::LatAcc: color = QColor(Qt::black); break;
                default: color = QColor(Qt::transparent);
            }

            rect->setPen(QPen(color));
            rect->setBrush(QBrush(color));

            rect->topLeft->setCoords(modes[i].s0+k*smax,0);
            rect->bottomRight->setCoords(modes[i].s1+k*smax,0.1);

            if(i>0){
                if(modes.at(i).mode!=modes.at(i-1).mode){
                    QCPItemLine *line = new QCPItemLine(ui->mainPlot);
                    QPen pen(Qt::gray);
                    pen.setStyle(Qt::DotLine);
                    line->setPen(pen);
                    line->start->setAxes(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft,1));
                    line->end->setAxes(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft,1));
                    line->start->setCoords(modes[i].s0+k*smax,0);
                    line->end->setCoords(modes[i].s0+k*smax,1);
                    line->setClipAxisRect(upRect);
                }
            }else{
                QCPItemLine *line = new QCPItemLine(ui->mainPlot);
                QPen pen(Qt::gray);
                pen.setStyle(Qt::DotLine);
                line->setPen(pen);
                line->start->setAxes(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft,1));
                line->end->setAxes(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft,1));
                line->start->setCoords(modes[i].s0+k*smax,0);
                line->end->setCoords(modes[i].s0+k*smax,1);
                line->setClipAxisRect(upRect);
            }
        }
    }

    //Add color block on lower subplot
    for(int k=-1;k<=1;k++){
        if(!m_loop && k!=0)
            continue;

        for(int i=0;i<infos.size();i++){
            QCPItemRect *rect = new QCPItemRect(ui->mainPlot);
            rect->topLeft->setAxes(bottomRect->axis(QCPAxis::atBottom), bottomRect->axis(QCPAxis::atLeft,1));
            rect->bottomRight->setAxes(bottomRect->axis(QCPAxis::atBottom), bottomRect->axis(QCPAxis::atLeft,1));
            rect->topLeft->setAxisRect(bottomRect);
            rect->bottomRight->setAxisRect(bottomRect);
            rect->setClipAxisRect(bottomRect);

            int index = infos[i].index;

            if( index % 2 ){
                rect->setPen(QPen(Qt::green));
                rect->setBrush(QBrush(Qt::green));
            }else{
                rect->setPen(QPen(Qt::black));
                rect->setBrush(QBrush(Qt::black));
            }
            rect->topLeft->setCoords(infos[i].s0+k*smax,0);
            rect->bottomRight->setCoords(infos[i].s1+k*smax,0.1);

            QCPItemLine *line = new QCPItemLine(ui->mainPlot);
            QPen pen(Qt::gray);
            pen.setStyle(Qt::DotLine);
            line->setPen(pen);
            line->start->setAxes(bottomRect->axis(QCPAxis::atBottom), bottomRect->axis(QCPAxis::atLeft));
            line->end->setAxes(bottomRect->axis(QCPAxis::atBottom), bottomRect->axis(QCPAxis::atLeft));
            line->start->setCoords(infos[i].s1+k*smax,-10);
            line->end->setCoords(infos[i].s1+k*smax,10);
            line->setClipAxisRect(bottomRect);
        }
    }

    //Set axis titles and ranges
    double xAxisSize = smax * (1.0 - ui->zoomSlider->value()/100.0);
    upRect->axis(QCPAxis::atBottom)->setLabel("curvilinear abscissa (m)");
    upRect->axis(QCPAxis::atLeft)->setLabel("curvature (1/m)");
    upRect->axis(QCPAxis::atBottom)->setRange(0,xAxisSize);

    bottomRect->axis(QCPAxis::atBottom)->setLabel("curvilinear abscissa (m)");
    bottomRect->axis(QCPAxis::atLeft)->setLabel("desired velocity (m/s)");
    bottomRect->axis(QCPAxis::atBottom)->setRange(0, xAxisSize);

    //Set axis titles
    ui->longitudinalErrorPlot->xAxis->setLabel("robot index");
    ui->longitudinalErrorPlot->yAxis->setLabel("longitudinal error (m)");
    ui->longitudinalErrorPlot->xAxis->setTicks(false);
    ui->longitudinalErrorPlot->xAxis->setRange(0,1);
    ui->longitudinalErrorPlot->yAxis->setRange(-1.0,1.0);
    
    //Set longitudinal legend
    {
        QCPBars* bar = new QCPBars(ui->longitudinalErrorPlot->xAxis, ui->longitudinalErrorPlot->yAxis);
        bar->setPen(QPen(Qt::black));
        bar->setBrush(QColor(Qt::gray).dark());
        bar->setAntialiased(false);
        bar->setAntialiasedFill(false);
	bar->setName("Leader");
	ui->longitudinalErrorPlot->addPlottable(bar);
    }
    {
        QCPBars* bar = new QCPBars(ui->longitudinalErrorPlot->xAxis, ui->longitudinalErrorPlot->yAxis);
        bar->setPen(QPen(Qt::black));
        bar->setBrush(QColor(Qt::gray));
        bar->setAntialiased(false);
        bar->setAntialiasedFill(false);
	bar->setName("Preceding");
	ui->longitudinalErrorPlot->addPlottable(bar);
    }
    {
        QCPBars* bar = new QCPBars(ui->longitudinalErrorPlot->xAxis, ui->longitudinalErrorPlot->yAxis);
        bar->setPen(QPen(Qt::black));
        bar->setBrush(QColor(Qt::gray).light());
        bar->setAntialiased(false);
        bar->setAntialiasedFill(false);
	bar->setName("Composite");
	ui->longitudinalErrorPlot->addPlottable(bar);
    }
    //Create and attach legen to upper subplot
    QCPLegend *legend = ui->longitudinalErrorPlot->legend;
    legend->setFont(font);
    legend->setRowSpacing(-8);
    legend->setAutoMargins(QCP::msNone);
    legend->setMargins(QMargins(0,0,0,0));
    legend->setBrush(QBrush(QColor(255,255,255,200)));
    
    ui->longitudinalErrorPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignTop);
    ui->longitudinalErrorPlot->legend->setVisible(true);
    
    ui->lateralErrorPlot->xAxis->setLabel("robot index");
    ui->lateralErrorPlot->yAxis->setLabel("lateral error (m)");
    ui->lateralErrorPlot->xAxis->setTicks(false);
    ui->lateralErrorPlot->xAxis->setRange(0,1);
    ui->lateralErrorPlot->yAxis->setRange(-1.0,1.0);

    updatePlots();
}

void MainWindow::updatePlots()
{
    ui->mainPlot->replot();
    ui->longitudinalErrorPlot->replot();
    ui->lateralErrorPlot->replot();
}

void MainWindow::setRobotPositionVelocityError(int index, double x, double y, double abscissa, double velocity, double longitudinalErrorLeader, double longitudinalErrorPreceding, double ratio, double lateralError, bool tracePosition, bool traceVelocity, bool center)
{
    setRobotPosition(index,x,y,tracePosition);
    setRobotVelocity(index,abscissa,velocity,traceVelocity,center);
    setRobotError(index,longitudinalErrorLeader,longitudinalErrorPreceding,ratio,lateralError);
}


void MainWindow::setRobotPosition(int index, double x, double y, bool trace)
{
    if(!m_init || index<0) return;

    //Select color
    QColor color = m_colors.at((index-1)%m_colors.size());

    //Find curvilinear abscissa
    QPair<int,double> s = vp.findClosestPoint(x,y);

    //Create robot position
    QPointF robotRealPosition(x,y);
    QPointF robotDesiredPosition(vp.path(s.first,s.second));

    //Inverse y to display
    robotRealPosition.ry() *= -1.0;
    robotDesiredPosition.ry() *= -1.0;

    //Draw ellipse to represent robot position
    if(!robotEllipseItem.contains(index)){
        robotEllipseItem[index] = ui->graphicsView->scene()->addEllipse(QRect(-5,-5,10,10),QPen(Qt::black),QBrush(color));
        robotEllipseItem[index]->setPos(10*robotRealPosition);
    }else{
        robotEllipseItem[index]->setPos(10*robotRealPosition);
    }

    //Draw line betwwen robot and projected point on path
    if(!robotLineItem.contains(index)){
        robotLineItem[index] = ui->graphicsView->scene()->addLine(QLineF(10*robotRealPosition, 10*robotDesiredPosition),QPen(color));
    }else{
        robotLineItem[index]->setLine( QLineF(10*robotRealPosition, 10*robotDesiredPosition) );
    }

    if(trace){
        //Draw robot trace
        if(!robotTraceItem.contains(index)){
            robotTraceItem[index] = ui->graphicsView->scene()->addPath(QPainterPath(),QPen(color),QBrush(Qt::transparent));
            QPainterPath path = robotTraceItem[index]->path();
            path.moveTo(10*robotRealPosition);
            robotTraceItem[index]->setPath(path);
        }else{
            QPainterPath path = robotTraceItem[index]->path();
            path.lineTo(10*robotRealPosition);
            robotTraceItem[index]->setPath(path);
        }
    }
}

void MainWindow::setRobotError(int index, double longitudinalErrorLeader, double longitudinalErrorPreceding, double ratio, double lateral)
{
    setRobotLongitudinalError(index,longitudinalErrorLeader,longitudinalErrorPreceding,ratio);
    setRobotLateralError(index,lateral,1.0);
}


void MainWindow::setRobotVelocity(int index, double abscissa, double velocity, bool trace, bool center)
{
    if(!m_init || index<0) return;

    //Select color
    QColor color = m_colors.at((index-1)%m_colors.size());

    //Check abscissa validity
    double smax = vp.length();
    if(m_loop){
      while(abscissa<0.0)
	abscissa += smax;
      while(abscissa>=smax)
	abscissa -= smax;
      
      //Check if the plot need to be cleared
      if(errorCurveGraph.contains(index)){
	double last = errorCurveGraph[index]->data()->keys().last();
	if(abscissa<last-smax*0.75){
	  //QList<double> data_x = errorCurveGraph[index]->data()->keys();
	  //for(int i=0;i<data_x.size();i++)
	  //  if(data_x.at(i) < abscissa + 0.1*smax)
	  //    errorCurveGraph[index]->data()->remove(data_x.at(i));
	  
	  errorCurveGraph[index]->clearData();
	}
      }
      
    }else{
      if(abscissa<0){
	qWarning() << "Invalid abscissa (<0)";
	return;
      }
      if(abscissa>=smax){
	qWarning() << "Invalid abscissa (>=smax)";
	return;
      }
    }
    
    //Compute desired velocity
    double desiredVelocity = vp.velocity(abscissa);

    //Create point on curvature subplot
    if(!positionGraph.contains(index)){
        positionGraph[index] = ui->mainPlot->addGraph(upRect->axis(QCPAxis::atBottom), upRect->axis(QCPAxis::atLeft));
        positionGraph[index]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1.5), QBrush(color), 9));
        positionGraph[index]->setPen(QPen(Qt::gray, 2));
    }

    //Set curvature points
    positionGraph[index]->setData(QVector<double>() << abscissa,
                                  QVector<double>() << vp.curvature(abscissa));

    //Create line between real velocity and desired velocity
    if(!errorGraph.contains(index)){
        errorGraph[index] = ui->mainPlot->addGraph(bottomRect->axis(QCPAxis::atBottom), bottomRect->axis(QCPAxis::atLeft));
        errorGraph[index]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1.5), QBrush(color), 9));
        errorGraph[index]->setPen(QPen(Qt::gray, 2));
    }

    //Set velocity values
    errorGraph[index]->setData(QVector<double>() << abscissa << abscissa,
                               QVector<double>() << desiredVelocity << velocity);

    //Reset range
    if(velocityRange.addValue(velocity)){
        bottomRect->axis(QCPAxis::atLeft)->setRange(velocityRange.min() - 0.1*fabs(velocityRange.min()) - 0.1*(velocityRange.max()-velocityRange.min())/0.9,
                                                    velocityRange.max() + 0.1*fabs(velocityRange.max()));
    }

    if(trace){
        //Create error graph
        if(!errorCurveGraph.contains(index)){
            errorCurveGraph[index] = ui->mainPlot->addGraph(bottomRect->axis(QCPAxis::atBottom), bottomRect->axis(QCPAxis::atLeft));
            errorCurveGraph[index]->setPen(QPen(color, 2));
        }
        errorCurveGraph[index]->addData(abscissa,velocity);
    }


    //Center graph on current abscissa
    if(center && ui->autoCheckBox->isChecked()){
        double size = upRect->axis(QCPAxis::atBottom)->range().size();
        upRect->axis(QCPAxis::atBottom)->setRange(abscissa,size,Qt::AlignCenter);
        bottomRect->axis(QCPAxis::atBottom)->setRange(abscissa,size,Qt::AlignCenter);
    }

    //Force replot
    if(autoReplot)
      ui->mainPlot->replot(QCustomPlot::rpQueued);
}

void MainWindow::setRobotLongitudinalError(int index, double valueLeader, double valuePreceding, double ratio)
{
    if(!m_init || index<0) return;

    ratio = fabs(ratio);
    
    //Select color
    QColor color = m_colors.at((index-1)%m_colors.size());

    bool resetTicks = false;
    
    //Create new graph
    if(!longitudinalBarGraphs1.contains(index)){
        QCPBars* bar = new QCPBars(ui->longitudinalErrorPlot->xAxis, ui->longitudinalErrorPlot->yAxis);
        longitudinalBarGraphs1.insert(index,bar);

        //Set bar properties
        bar->setPen(QPen(Qt::black));
        bar->setBrush(QBrush(color.dark()));
        bar->setAntialiased(false);
        bar->setAntialiasedFill(false);
        bar->keyAxis()->setAutoTicks(false);
        bar->keyAxis()->setSubTickCount(0);

	resetTicks = true;
    }
    if(!longitudinalBarGraphs2.contains(index)){
        QCPBars* bar = new QCPBars(ui->longitudinalErrorPlot->xAxis, ui->longitudinalErrorPlot->yAxis);
        longitudinalBarGraphs2.insert(index,bar);
	//longitudinalBarGraphs2[index]->moveAbove(longitudinalBarGraphs1[index]);

        //Set bar properties
        bar->setPen(QPen(Qt::black));
        bar->setBrush(QBrush(color));
        bar->setAntialiased(false);
        bar->setAntialiasedFill(false);
        bar->keyAxis()->setAutoTicks(false);
        bar->keyAxis()->setSubTickCount(0);

	resetTicks = true;
    }
    if(!longitudinalBarGraphs3.contains(index)){
        QCPBars* bar = new QCPBars(ui->longitudinalErrorPlot->xAxis, ui->longitudinalErrorPlot->yAxis);
        longitudinalBarGraphs3.insert(index,bar);
	//longitudinalBarGraphs3[index]->moveAbove(longitudinalBarGraphs2[index]);

        //Set bar properties
        bar->setPen(QPen(Qt::black));
        bar->setBrush(QBrush(color.light()));
        bar->setAntialiased(false);
        bar->setAntialiasedFill(false);
        bar->keyAxis()->setAutoTicks(false);
        bar->keyAxis()->setSubTickCount(0);

	resetTicks = true;
    }
    
    if(resetTicks){
      //Update list of ticks
        QList<int> keys = longitudinalBarGraphs1.keys();
        QVector<double> keysDouble;
        QVector<QString> keysLabel;
        foreach (int k, keys) {
	    keysDouble << k;
            keysLabel << "Veh " + QString::number(k);
        }
	ui->longitudinalErrorPlot->xAxis->setAutoTicks(false);
	ui->longitudinalErrorPlot->xAxis->setAutoTickLabels(false);
        ui->longitudinalErrorPlot->xAxis->setTickVector(keysDouble);
        ui->longitudinalErrorPlot->xAxis->setTickVectorLabels(keysLabel);
        ui->longitudinalErrorPlot->xAxis->setRange(0,keys.last()+1);
        ui->longitudinalErrorPlot->xAxis->setTicks(true);
        ui->longitudinalErrorPlot->xAxis->setTickLabels(true);
    }
    
    
    /*if(!longitudinalErrorValue.contains(index)){
      QCPItemText *text = new QCPItemText(ui->longitudinalErrorPlot);
      ui->longitudinalErrorPlot->addItem(text);
      text->setFont(QFont(font().family(), 9));
      text->setPadding(QMargins(8, 0, 0, 0));
      longitudinalErrorValue[index] = text;
    }*/

    //Set bar data
    longitudinalBarGraphs1[index]->setWidth(0.3);
    longitudinalBarGraphs1[index]->setData(QVector<double>() << index-0.3,
					  QVector<double>() << valueLeader);
    
    //Set bar data
    longitudinalBarGraphs2[index]->setWidth(0.3);
    longitudinalBarGraphs2[index]->setData(QVector<double>() << index,
					   QVector<double>() << valuePreceding);
    
    
    //Set bar data
    longitudinalBarGraphs3[index]->setWidth(0.3);
    longitudinalBarGraphs3[index]->setData(QVector<double>() << index+0.3,
					   QVector<double>() << ratio*valueLeader + (1.0-ratio)*valuePreceding);

    //Set ratio text
    /*longitudinalErrorValue[index]->setText( QString::number(ratio,'g',2) );
    if(value>=0)
      longitudinalErrorValue[index]->position->setCoords(index,value+0.1);
    else
      longitudinalErrorValue[index]->position->setCoords(index,value-0.1);*/
    
    //Reset range
    bool rangeChanged1 = longitudinalErrorRange.addValue(valueLeader);
    bool rangeChanged2 = longitudinalErrorRange.addValue(valuePreceding);
    bool rangeChanged3 = longitudinalErrorRange.addValue(ratio*valueLeader + (1.0-ratio)*valuePreceding);
    
    if(rangeChanged1 || rangeChanged2 || rangeChanged3){
        ui->longitudinalErrorPlot->yAxis->setRange(longitudinalErrorRange.min()-0.1*fabs(longitudinalErrorRange.min()),
						   longitudinalErrorRange.max()+0.1*fabs(longitudinalErrorRange.max()));
    }

    //Force replot
    if(autoReplot)
      ui->longitudinalErrorPlot->replot(QCustomPlot::rpQueued);
}

void MainWindow::setRobotLateralError(int index, double value, double ratio)
{
    if(!m_init || index<0) return;

    ratio = fabs(ratio);
    
    //Select color
    QColor color = m_colors.at((index-1)%m_colors.size());

    bool resetTicks = false;
    
    //Create new graph
    if(!lateralBarGraphs.contains(index)){
        QCPBars* bar = new QCPBars(ui->lateralErrorPlot->xAxis, ui->lateralErrorPlot->yAxis);
        lateralBarGraphs.insert(index,bar);

        //Set bar properties
        bar->setPen(QPen(Qt::black));
        bar->setBrush(QBrush(color));
        bar->setAntialiased(false);
        bar->setAntialiasedFill(false);
        bar->keyAxis()->setAutoTicks(false);
        bar->keyAxis()->setSubTickCount(0);

	resetTicks = true;
    }
    if(!lateralBarGraphs2.contains(index)){
        QCPBars* bar = new QCPBars(ui->lateralErrorPlot->xAxis, ui->lateralErrorPlot->yAxis);
        lateralBarGraphs2.insert(index,bar);
	lateralBarGraphs2[index]->moveAbove(lateralBarGraphs[index]);

        //Set bar properties
        bar->setPen(QPen(Qt::black));
        bar->setBrush(QBrush(color.light()));
        bar->setAntialiased(false);
        bar->setAntialiasedFill(false);
        bar->keyAxis()->setAutoTicks(false);
        bar->keyAxis()->setSubTickCount(0);

	resetTicks = true;
    }
    
    if(resetTicks){
      //Update list of ticks
        QList<int> keys = lateralBarGraphs.keys();
        QVector<double> keysDouble;
        QVector<QString> keysLabel;
        foreach (int k, keys) {
	    keysDouble << k;
            keysLabel << "Veh " + QString::number(k);
        }
	ui->lateralErrorPlot->xAxis->setAutoTicks(false);
	ui->lateralErrorPlot->xAxis->setAutoTickLabels(false);
        ui->lateralErrorPlot->xAxis->setTickVector(keysDouble);
        ui->lateralErrorPlot->xAxis->setTickVectorLabels(keysLabel);
        ui->lateralErrorPlot->xAxis->setRange(0,keys.last()+1);
        ui->lateralErrorPlot->xAxis->setTicks(true);
        ui->lateralErrorPlot->xAxis->setTickLabels(true);
    }
    
    /*if(!lateralErrorValue.contains(index)){
      QCPItemText *text = new QCPItemText(ui->lateralErrorPlot);
      ui->lateralErrorPlot->addItem(text);
      text->setFont(QFont(font().family(), 9));
      text->setPadding(QMargins(8, 0, 0, 0));
      lateralErrorValue[index] = text;
    }*/

    //Set bar data
    lateralBarGraphs[index]->setWidth(0.9);
    lateralBarGraphs[index]->setData(QVector<double>() << index,
				     QVector<double>() << ratio*value);
    
    //Set bar data
    lateralBarGraphs2[index]->setWidth(0.9);
    lateralBarGraphs2[index]->setData(QVector<double>() << index,
				      QVector<double>() << (1.0-ratio)*value);

    //Set ratio text
    /*lateralErrorValue[index]->setText( QString::number(ratio,'g',2) );
    if(value>=0)
      lateralErrorValue[index]->position->setCoords(index,value+0.1);
    else
      lateralErrorValue[index]->position->setCoords(index,value-0.1);*/
      
    //Reset range
    if(lateralErrorRange.addValue(value)){
        ui->lateralErrorPlot->yAxis->setRange(lateralErrorRange.min()-0.1*fabs(lateralErrorRange.min()),
					      lateralErrorRange.max()+0.1*fabs(lateralErrorRange.max()));
    }

    //Force replot
    if(autoReplot)
      ui->lateralErrorPlot->replot(QCustomPlot::rpQueued);
}

void MainWindow::zoomIn()
{
    ui->zoomSlider->setValue( ui->zoomSlider->value() + 1 );
}

void MainWindow::zoomOut()
{
    ui->zoomSlider->setValue( ui->zoomSlider->value() - 1 );
}

void MainWindow::zoomFit()
{
    ui->zoomSlider->setValue( 0 );
}

void MainWindow::zoom(int value)
{
    double smax = vp.length() * (1.0 - value/100.0);
    double center = upRect->axis(QCPAxis::atBottom)->range().center();
    upRect->axis(QCPAxis::atBottom)->setRange(center,smax,Qt::AlignCenter);
    bottomRect->axis(QCPAxis::atBottom)->setRange(center,smax,Qt::AlignCenter);
    
    ui->mainPlot->replot(QCustomPlot::rpQueued);  
}

void MainWindow::positionFit()
{
    positionChanged(0);
}

void MainWindow::positionChanged(int value)
{
    double smax = vp.length();
    double size = upRect->axis(QCPAxis::atBottom)->range().size();
    upRect->axis(QCPAxis::atBottom)->setRange(smax * value/1000.0 + size/2.0, size, Qt::AlignCenter);
    bottomRect->axis(QCPAxis::atBottom)->setRange(smax * value/1000.0 + size/2.0, size, Qt::AlignCenter);
    ui->mainPlot->replot(QCustomPlot::rpQueued);
}

void MainWindow::xAxisChanged(QCPRange range)
{
    double smax = vp.length();
    double size = range.size();
    ui->positionScrollBar->setValue(qRound((range.center()-size/2)*1000.0/smax));
}


void MainWindow::settings()
{
    ColorDialog *colorDialog = new ColorDialog(m_colors,this);
    
    if(colorDialog->exec()){
	m_colors = colorDialog->getListColor();
	
	for(int i=0;i<m_colors.size();i++){
	    int robotIndex = i+1;
	    //Pavin view
	    if(robotEllipseItem.contains(robotIndex))
	      robotEllipseItem[robotIndex]->setBrush(m_colors.at(i));
	    if(robotLineItem.contains(robotIndex))
	      robotLineItem[robotIndex]->setPen(m_colors.at(i));
	    if(robotTraceItem.contains(robotIndex))
	      robotTraceItem[robotIndex]->setPen(m_colors.at(i));
	    
	    //Bar graphs
	    if(longitudinalBarGraphs1.contains(robotIndex))
	      longitudinalBarGraphs1[robotIndex]->setBrush(m_colors.at(i));
	    if(lateralBarGraphs.contains(robotIndex))
	      lateralBarGraphs[robotIndex]->setBrush(m_colors.at(i));

	    //Main graphs
	    if(positionGraph.contains(robotIndex))
	      positionGraph[robotIndex]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1.5), QBrush(m_colors.at(i)), 9));
	    if(errorGraph.contains(robotIndex))
	      errorGraph[robotIndex]->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QPen(Qt::black, 1.5), QBrush(m_colors.at(i)), 9));
	    if(errorCurveGraph.contains(robotIndex))
	      errorCurveGraph[robotIndex]->setPen(m_colors.at(i));
	    
	    updatePlots();
	}
    }
}








void MainWindow::simulate()
{
    if(!m_init) return;

    if(!timer)
        timer = new QTimer(this);

    if(!timer->isActive()){
        foreach (QGraphicsPathItem* item, robotTraceItem) {
            ui->graphicsView->scene()->removeItem(item);
        }
        robotTraceItem.clear();
        foreach (QCPGraph* graph, errorCurveGraph) {
            ui->mainPlot->removeGraph(graph);
        }
        errorCurveGraph.clear();

        timer->setSingleShot(false);
        timer->setInterval(50);
        connect(timer,SIGNAL(timeout()),this,SLOT(simulate()));
        timer->start();
        screenshotIndex = 0;
    }

    simulatedTime += 0.25;

    double smax = vp.length();

    if(simulatedTime>0 && simulatedTime<1.3*smax){
	double abscissa = simulatedTime;
	while(abscissa>=smax)
	  abscissa -= smax;
        QPointF pt = vp.path(abscissa) + QPointF(frand(-0.1,0.1),frand(-0.1,0.1));
        QPair<int,double> abs = vp.findClosestPoint(pt.x(),pt.y());
        double s = vp.toGlobalAbscissa(abs.first,abs.second);
        double dv = vp.velocity(abscissa);
        double v = dv + (double)(qrand()%1000-500)/10000.0 + simulatedTime/1500.0;
        double e1 = (double)(rand()%10000-5000)/10000.0;
	double e2 = (double)(rand()%10000-5000)/10000.0;
	double e3 = (double)(rand()%10000-5000)/10000.0;
	setRobotPositionVelocityError(1,pt.x(),pt.y(),s,v,e1,e2,0.1 + 0.9*(abscissa/smax) ,e3,true,true,true);
    }
    if(simulatedTime>10 && simulatedTime<1.2*smax+10){
	double abscissa = simulatedTime-10;
	while(abscissa>=smax)
	  abscissa -= smax;
        QPointF pt = vp.path(abscissa) + QPointF(frand(-0.1,0.1),frand(-0.1,0.1));
        QPair<int,double> abs = vp.findClosestPoint(pt.x(),pt.y());
        double s = vp.toGlobalAbscissa(abs.first,abs.second);
        double dv = vp.velocity(abscissa);
        double v = dv - (double)(qrand()%1000-500)/10000.0 - simulatedTime/1000.0;
        double e1 = (double)(rand()%10000-5000)/10000.0;
	double e2 = (double)(rand()%10000-5000)/10000.0;
	double e3 = (double)(rand()%10000-5000)/10000.0;
	setRobotPositionVelocityError(3,pt.x(),pt.y(),s,v,e1,e2,0.2 + 0.8*(abscissa/smax),e3,false,true,false);
    }
    if(simulatedTime>20 && simulatedTime<smax+20){
	double abscissa = simulatedTime-20;
	while(abscissa>=smax)
	  abscissa -= smax;
        QPointF pt = vp.path(abscissa) + QPointF(frand(-0.3,0.3),frand(-0.3,0.3));
        QPair<int,double> abs = vp.findClosestPoint(pt.x(),pt.y());
        double s = vp.toGlobalAbscissa(abs.first,abs.second);
        double dv = vp.velocity(abscissa);
        double v = dv + (double)(qrand()%1000-500)/10000.0 + simulatedTime/500.0;
        double e1 = (double)(rand()%10000-5000)/10000.0;
	double e2 = (double)(rand()%10000-5000)/10000.0;
	double e3 = (double)(rand()%10000-5000)/10000.0;
	setRobotPositionVelocityError(2,pt.x(),pt.y(),s,v,e1,e2,0.3 + 0.7*(abscissa/smax),e3,true,true,false);
    }

    updatePlots();
/*
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QPixmap pm = QPixmap::grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#else
  QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#endif
  QString fileName = QString("image%1.png").arg(screenshotIndex,5);
  screenshotIndex++;
  fileName.replace(" ","0");
  pm.save("./screenshots/"+fileName);
*/
    if(simulatedTime>smax+20)
        timer->stop();
}
