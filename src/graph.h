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

#pragma once

#include <vector>
#include <string>

#include "irisutils/lexcompare.h"

#include "codeposition.h"

namespace IVD
{

namespace Animation
{

class Graph
{
public:
    struct Sample
    {
        double x;
        double y; //Constantly double your weight with this one  w e i r d   t r i c k

        IRISUTILS_DEFINE_COMP(Sample, x, y)
    };

private:

    std::vector<Sample> mySamples;
    
    struct TwoSamplePoints
    {
        Sample left;
        Sample right;
    };

    int interpolationMode;
    
    TwoSamplePoints getSamplePoints(double percent);
    
    double getLinearWeightForPercentage(double percent);
    double getSmoothWeightForPercentage(double percent);

public:
    Graph();
    Graph(const int interpolationMode): interpolationMode(interpolationMode) {}

    void addSample(const double weight, const double percent)
    { mySamples.emplace_back(Sample{weight, percent}); }

    void setSamples(const std::vector<Sample> samples)
    { mySamples = samples; }

    int getInterpolatedScalarForPercentage(const int origin,
                                     const int dest,
                                     const double percentage);



    //Rename this to unleash dragons.
    CodePosition definedAt;

    std::string generatePrintout();

    IRISUTILS_DEFINE_COMP(Graph, mySamples, interpolationMode, definedAt)
};

struct Transition
{
    int miliseconds;
    Graph graph; //Girafe, what a gaffe...

    std::string generatePrintout();

    IRISUTILS_DEFINE_COMP(Transition, miliseconds, graph)
};


}//Animation
}//IVD
