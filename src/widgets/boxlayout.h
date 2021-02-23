#pragma once

#include "user_include/cpp/IVD_cpp.h"

namespace IVD
{

namespace std_widgets
{

class BoxLayout : public bindings::UserLayout
{
    const Angle Adjacent;

public:
    BoxLayout(const Angle theAngel): Adjacent(theAngel) {}
    ~BoxLayout() override {}

    virtual FillPrecedence get_fill_precedence(const Angle angle) final
    { return any_greedy_children_for_angle(angle); }

    virtual void shape(const GeometryProposal officialProposal) final;

    virtual void draw(bindings::Canvas theCanvas) final
    { render_children_unordered(); }

    virtual void bubble_children() final
    { bubble_children_unordered(); }
};

class VboxLayout : public BoxLayout
{
public:
    VboxLayout(): BoxLayout(Angle::Vertical) {}
    ~VboxLayout() override {}
};

class HboxLayout : public BoxLayout
{
public:
    HboxLayout(): BoxLayout(Angle::Horizontal) {}
    ~HboxLayout() override {}
};


}//std_widgets
}//IVD
