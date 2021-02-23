#include "image.h"


namespace IVD
{
namespace std_widgets
{


void ImageWidget::shape(const GeometryProposal officialProposal)
{
    OIIO::ImageSpec spec;
    const char* imagePath = "oof need custom attributes TODO";
    bool stat = cache->get_imagespec(OIIO::ustring(imagePath), spec);

    if(stat)
        //This is about as simple as it gets.
        myDimens = officialProposal.roundConflicts(Dimens(spec.width, spec.height));
    else
        myDimens = officialProposal.proposedDimensions;

}

void ImageWidget::draw(Canvas theCanvas)
{
    const OIIO::ustring path("aiufdshaoiuha");

    OIIO::ImageSpec spec;
    bool stat = cache->get_imagespec(path, spec);

    if(!stat) return;

    const int channelCount = spec.nchannels > 4 ? 4
                                                : spec.nchannels;

    std::vector<unsigned char> pixels(spec.width * spec.height * channelCount);

    stat = cache->get_pixels(path,
                             0, 0,
                             spec.x, spec.x + spec.width,
                             spec.y, spec.y + spec.height,
                             spec.z, spec.z + spec.depth,
                             0, channelCount,
                             OIIO::TypeDesc::UCHAR, &pixels[0]);

    if(!stat)
    {
        std::cerr << cache->geterror() << std::endl;
        return;
    }

    theCanvas.drawBitmapRGBoptionalA(Coords(),
                                     spec.width,
                                     spec.height,
                                     0,
                                     channelCount,
                                     &pixels[0]);
}


}//std_widgets
}//IVD
