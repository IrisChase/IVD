#include "material/simplematerial.h"

#include "displayitem.h"

namespace IVD
{

//Not as simple as we thought.
FillPrecedence SimpleMaterial::computerFillPrecedenceForAngle(const Angle theAngle)
{
    //It'll accept anything you give it, unless it has a size defined, in which case it's shrinky.
    if(myItem->getReservedDimens().get(theAngle) + myItem->getSizeForAngle(theAngle))
        return FillPrecedence::Shrinky;
    return FillPrecedence::Greedy;
}



}//IVD
