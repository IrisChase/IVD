#include "stacklayout.h"

namespace IVD
{
namespace std_widgets
{

void StackLayout::shape(const GeometryProposal officalProposal)
{
    std::vector<Dimens> initialSizes;

    applyToChildren([&](Element elem)
    {
        elem.shape(officalProposal);
        initialSizes.push_back(elem.get_dimens());
    });

    Dimens maxed;

    auto maximizeForAngle = [&](const Angle theAngle)
    {
        auto getLargerAnglePredicate = [&](const Angle theAngle)
        {
            return [=](const Dimens& left, const Dimens& right)
            {
                return left.get(theAngle) < right.get(theAngle);
            };
        };

        auto it = std::max_element(initialSizes.begin(),
                                   initialSizes.end(),
                                   getLargerAnglePredicate(theAngle));

        maxed.get(theAngle) = it != initialSizes.end() ? it->get(theAngle)
                                                       : officalProposal.proposedDimensions.get(theAngle);
    };

    maximizeForAngle(Angle::Horizontal);
    maximizeForAngle(Angle::Vertical);

    //Allow shrink/grow all false by default
    GeometryProposal fin;
    fin.proposedDimensions = maxed;

    applyToChildren([&](Element elem)
    {
        elem.shape(fin);
        elem.set_offset(Coords());
    });

    myDimens = maxed;
}

}//std_widgets
}//IVD
