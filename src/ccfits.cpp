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
#include <set>

#include "ccfits.h"

using namespace std;
using namespace cimg_library;
using namespace CCfits;

typedef tuple<int /*x*/,int /*y*/> PixelPosT;
typedef set<PixelPosT> PixelPosSetT;
typedef list<PixelPosT> PixelPosListT;
typedef tuple<float, float> PixSubPosT;
typedef tuple<float /*x1*/, float /*y1*/, float /*x2*/, float /*y2*/> FrameT;

struct StarInfoT {
  FrameT clusterFrame;
  FrameT cogFrame;
  FrameT hfdFrame;
  PixSubPosT cogCentroid;
  PixSubPosT subPixelInterpCentroid;
  float hfd;
  float fwhmHorz;
  float fwhmVert;
  float maxPixValue;
  bool saturated;
};
typedef list<StarInfoT> StarInfoListT;

namespace AstroAir
{
    void readFile(CImg<float> & inImg, const string & inFilename, long * outBitPix = 0)
    {
        std::unique_ptr<FITS> pInfile(new FITS(inFilename, Read, true));
        PHDU & image = pInfile->pHDU(); 
        if (outBitPix) 
        {
            *outBitPix = image.bitpix();
        }
        inImg.resize(image.axis(0) /*x*/, image.axis(1) /*y*/, 1/*z*/, 1 /*1 color*/);
        std::valarray<unsigned long> imgData;
        image.read(imgData);
        cimg_forXY(inImg, x, y) 
        { 
            inImg(x, inImg.height() - y - 1) = imgData[inImg.offset(x, y)]; 
        }  
    } 

    void thresholdOtsu(const CImg<float> & inImg, long inBitPix, CImg<float> * outBinImg)
    {
        CImg<> hist = inImg.get_histogram(pow(2.0, inBitPix));
        
        float sum = 0;
        cimg_forX(hist, pos) { sum += pos * hist[pos]; }
        
        float numPixels = inImg.width() * inImg.height();
        float sumB = 0, wB = 0, max = 0.0;
        float threshold1 = 0.0, threshold2 = 0.0;
        
        cimg_forX(hist, i) {
            wB += hist[i];
        
            if (! wB) { continue; }    
        
            float wF = numPixels - wB;
            
            if (! wF) { break; }
            
            sumB += i * hist[i];
        
            float mF = (sum - sumB) / wF;
            float mB = sumB / wB;
            float diff = mB - mF;
            float bw = wB * wF * pow(diff, 2.0);
            
            if (bw >= max) {
            threshold1 = i;
            if (bw > max) {
                threshold2 = i;
            }
            max = bw;            
            }
        } // end loop
        
        float th = (threshold1 + threshold2) / 2.0;
        
        *outBinImg = inImg; // Create a copy
        outBinImg->threshold(th); 
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

    void getAndRemoveNeighbours(PixelPosT inCurPixelPos, PixelPosSetT * inoutWhitePixels, PixelPosListT * inoutPixelsToBeProcessed)
    {
        const size_t _numPixels = 8, _x = 0, _y = 1;
        const int offsets[_numPixels][2] = { { -1, -1 }, { 0, -1 }, { 1, -1 },
                                            { -1, 0 },              { 1, 0 },
                                            { -1, 1 }, { 0, 1 }, { 1, 1 } };
        
        for (size_t p = 0; p < _numPixels; ++p) {
            PixelPosT curPixPos(std::get<0>(inCurPixelPos) + offsets[p][_x], std::get<1>(inCurPixelPos) + offsets[p][_y]);
            PixelPosSetT::iterator itPixPos = inoutWhitePixels->find(curPixPos);
        
            if (itPixPos != inoutWhitePixels->end()) {
            const PixelPosT & curPixPos = *itPixPos;
            inoutPixelsToBeProcessed->push_back(curPixPos);
            inoutWhitePixels->erase(itPixPos); // Remove white pixel from "white set" since it has been now processed
            }
        }
        return;
    }

    template<typename T> 
    void clusterStars(const CImg<T> & inImg, StarInfoListT * outStarInfos)
    {
        PixelPosSetT whitePixels;
        cimg_forXY(inImg, x, y) 
        {
            if (inImg(x, y))
            {
                whitePixels.insert(whitePixels.end(), PixelPosT(x, y));
            }
        }
        // Iterate over white pixels as long as set is not empty
        while (whitePixels.size()) 
        {
            PixelPosListT pixelsToBeProcessed;
            PixelPosSetT::iterator itWhitePixPos = whitePixels.begin();
            pixelsToBeProcessed.push_back(*itWhitePixPos);
            whitePixels.erase(itWhitePixPos);
            FrameT frame(inImg.width(), inImg.height(), 0, 0);
            while(! pixelsToBeProcessed.empty()) 
            {
                PixelPosT curPixelPos = pixelsToBeProcessed.front();
                // Determine boundaries (min max in x and y directions)
                if (std::get<0>(curPixelPos) /*x*/ < std::get<0>(frame) /*x1*/) 
                    std::get<0>(frame) = std::get<0>(curPixelPos);
                if (std::get<0>(curPixelPos) /*x*/ > std::get<2>(frame) /*x2*/)
                    std::get<2>(frame) = std::get<0>(curPixelPos);
                if (std::get<1>(curPixelPos) /*y*/ < std::get<1>(frame) /*y1*/)
                    std::get<1>(frame) = std::get<1>(curPixelPos);
                if (std::get<1>(curPixelPos) /*y*/ > std::get<3>(frame) /*y2*/)
                    std::get<3>(frame) = std::get<1>(curPixelPos);
                getAndRemoveNeighbours(curPixelPos, & whitePixels, & pixelsToBeProcessed);
                pixelsToBeProcessed.pop_front();
            }
            // Create new star-info and set cluster-frame.
            // NOTE: we may use new to avoid copy of StarInfoT...
            StarInfoT starInfo;
            starInfo.clusterFrame = frame;
            outStarInfos->push_back(starInfo);
        }
    }

    void ClacStarInfo(std::string ImageName,int &HFD,int &StarIndex,int &ImageWidth,int &ImageHeight)
    {
        StarInfoListT starInfos;
        CImg<float> img;
        long bitPix = 0;
        try
        {
            readFile(img, ImageName,& bitPix);
        }
        catch (FitsException &) 
        {
            cerr << "Read FITS failed." << endl;
            return;
        }
        CImg<unsigned char> rgbImg(img.width(), img.height(), 1 /*depth*/, 3 /*3 channels - RGB*/);
        float min = img.min(), mm = img.max() - min;
        
        cimg_forXY(img, x, y) 
        {
            int value = 255.0 * (img(x,y) - min) / mm;
            rgbImg(x, y, 0 /*red*/) = value;
            rgbImg(x, y, 1 /*green*/) = value;
            rgbImg(x, y, 2 /*blue*/) = value;
        }
        CImg<float> & aiImg = img.blur_anisotropic(30.0f, /*amplitude*/
                                0.7f, /*sharpness*/
                                0.3f, /*anisotropy*/
                                0.6f, /*alpha*/
                                1.1f, /*sigma*/
                                0.8f, /*dl*/
                                30,   /*da*/
                                2,    /*gauss_prec*/
                                0,    /*interpolation_type*/
                                false /*fast_approx*/
                                );
        CImg<float> binImg;
        thresholdOtsu(aiImg, bitPix, & binImg);
        const unsigned int outerDiameter = 60;
        HFD = calcHfd(img, outerDiameter);
        clusterStars(binImg, & starInfos);
        StarIndex = starInfos.size();
        ImageWidth = img.width();
        ImageHeight = img.height();
    }
}