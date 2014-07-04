#include "plot.h"

/**
* @author Léo Baudouin
* @em leo.baudouin@univ-bpclermont.fr
*/

int main(int argc, char* argv[])
{
    Plot plot;
    plot.load("../../data/px_t2.txt","../../data/py_t2.txt","../../data/pk_t2.txt","../../data/vp_t2_a0.3mv1.5ca0.2.txt",true);

    plot.setRobotPositionVelocityError(1,172,34,45,0.75,0.3,0.5,0.25,0.5,true,true,true);
    plot.setRobotPositionVelocityError(1,172,34,45,0.75,0.3,-0.5,0.25,-0.5,true,true,true);
    plot.setRobotPositionVelocityError(1,172,34,2046,0.75,0.3,-0.5,0.25,-0.5,true,true,true);
    plot.setRobotPositionVelocityError(1,172,34,-2046,0.75,0.3,-0.5,0.25,-0.5,true,true,true);

    return 0;
}

