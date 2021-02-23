#pragma once

#include "user_include/cpp/IVD_cpp.h"

#include "OpenImageIO/imagecache.h"

namespace IVD
{
namespace std_widgets
{

class ImageWidget : public UserWidget
{
    OIIO::ImageCache* cache;

public:
    virtual FillPrecedence get_fill_precedence(const Angle)
    { return FillPrecedence::Shrinky; }

    virtual void shape(const GeometryProposal officialProposal);
    virtual void draw(Canvas theCanvas);
};


}//std_widgets
}//IVD
