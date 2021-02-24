/*
 * ccfits.cpp
 * 
 * Copyright (C) 2020-2021 Max Qian
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
 
/************************************************* 
 
Copyright: 2020-2021 Max Qian. All rights reserved
 
Author:Max Qian

E-mail:astro_air@126.com
 
Date:2021-2-24
 
Description:CCFits Lib
 
**************************************************/

#include <iostream>
#include <assert.h>


#include "ccfits.h"

using namespace std;
using namespace cimg_library;
using namespace CCfits;

namespace AstroAir
{
    void readFile(CImg<float> & inImg, const string & inFilename) 
    {
        std::auto_ptr<FITS> pInfile(new FITS(inFilename, Read, true));
        PHDU & image = pInfile->pHDU(); 
        inImg.resize(image.axis(0) /*x*/, image.axis(1) /*y*/, 1 /*z*/, 1 /*1 color*/);
        std::valarray<unsigned long> imgData;
        image.read(imgData);
        cimg_forXY(inImg, x, y)
        {
            inImg(x, inImg.height() - y - 1) = imgData[inImg.offset(x, y)];
        }
    }

    bool insideCircle(float inX /*pos of x*/, float inY /*pos of y*/, float inCenterX, float inCenterY, float inRadius)
    {
    return (pow(inX - inCenterX, 2.0) + pow(inY - inCenterY, 2.0) <= pow(inRadius, 2.0));
    }

    float calcHfd(const CImg<float> & inImage, unsigned int inOuterDiameter) 
    {
        float outerRadius = inOuterDiameter / 2;
        float sum = 0, sumDist = 0;
        int centerX = ceil(inImage.width() / 2.0);
        int centerY = ceil(inImage.height() / 2.0);
        cimg_forXY(inImage, x, y) 
        {
            if (insideCircle(x, y, centerX, centerY, outerRadius)) 
            {
                sum += inImage(x, y);
                sumDist += inImage(x, y) * sqrt(pow((float) x - (float) centerX, 2.0f) + pow((float) y - (float) centerY, 2.0f));
            }
        }
        return (sum ? 2.0 * sumDist / sum : sqrt(2.0) * outerRadius);
    }

    float CalcHFD(std::string ImageName)
    {
        CImg<float> img;
        try {
        readFile(img, ImageName);
        } catch (FitsException &) {
        cerr << "Read FITS failed." << endl;
        return 1;
        }
        CImg<float> img2(img);
        const unsigned int outerDiameter = 60;
        float hfd = calcHfd(img2, outerDiameter);
        return hfd;
    }
}