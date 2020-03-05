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

#include "imagematerial.h"
#include "displayitem.h"
#include "canvas.h"

namespace IVD
{

std::string ImageMaterial::getPath(DisplayItem* item)
{
    auto optionalUserToken = item->getAttr().getUserToken(AttributeKey::ImagePath);
    assert(optionalUserToken);
    return *optionalUserToken;
}

void ImageMaterial::shapeDrawingArea(const GeometryProposal officalProposal)
{
    Dimens result;

    OIIO::ImageSpec spec;
    bool stat = cache->get_imagespec(OIIO::ustring(getPath(myItem)), spec);

    if(!stat)
    {
        setDrawingArea(officalProposal.proposedDimensions);
        return;
    }

    //This is about as simple as it gets.
    setDrawingArea(officalProposal.roundConflicts(Dimens(spec.width, spec.height)));
}

void ImageMaterial::drawConcrete(Canvas* theCanvas)
{
    const OIIO::ustring path(getPath(myItem));

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

    Bitmap myBitmap;
    myBitmap.stride = 0;
    myBitmap.width = spec.width;
    myBitmap.height = spec.height;
    myBitmap.channels = channelCount;
    myBitmap.data = &pixels[0];

    theCanvas->drawBitmapRGBoptionalA(getDrawingArea(), myBitmap);
}

}//IVD
