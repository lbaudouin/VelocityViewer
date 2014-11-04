#include "velocitydata.h"

VelocityData::VelocityData(QWidget* parent) : m_parent(parent)
{
}


bool VelocityData::loadCurvature(const QString &curvatureFilename)
{
    //Open file
    QFile file(curvatureFilename);
    if(!file.exists()){
	if(m_parent)
	  QMessageBox::critical(m_parent,QString("Error"),QString("File '%1' doesn't exists").arg(curvatureFilename));
        return false;
    }
    if(!file.open(QFile::ReadOnly)){
	if(m_parent)
	  QMessageBox::critical(m_parent,QString("Error"),QString("Can't read '%1'").arg(curvatureFilename));
        return false;
    }

    m_curvature.clear();

    QTextStream stream(&file);
    QString line;

    do{
        line = stream.readLine();
        if(line.isEmpty()) break;

        QList<double> values;

        QTextStream lineStream(&line);

        for(int i=0;i<3;i++){
            double v;
            lineStream >> v;
            values.push_front(v);
        }

        m_curvature.push_back(values);

    }while(!line.isEmpty());

    return true;
}

bool VelocityData::loadVelocityProfile(const QString &velocityFilename)
{
    //Open file
    QFile file(velocityFilename);
    if(!file.exists()){
	if(m_parent)
	  QMessageBox::critical(m_parent,QString("Error"),QString("File '%1' doesn't exists").arg(velocityFilename));
        return false;
    }
    if(!file.open(QFile::ReadOnly)){
	if(m_parent)
	  QMessageBox::critical(m_parent,QString("Error"),QString("Can't read '%1'").arg(velocityFilename));
        return false;
    }

    m_velocityCurves.clear();

    QTextStream stream(&file);
    QString line;

    do{
        line = stream.readLine();
        if(line.isEmpty()) break;

        QTextStream lineStream(&line);

        QString key,tmp;

        lineStream >> key;

        if(key.isEmpty())
            continue;

        if(key == "curve"){
            int index;
            lineStream >> tmp >> index;
            Curve curve;
            curve.index = index;
            m_velocityCurves.push_back(curve);
        }else{
            double u0,u1,s0,s1;
            lineStream >> u0 >> u1 >> s0 >> s1;
            m_velocityCurves.last().u0.push_back(u0);
            m_velocityCurves.last().u1.push_back(u1);
            m_velocityCurves.last().s0.push_back(s0);
            m_velocityCurves.last().s1.push_back(s1);

            QList<double> values;
            double v;
            if( key == "MaxVel" ){
                m_velocityCurves.last().mode.push_back(VelocityData::MaxVel);
                lineStream >> v;
                values << v;
            }else if( key == "LonAcc" ){
                m_velocityCurves.last().mode.push_back(VelocityData::LonAcc);
                for(int i=0;i<6;i++){
                    lineStream >> v;
                    values << v;
                }
            }else if( key == "LatAcc" ){
                m_velocityCurves.last().mode.push_back(VelocityData::LatAcc);
                for(int i=0;i<5;i++){
                    lineStream >> v;
                    values << v;
                }
            }
            m_velocityCurves.last().values.push_back(values);
        }

    }while(!line.isEmpty());

    return true;
}

bool VelocityData::loadPath(const QString &xFilename, const QString &yFilename)
{
    //Open file
    QFile xFile(xFilename);
    if(!xFile.exists()){
	if(m_parent)
	  QMessageBox::critical(m_parent,QString("Error"),QString("File '%1' doesn't exists").arg(xFilename));
        return false;
    }
    if(!xFile.open(QFile::ReadOnly)){
	if(m_parent)
	  QMessageBox::critical(m_parent,QString("Error"),QString("Can't read '%1'").arg(xFilename));
        return false;
    }

    m_xPath.clear();

    QTextStream xStream(&xFile);
    QString line;

    do{
        line = xStream.readLine();
        if(line.isEmpty()) break;

        QList<double> values;

        QTextStream lineStream(&line);

        for(int i=0;i<8;i++){
            double v;
            lineStream >> v;
            values.push_front(v);
        }

        m_xPath.push_back(values);

    }while(!line.isEmpty());

    QFile yFile(yFilename);
    if(!yFile.exists()){
	if(m_parent)
	  QMessageBox::critical(m_parent,QString("Error"),QString("File '%1' doesn't exists").arg(xFilename));
        return false;
    }
    if(!yFile.open(QFile::ReadOnly)){
	if(m_parent)
	  QMessageBox::critical(m_parent,QString("Error"),QString("Can't read '%1'").arg(xFilename));
        return false;
    }

    m_yPath.clear();

    QTextStream yStream(&yFile);

    do{
        line = yStream.readLine();
        if(line.isEmpty()) break;

        QList<double> values;

        QTextStream lineStream(&line);

        for(int i=0;i<8;i++){
            double v;
            lineStream >> v;
            values.push_front(v);
        }

        m_yPath.push_back(values);

    }while(!line.isEmpty());

    return true;
}

bool VelocityData::isValid()
{
    int size = m_velocityCurves.size();
    return (m_xPath.size() == size) && (m_yPath.size() == size) && (m_curvature.size() ==size);
}

double VelocityData::evalPoly(const QList<double> &poly,  const double &s) const
{
    double v = poly[0];
    double fs = s;
    for(int i=1;i<poly.size();i++){
        v += poly[i]*fs;
        fs *= s;
    }
    return v;
}

QPointF VelocityData::path(double abscissa) const
{
    QPair<int,double> localAbscissa = toLocalAbscissa(abscissa);
    return path(localAbscissa.first,localAbscissa.second);
}

QPointF VelocityData::path(int curveIndex, double localAbscissa) const
{
    double x = evalPoly(m_xPath.at(curveIndex),localAbscissa);
    double y = evalPoly(m_yPath.at(curveIndex),localAbscissa);
    return QPointF(x,y);
}

double VelocityData::curvature(double abscissa) const
{
    QPair<int,double> localAbscissa = toLocalAbscissa(abscissa);
    return curvature(localAbscissa.first,localAbscissa.second);
}

double VelocityData::curvature(int curveIndex, double localAbscissa) const
{
    return evalPoly(m_curvature.at(curveIndex),localAbscissa);
}

double VelocityData::velocity(double abscissa) const
{
    QPair<int,double> localAbscissa = toLocalAbscissa(abscissa);
    return velocity(localAbscissa.first,localAbscissa.second);
}

double VelocityData::velocity(int curveIndex, double localAbscissa) const
{
    double abscissa = toGlobalAbscissa(curveIndex,localAbscissa);

    //Searching current mode index
    int currentMode=0;
    while(localAbscissa>m_velocityCurves[curveIndex].u1[currentMode]) {
      currentMode++;
    }
    assert(currentMode<m_velocityCurves[curveIndex].mode.size());

    //Current parameters
    VelocityMode mode = m_velocityCurves[curveIndex].mode.at(currentMode);
    QList<double> values = m_velocityCurves[curveIndex].values[currentMode];

    //Compute desired velocity
    switch(mode){
        case MaxVel:{
            return values[0];
        }
        case LonAcc:{
            assert(values.size()==6);
            double s0=values[0], v0=values[2], ac=values[4], a=values[5];
            double sgn_a = (a < 0) ? -1.0 : 1.0;
            return sqrt(pow(v0,2) + sgn_a*2.0*ac*(abscissa-s0));
        }
        case LatAcc:{
            assert(values.size()==5);
            double alpha=values[0], p=values[1];
            QList<double> pk;
            pk << values[4] <<  values[3] << values[2];
            double k = evalPoly(pk,localAbscissa);
            return alpha*(pow(fabs(k),p));
        }
        default:
            return 0.0;
    }
}

int VelocityData::findCurveIndex(double abscissa) const
{
    int index = 0;
    while(m_velocityCurves.at(index).s1.last()<abscissa){
        index++;
        assert(index<m_velocityCurves.size());
    }
    return index;
}

QPair<int,double> VelocityData::toLocalAbscissa(double abscissa) const
{
    int curveIndex = findCurveIndex(abscissa);
    double localAbscissa = (abscissa-m_velocityCurves[curveIndex].s0.front()) / (m_velocityCurves[curveIndex].s1.back()-m_velocityCurves[curveIndex].s0.front());
    return qMakePair<int,double>(curveIndex,localAbscissa);
}

double VelocityData::toGlobalAbscissa(int curveIndex, double localAbscissa) const
{
    assert(curveIndex>=0 && curveIndex<m_velocityCurves.size());
    return m_velocityCurves[curveIndex].s0.front() + localAbscissa * (m_velocityCurves[curveIndex].s1.back() - m_velocityCurves[curveIndex].s0.front());
}

double VelocityData::length() const
{
    return m_velocityCurves.last().s1.last();
}

VelocityData::VelocityMode VelocityData::mode(double abscissa) const
{
    QPair<int,double> localAbscissa = toLocalAbscissa(abscissa);
    int currentCurve = localAbscissa.first;
    double currentAbscissa = localAbscissa.second;

    //Searching current mode index
    int currentMode=0;
    while(currentAbscissa>m_velocityCurves[currentCurve].u1[currentMode]) {
      currentMode++;
    }

    //Current parameters
    return m_velocityCurves[currentCurve].mode.at(currentMode);
}

QList<ModeInfo> VelocityData::modes() const
{
    QList<ModeInfo> modes;
    VelocityData::VelocityMode previousMode = VelocityData::NoMode;
    for(int i=0;i<m_velocityCurves.size();i++){
        for(int j=0;j<m_velocityCurves.at(i).mode.size();j++){
            if(m_velocityCurves.at(i).mode.at(j)!=previousMode){
                ModeInfo info;
                info.mode = m_velocityCurves.at(i).mode.at(j);
                info.s0 =  m_velocityCurves.at(i).s0.at(j);
                modes << info;

            }
            modes.back().s1 = m_velocityCurves.at(i).s1.at(j);
        }
    }
    return modes;
}

int VelocityData::curveInfo(double abscissa) const
{
    return findCurveIndex(abscissa);
}

QList<CurveInfo> VelocityData::curveInfos() const
{
    QList<CurveInfo> list;
    for(int i=0;i<m_velocityCurves.size();i++){
        CurveInfo info;
        info.index = m_velocityCurves.at(i).index;
        info.s0 = m_velocityCurves.at(i).s0.front();
        info.s1 = m_velocityCurves.at(i).s1.back();
        list << info;
    }
    return list;
}

QList<QPointF> VelocityData::startPoints() const
{
    QList<QPointF> list;
    for(int i=0;i<m_xPath.size();i++){
        list << QPointF(evalPoly(m_xPath[i],0),evalPoly(m_yPath[i],0));
    }
    return list;
}

QPair<int,double> VelocityData::findClosestPoint(double x, double y) const
{
    QPointF p(x,y);

    int bestCurve = -1;
    double bestAbscissa = -1;
    double bestDistance = -1;

    double step = 0.01;
    for(int i=0;i<m_xPath.size();i++){
        for(double s=0;s<1.0;s+=step){
            QPointF ps = path(i,s);
            double distance = QLineF(p,ps).length();

            if(bestDistance<0 || distance<bestDistance){
                bestDistance = distance;
                bestCurve = i;
                bestAbscissa = s;
            }
        }
    }

    return qMakePair<int,double>(bestCurve,bestAbscissa);
}
