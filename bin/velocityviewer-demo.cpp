#include "plot.h"

/**
* @author Léo Baudouin
* @em leo.baudouin@univ-bpclermont.fr
*/

int main(int argc, char* argv[])
{
    Plot plot;
    plot.load("../data/px_t2.txt","../data/py_t2.txt","../data/pk_t2.txt","../data/vp_t2_a0.3mv1.5ca0.2.txt",true);

    plot.setRobotPositionVelocityError(1,172,34,45,0.75,0,true,true,true);

    plot.setRobotPositionVelocityError(1,172,35,46,0.35,0.25,true,true,true);
    plot.setRobotError(1,0.5);

    return 0;
}

