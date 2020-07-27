// Copyright 2020 Iris Chase
//
// Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "graph.h"
#include "keywords.h"

#include <cmath>
#include <sstream>

namespace IVD
{
namespace Animation
{

Graph::TwoSamplePoints Graph::getSamplePoints(double percent) const
{
    Sample leftSample  = {0, 0};
    Sample rightSample = {1, 1};

    for(auto it = mySamples.begin(); it != mySamples.end(); ++it)
    {
        auto sample = *it;

        if(sample.x == percent) return {sample, sample};

        if(sample.x < percent)
        {
            if(it + 1 != mySamples.end()) continue;

            //Otherwise, we don't have a user-defined *right* weight that is as large
            // as the percentage, which defaults to 1 @ 1, which is already set above.
            //So we just use this one as the left weight
            leftSample = leftSample;
            break;
        }
        if(sample.x > percent)
        {
            rightSample = sample;

            if(it == mySamples.begin())
            {
                //then, there is no defined weight small enough, in which case we
                // define it as 0 @ 0, which is what leftWeight is already initialized to.
                break;
            }

            --it;

            leftSample = *it;
            break;
        }
    }
    
    return {leftSample, rightSample};
}

double Graph::getLinearWeightForPercentage(double xpos) const
{
	const TwoSamplePoints points = getSamplePoints(xpos);
    
    if(points.left.y == points.right.y)
        return points.left.y;

    //http://paulbourke.net/miscellaneous/interpolation/
    return points.left.y * (1 - xpos) + points.right.y * xpos;
}

double Graph::getSmoothWeightForPercentage(double xpos) const
{
	const TwoSamplePoints points = getSamplePoints(xpos);
    
    if(points.left.y == points.right.y)
        return points.left.y;
    
    //Used without understanding from:
    //http://paulbourke.net/miscellaneous/interpolation/
    xpos = (1 - std::cos(xpos * M_PI)) / 2;
    return points.left.y * (1 - xpos) + points.right.y * xpos;
}

Graph::Graph(): interpolationMode(Keyword::Linear) {}

int Graph::getInterpolatedScalarForPercentage(const int origin,
                                              const int dest,
                                              const double percentage) const
{
    const double destWeight = [&]
    {
        switch(interpolationMode)
        {
        case Keyword::Linear: return getLinearWeightForPercentage(percentage);
        case Keyword::Smooth: return getSmoothWeightForPercentage(percentage);
        default: assert(false);
        }
    }();
    
    const double originWeight  = 1 - destWeight;
    return (origin * originWeight) + (dest * destWeight);
}

std::string Graph::generatePrintout()
{
    std::stringstream ss;
    ss << "Mode: ";

    if(interpolationMode == Keyword::Linear)      ss << "Linear";
    else if(interpolationMode == Keyword::Smooth) ss << "Sinusoidal";
    else throw std::logic_error("Corrupt interpolation mode in Graph");

    ss << ", samples: ";

    for(auto it = mySamples.begin(); it != mySamples.end(); ++it)
    {
        const Sample sample = *it;
        ss << sample.x << " @ " << sample.y << (it + 1 != mySamples.end() ? ", "
                                                                          : ".");
    }

    return ss.str();
}

std::string Transition::generatePrintout()
{
    std::stringstream ss;
    ss << "Time: " << miliseconds << "ms. " << graph.generatePrintout();
    return ss.str();
}

}//Animation
}//IVD

