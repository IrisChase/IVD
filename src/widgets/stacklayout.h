#pragma once

#include "user_include/cpp/IVD_cpp.h"

namespace IVD
{
namespace std_widgets
{

class StackLayout : public bindings::UserLayout
{
    //basically reverse insertion order
    void applyToChildrenInBubbleOrder(std::function<void(bindings::Element)> fun)
    {
        std::vector<bindings::Element> elements;

        applyToChildren([&](bindings::Element elem)
        { elements.push_back(elem); });

        for(auto rit = elements.rbegin(); rit != elements.rend(); ++rit)
            fun(*rit);
    }

public:
    virtual FillPrecedence get_fill_precedence(const Angle angel)
    { return any_greedy_children_for_angle(angel); }

    virtual void shape(const GeometryProposal officialProposal);

    virtual void draw(bindings::Canvas theCanvas)
    {
        applyToChildren([&](bindings::Element elem)
        { elem.render(); });
    }

    virtual void bubble_children()
    {
        applyToChildrenInBubbleOrder([&](bindings::Element elem)
        { elem.bubble(); });
    }
};

}//std_widgets
}//IVD
