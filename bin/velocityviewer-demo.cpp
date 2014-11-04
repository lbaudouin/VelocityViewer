#include "plot.h"

/**
* @author Léo Baudouin
* @em leo.baudouin@univ-bpclermont.fr
*/

inline double frand(double min, double max)
{
    if(min==max) return min;
    if(min>max) std::swap(min,max);
    double f = (double)rand() / RAND_MAX;
    return min + f * (max - min);
}
    
int main(int argc, char* argv[])
{
    QString xPathFilename = "../../data/px_t2.txt";
    QString yPathFilename = "../../data/py_t2.txt";
    QString curvatureFilename = "../../data/pk_t2.txt";
    QString velocityFilename = "../../data/vp_t2_a0.3mv1.5ca0.2.txt";
  
  
    Plot plot;
    plot.load(xPathFilename,yPathFilename,curvatureFilename,velocityFilename,true);

    plot.setRobotPositionVelocityError(1, 165, 30, 45, 0.75, 0.3,0.5,0.25, 0.25,true,true,true);
    


    
    plot.clearGraphics();
    
    VelocityData vp;
    vp.loadPath(xPathFilename,yPathFilename);
    vp.loadCurvature(curvatureFilename);
    vp.loadVelocityProfile(velocityFilename);
    
    int nbRobots = 200;
    double smax = vp.length();    
    for(double simulatedTime = 0.0; simulatedTime<smax+nbRobots*10; simulatedTime += 0.25){
    
      for(int i=0;i<nbRobots;i++){
	double time = simulatedTime - i*10;
	if(time>0 && time<smax){
	    double abscissa = time;
	    while(abscissa>=smax)
	      abscissa -= smax;
	    QPointF pt = vp.path(abscissa) + QPointF(frand(-0.1,0.1),frand(-0.1,0.1));
	    QPair<int,double> abs = vp.findClosestPoint(pt.x(),pt.y());
	    double s = vp.toGlobalAbscissa(abs.first,abs.second);
	    double dv = vp.velocity(abscissa);
	    double v = dv + (double)(rand()%1000-500)/10000.0 + time/1500.0;
	    double e1 = (double)(rand()%10000-5000)/10000.0;
	    double e2 = (double)(rand()%10000-5000)/10000.0;
	    double e3 = (double)(rand()%10000-5000)/10000.0;
	    plot.setRobotPositionVelocityError(i+1,pt.x(),pt.y(),s,v,e1,e2,0.1 + 0.9*(abscissa/smax) ,e3,true,true,(i==0?true:false));
	}
      }
      usleep(15000);
    }
    

    return 0;
}

