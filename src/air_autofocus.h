/*
 * air_autofocus.h
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
 
Date:2021-6-24
 
Description:Focus offical port(Auto)

**************************************************/

#ifndef _AIR_AUTOFOCUS_H_
#define _AIR_AUTOFOCUS_H_

#include <tuple>
#include <list>
#include <set>
#include <CImg.h>
#include <gsl/gsl_multifit_nlin.h>

using namespace cimg_library;

typedef std::tuple<int /*x*/,int /*y*/> PixelPosT;
typedef std::set<PixelPosT> PixelPosSetT;
typedef std::list<PixelPosT> PixelPosListT;
typedef std::tuple<float, float> PixSubPosT;
typedef std::tuple<float /*x1*/, float /*y1*/, float /*x2*/, float /*y2*/> FrameT;
/*星体信息*/
struct StarInfoT
{
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


struct DataPointT {
  float x;
  float y;
  DataPointT(float inX = 0, float inY = 0) : x(inX), y(inY) {}
};
  
typedef std::vector<DataPointT> DataPointsT;
  
struct GslMultiFitDataT {
  float y;
  float sigma;
  DataPointT pt;
};
  
typedef std::vector<GslMultiFitDataT> GslMultiFitParmsT;

namespace AstroAir
{
    class AUTOFOCUS
    {
        public:
            bool SolveImageInfo(std::string FileName);          //处理图像信息
            typedef std::list<StarInfoT> StarInfoListT;
            /*摘自https://www.lost-infinity.com/night-sky-image-processing-part-7-automatic-star-recognizer/*/
            virtual bool insideCircle(float inX /*pos of x*/, float inY /*pos of y*/, float inCenterX, float inCenterY, float inRadius);
            void readFile(CImg<float> &inImg, const std::string &inFilename, long *outBitPix = 0);
            void thresholdOtsu(const CImg<float> &inImg, long inBitPix, CImg<float> *outBinImg);
            void getAndRemoveNeighbours(PixelPosT inCurPixelPos, PixelPosSetT *inoutWhitePixels, PixelPosListT *inoutPixelsToBeProcessed);
            template<typename T>
            void clusterStars(const CImg<T> &inImg, StarInfoListT *outStarInfos);
            float calcIx2(const CImg<float> &img, int x);
            float calcJy2(const CImg<float> &img, int y);
            void calcIntensityWeightedCenter(const CImg<float> &inImg, float *outX, float *outY);
            void calcSubPixelCenter(const CImg<float> &inImg, float *outX, float *outY, size_t inNumIter = 10 /*num iterations*/);
            void calcCentroid(const CImg<float> &inImg, const FrameT &inFrame, PixSubPosT *outPixelPos, PixSubPosT *outSubPixelPos = 0, size_t inNumIterations = 10);
            float calcHfd(const CImg<float> &inImage, unsigned int inOuterDiameter);
        private:
            double hfd_temp;
            

    };

    extern int StarIndex,AImageWeight,AImageHeight;
    extern double HFD;
    extern AUTOFOCUS *autofocus;
}

#endif