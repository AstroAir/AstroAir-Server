#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "qhyccd.h"
#include "fitsio.h"
#include "fitsio2.h"

void SDKVersion()
{
  unsigned int  YMDS[4];
  unsigned char sVersion[80];

  memset ((char *)sVersion,0x00,sizeof(sVersion));
  GetQHYCCDSDKVersion(&YMDS[0],&YMDS[1],&YMDS[2],&YMDS[3]);

  if ((YMDS[1] < 10)&&(YMDS[2] < 10))
  {
    sprintf((char *)sVersion,"V20%d0%d0%d_%d\n",YMDS[0],YMDS[1],YMDS[2],YMDS[3]	);
  }
  else if ((YMDS[1] < 10)&&(YMDS[2] > 10))
  {
    sprintf((char *)sVersion,"V20%d0%d%d_%d\n",YMDS[0],YMDS[1],YMDS[2],YMDS[3]	);
  }
  else if ((YMDS[1] > 10)&&(YMDS[2] < 10))
  {
    sprintf((char *)sVersion,"V20%d%d0%d_%d\n",YMDS[0],YMDS[1],YMDS[2],YMDS[3]	);
  }
  else
  {
    sprintf((char *)sVersion,"V20%d%d%d_%d\n",YMDS[0],YMDS[1],YMDS[2],YMDS[3]	);
  }

  fprintf(stderr,"QHYCCD SDK Version: %s\n", sVersion);
}


void FirmWareVersion(qhyccd_handle *h)
{
  int i = 0;
  unsigned char fwv[32],FWInfo[256];
  unsigned int ret;
  memset (FWInfo,0x00,sizeof(FWInfo));
  ret = GetQHYCCDFWVersion(h,fwv);
  if(ret == QHYCCD_SUCCESS)
  {
    if((fwv[0] >> 4) <= 9)
    {

      sprintf((char *)FWInfo,"Firmware version:20%d_%d_%d\n",((fwv[0] >> 4) + 0x10),
              (fwv[0]&~0xf0),fwv[1]);

    }
    else
    {

      sprintf((char *)FWInfo,"Firmware version:20%d_%d_%d\n",(fwv[0] >> 4),
              (fwv[0]&~0xf0),fwv[1]);

    }
  }
  else
  {
    sprintf((char *)FWInfo,"Firmware version:Not Found!\n");
  }
  fprintf(stderr,"%s\n", FWInfo);

}

int CFW(int choice)
{
    int num = 0;
    qhyccd_handle *camhandle;
    int ret;
    char id[32];
    char camtype[16];
    int found = 0;
    char ch = '0';
    char currentpos[64];
    int checktimes = 0;

    ret = InitQHYCCDResource();
    if(ret == QHYCCD_SUCCESS)
    {
        printf("Init SDK success!\n");
    }
    else
    {
        goto failure;
    }
    num = ScanQHYCCD();
    if(num > 0)
    {
        printf("Yes!Found QHYCCD,the num is %d \n",num);
    }
    else
    {
        printf("Not Found QHYCCD,please check the usblink or the power\n");
 
    }

    for(int i = 0;i < num;i++)
    {
        ret = GetQHYCCDId(i,id);
        if(ret == QHYCCD_SUCCESS)
        {
            printf("connected to the first camera from the list,id is %s\n",id);
            found = 1;
            break;
        }
    }

    if(found == 1)
    {
        camhandle = OpenQHYCCD(id);
        if(camhandle != NULL)
        {
            printf("Open QHYCCD success!\n");
        }
        
        while(ch != 'e' && ch != 'E')
        {
            
            if(ch >= '0' && ch <= '8')
            {
                ret = SendOrder2QHYCCDCFW(camhandle,&ch,1);
                if(ret != QHYCCD_SUCCESS)
                {
                    printf("Control the color filter wheel failure \n");
                    goto failure;
                }
                else
                {
                    checktimes = 0;

                    while(checktimes < 90)
                    {
                        ret = GetQHYCCDCFWStatus(camhandle,currentpos);
                        if(ret == QHYCCD_SUCCESS)
                        {
                            if((ch + 1) == currentpos[0])
                            {
                                break;
                            }
                        }
                        checktimes++;
                    }
                }
            }
        }
    }
    else
    {
        printf("The camera is not QHYCCD or other error \n");
        goto failure;
    }
    
        
    ret = CloseQHYCCD(camhandle);
    if(ret == QHYCCD_SUCCESS)
    {
        printf("Close QHYCCD success!\n");
    }
    else
    {
        goto failure;
    }

    ret = ReleaseQHYCCDResource();
    if(ret == QHYCCD_SUCCESS)
    {
        printf("Rlease SDK Resource  success!\n");
    }
    else
    {
        goto failure;
    }

    return 0;

failure:
    printf("some fatal error happened\n");
    return 1;
}

int main(int argc, char *argv[])
{
  
  //QHYCCD Camera Setting
  int USB_TRAFFIC;
  int CHIP_GAIN;
  int CHIP_OFFSET;
  int EXPOSURE_TIME;
  int camBinX;
  int camBinY;
  
  int number = 1;

  FILE *pt;
  int n_frames;
  int filter,i_filter,n_filter;
  int bin;
  int k = 1;
  int i = 0;
  
  pt=fopen("qhy_filter.txt","r");  //Get filter setting
	fscanf(pt,"%d",&filter);
	fclose(pt);
  switch(filter)    //If position is {R,G,B,L,Ha,SII,OIII}
	{
  case 0:
    break;
	case 11:    //RGB
		i_filter=1;
		n_filter=3;
		break;
	case 12:    //RGBL
		i_filter=1;
		n_filter=4;
		break;
	case 13:    //RGBLHa
		i_filter=1;
		n_filter=5;
		break;
  case 14:    //RGBLHaSIIOIII
		i_filter=1;
		n_filter=7;
		break;
	case 15:    //HaSIIOIII
		i_filter=5;
		n_filter=7;
		break;
	default:
		i_filter=filter;
		n_filter=filter;
	}
  
  pt=fopen("qhy_num.txt","r");  //Get usb traffic
	fscanf(pt,"%d",&number);
	fclose(pt);
  
  pt=fopen("qhy_usb.txt","r");  //Get usb traffic
	fscanf(pt,"%d",&USB_TRAFFIC);
	fclose(pt);
  
  pt=fopen("qhy_nf.txt","r");   //Get n_frame
	fscanf(pt,"%d",&n_frames);
	fclose(pt);
  
  pt=fopen("qhy_ex.txt","r");   //Get exposure
	fscanf(pt,"%d",&EXPOSURE_TIME);
	fclose(pt);
  
  pt=fopen("qhy_gain.txt","r");   //Get gain
	fscanf(pt,"%d",&CHIP_GAIN);
	fclose(pt);
  
  pt=fopen("qhy_offset.txt","r");   //Get offset
	fscanf(pt,"%d",&CHIP_OFFSET);
	fclose(pt);
  
  pt=fopen("qhy_bin.txt","r");    //Get bin mode
	fscanf(pt,"%d",&bin);
	fclose(pt);
  if(bin == 1)
    camBinX=1,camBinY=1;
  if(bin == 2)
    camBinX=2,camBinY=2;
  if(bin == 4)
    camBinX=4,camBinY=4;

	int ra_h, ra_m, ra_s;
	int dec_sign, dec_d, dec_m, dec_s;

	char RA[100];
	char DEC[100];

	pt=fopen("coord_RA.txt","r");
	fscanf(pt,"%d\n%d\n%d",&ra_h,&ra_m,&ra_s);
	fclose(pt);

	pt=fopen("coord_DEC.txt","r");
	fscanf(pt,"%d\n%d\n%d\n%d",&dec_sign,&dec_d,&dec_m,&dec_s);
	fclose(pt);

	sprintf(RA,"%d:%d:%d",ra_h,ra_m,ra_s);/*Set RA*/

	if (dec_sign > 0){/*Set DEC*/
		sprintf(DEC,"+%d:%d:%d",dec_d,dec_m,dec_s);
	} else {
		sprintf(DEC,"-%d:%d:%d",dec_d,dec_m,dec_s);
	}

  double chipWidthMM;
  double chipHeightMM;
  double pixelWidthUM;
  double pixelHeightUM;

  unsigned int roiStartX;
  unsigned int roiStartY;
  unsigned int roiSizeX;
  unsigned int roiSizeY;

  unsigned int overscanStartX;
  unsigned int overscanStartY;
  unsigned int overscanSizeX;
  unsigned int overscanSizeY;

  unsigned int effectiveStartX;
  unsigned int effectiveStartY;
  unsigned int effectiveSizeX;
  unsigned int effectiveSizeY;

  unsigned int maxImageSizeX;
  unsigned int maxImageSizeY;
  unsigned int bpp;
  unsigned int channels;

  unsigned char *pImgData = 0;

  SDKVersion();

  SetQHYCCDBufferNumber(4096);


  // init SDK
  unsigned int retVal = InitQHYCCDResource();
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("SDK resources initialized.\n");
  }
  else
  {
    printf("Cannot initialize SDK resources, error: %d\n", retVal);
    return 1;
  }

  // scan cameras
  int camCount = ScanQHYCCD();
  if (camCount > 0)
  {
    printf("Number of QHYCCD cameras found: %d \n", camCount);
  }
  else
  {
    printf("No QHYCCD camera found, please check USB or power.\n");
    return 1;
  }

  // iterate over all attached cameras
  bool camFound = false;
  char camId[32];

  for (int i = 0; i < camCount; i++)
  {
    retVal = GetQHYCCDId(i, camId);
    if (QHYCCD_SUCCESS == retVal)
    {
      printf("Application connected to the following camera from the list: Index: %d,  cameraID = %s\n", (i + 1), camId);
      camFound = true;
      break;
    }
  }

  if (!camFound)
  {
    printf("The detected camera is not QHYCCD or other error.\n");
    // release sdk resources
    retVal = ReleaseQHYCCDResource();
    if (QHYCCD_SUCCESS == retVal)
    {
      printf("SDK resources released.\n");
    }
    else
    {
      printf("Cannot release SDK resources, error %d.\n", retVal);
    }
    return 1;
  }

  // open camera
  qhyccd_handle *pCamHandle = OpenQHYCCD(camId);
  if (pCamHandle != NULL)
  {
    printf("Open QHYCCD success.\n");
  }
  else
  {
    printf("Open QHYCCD failure.\n");
    return 1;
  }

  FirmWareVersion(pCamHandle);

  // check camera support single frame
  retVal = IsQHYCCDControlAvailable(pCamHandle, CAM_SINGLEFRAMEMODE);
  if (QHYCCD_ERROR == retVal)
  {
    printf("The detected camera is not support single frame.\n");
    // release sdk resources
    retVal = ReleaseQHYCCDResource();
    if (QHYCCD_SUCCESS == retVal)
    {
      printf("SDK resources released.\n");
    }
    else
    {
      printf("Cannot release SDK resources, error %d.\n", retVal);
    }
    return 1;
  }
  
  // set single frame mode
  int mode = 0;
  retVal = SetQHYCCDStreamMode(pCamHandle, mode);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("SetQHYCCDStreamMode set to: %d, success.\n", mode);
  }
  else
  {
    printf("SetQHYCCDStreamMode: %d failure, error: %d\n", mode, retVal);
    return 1;
  }

  // initialize camera
  retVal = InitQHYCCD(pCamHandle);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("InitQHYCCD success.\n");
  }
  else
  {
    printf("InitQHYCCD faililure, error: %d\n", retVal);
    return 1;
  }

  // get overscan area
  retVal = GetQHYCCDOverScanArea(pCamHandle, &overscanStartX, &overscanStartY, &overscanSizeX, &overscanSizeY);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("GetQHYCCDOverScanArea:\n");
    printf("Overscan Area startX x startY : %d x %d\n", overscanStartX, overscanStartY);
    printf("Overscan Area sizeX  x sizeY  : %d x %d\n", overscanSizeX, overscanSizeY);
  }
  else
  {
    printf("GetQHYCCDOverScanArea failure, error: %d\n", retVal);
    return 1;
  }

  // get effective area
  retVal = GetQHYCCDOverScanArea(pCamHandle, &effectiveStartX, &effectiveStartY, &effectiveSizeX, &effectiveSizeY);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("GetQHYCCDEffectiveArea:\n");
    printf("Effective Area startX x startY: %d x %d\n", effectiveStartX, effectiveStartY);
    printf("Effective Area sizeX  x sizeY : %d x %d\n", effectiveSizeX, effectiveSizeY);
  }
  else
  {
    printf("GetQHYCCDOverScanArea failure, error: %d\n", retVal);
    return 1;
  }

  // get chip info
  retVal = GetQHYCCDChipInfo(pCamHandle, &chipWidthMM, &chipHeightMM, &maxImageSizeX, &maxImageSizeY, &pixelWidthUM, &pixelHeightUM, &bpp);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("GetQHYCCDChipInfo:\n");
    printf("Effective Area startX x startY: %d x %d\n", effectiveStartX, effectiveStartY);
    printf("Chip  size width x height     : %.3f x %.3f [mm]\n", chipWidthMM, chipHeightMM);
    printf("Pixel size width x height     : %.3f x %.3f [um]\n", pixelWidthUM, pixelHeightUM);
    printf("Image size width x height     : %d x %d\n", maxImageSizeX, maxImageSizeY);
  }
  else
  {
    printf("GetQHYCCDChipInfo failure, error: %d\n", retVal);
    return 1;
  }

  // set ROI
  roiStartX = 0;
  roiStartY = 0;
  roiSizeX = maxImageSizeX;
  roiSizeY = maxImageSizeY;

  // check color camera
  retVal = IsQHYCCDControlAvailable(pCamHandle, CAM_COLOR);
  if (retVal == BAYER_GB || retVal == BAYER_GR || retVal == BAYER_BG || retVal == BAYER_RG)
  {
    printf("This is a color camera.\n");
    printf("even this is a color camera, in Single Frame mode THE SDK ONLY SUPPORT RAW OUTPUT.So please do not set SetQHYCCDDebayerOnOff() to true;");
    //SetQHYCCDDebayerOnOff(pCamHandle, true);
    //SetQHYCCDParam(pCamHandle, CONTROL_WBR, 20);
    //SetQHYCCDParam(pCamHandle, CONTROL_WBG, 20);
    //SetQHYCCDParam(pCamHandle, CONTROL_WBB, 20);
  }
  else
  {
    printf("This is a mono camera.\n");
  }

  // check traffic
  retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_USBTRAFFIC);
  if (QHYCCD_SUCCESS == retVal)
  {
    retVal = SetQHYCCDParam(pCamHandle, CONTROL_USBTRAFFIC, USB_TRAFFIC);
    if (QHYCCD_SUCCESS == retVal)
    {
      printf("SetQHYCCDParam CONTROL_USBTRAFFIC set to: %d, success.\n", USB_TRAFFIC);
    }
    else
    {
      printf("SetQHYCCDParam CONTROL_USBTRAFFIC failure, error: %d\n", retVal);
      getchar();
      return 1;
    }
  }

  // check gain
  retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_GAIN);
  if (QHYCCD_SUCCESS == retVal)
  {
    retVal = SetQHYCCDParam(pCamHandle, CONTROL_GAIN, CHIP_GAIN);
    if (retVal == QHYCCD_SUCCESS)
    {
      printf("SetQHYCCDParam CONTROL_GAIN set to: %d, success\n", CHIP_GAIN);
    }
    else
    {
      printf("SetQHYCCDParam CONTROL_GAIN failure, error: %d\n", retVal);
      getchar();
      return 1;
    }
  }

  // check offset
  retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_OFFSET);
  if (QHYCCD_SUCCESS == retVal)
  {
    retVal = SetQHYCCDParam(pCamHandle, CONTROL_OFFSET, CHIP_OFFSET);
    if (QHYCCD_SUCCESS == retVal)
    {
      printf("SetQHYCCDParam CONTROL_GAIN set to: %d, success.\n", CHIP_OFFSET);
    }
    else
    {
      printf("SetQHYCCDParam CONTROL_GAIN failed.\n");
      getchar();
      return 1;
    }
  }

   // set exposure time
  retVal = SetQHYCCDParam(pCamHandle, CONTROL_EXPOSURE, EXPOSURE_TIME);
  printf("SetQHYCCDParam CONTROL_EXPOSURE set to: %d, success.\n", EXPOSURE_TIME);
  if (QHYCCD_SUCCESS == retVal)
  {}
  else
  {
    printf("SetQHYCCDParam CONTROL_EXPOSURE failure, error: %d\n", retVal);
    getchar();
    return 1;
  }

  // set image resolution
  retVal = SetQHYCCDResolution(pCamHandle, roiStartX, roiStartY, roiSizeX, roiSizeY);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("SetQHYCCDResolution roiStartX x roiStartY: %d x %d\n", roiStartX, roiStartY);
    printf("SetQHYCCDResolution roiSizeX  x roiSizeY : %d x %d\n", roiSizeX, roiSizeY);
  }
  else
  {
    printf("SetQHYCCDResolution failure, error: %d\n", retVal);
    return 1;
  }

  // set binning mode
  retVal = SetQHYCCDBinMode(pCamHandle, camBinX, camBinY);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("SetQHYCCDBinMode set to: binX: %d, binY: %d, success.\n", camBinX, camBinY);
  }
  else
  {
    printf("SetQHYCCDBinMode failure, error: %d\n", retVal);
    return 1;
  }

  retVal = SetQHYCCDParam(pCamHandle, CONTROL_DDR, 1.0);

  // set bit resolution
  retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_TRANSFERBIT);
  if (QHYCCD_SUCCESS == retVal)
  {
    retVal = SetQHYCCDBitsMode(pCamHandle, 16);
    if (QHYCCD_SUCCESS == retVal)
    {
      printf("SetQHYCCDParam CONTROL_GAIN set to: %d, success.\n", CONTROL_TRANSFERBIT);
    }
    else
    {
      printf("SetQHYCCDParam CONTROL_GAIN failure, error: %d\n", retVal);
      getchar();
      return 1;
    }
  }

  while(i<n_frames){
  // single frame
  number++;
 
  printf("ExpQHYCCDSingleFrame(pCamHandle) - start...\n");
  retVal = ExpQHYCCDSingleFrame(pCamHandle);
  printf("ExpQHYCCDSingleFrame(pCamHandle) - end...\n");
  if (QHYCCD_ERROR != retVal)
  {
    printf("ExpQHYCCDSingleFrame success.\n");
    if (QHYCCD_READ_DIRECTLY != retVal)
    {
      sleep(1);
    }
  }
  else
  {
    printf("ExpQHYCCDSingleFrame failure, error: %d\n", retVal);
    return 1;
  }

  // get requested memory lenght
  uint32_t length = GetQHYCCDMemLength(pCamHandle);

  if (length > 0)
  {
    pImgData = new unsigned char[length];
    memset(pImgData, 0, length);
    printf("Allocated memory for frame: %d [uchar].\n", length);
  }
  else
  {
    printf("Cannot allocate memory for frame.\n");
    return 1;
  }

  // get single frame
  retVal = GetQHYCCDSingleFrame(pCamHandle, &roiSizeX, &roiSizeY, &bpp, &channels, pImgData);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("GetQHYCCDSingleFrame: %d x %d, bpp: %d, channels: %d, success.\n", roiSizeX, roiSizeY, bpp, channels);
    //process image here
  /*
	time_t timep;
	struct tm *p;
	time (&timep);
	p = gmtime(&timep);
	//20200824162940
	long timeNum = (1900 + p->tm_year) * 10000000000 + (1 + p->tm_mon) * 100000000 + p->tm_mday * 1000000 + (8 + p->tm_hour) * 10000 + p->tm_min * 100 + p->tm_sec;
	*/
  char buffer[17] = {0};
	char nameBuf[31] = "/var/www/html/";
  sprintf(buffer, "Image_%d",number);
	//sprintf(buffer, "%5d", timeNum);
	strcat(buffer, ".fit");
	strcat(nameBuf, buffer);
	//printf("%s\n",nameBuf); /*获取bai当du前秒*/

	fitsfile *fptr;
	int status;
	long fpixel = 1, naxis = 2, nelements;
	long naxes[2] = { roiSizeX, roiSizeY };

  //Camera type
	char keywords[40];
  char description[40];
  //Mount RA&DEC
  char keywords1[40];
  char description1[40];
  char target[40];
  //Pixel size
  char keywords2[40];
  char description2[40];
  char ccd_px[40];
  //Mount RA
  char keywords3[40];
  char description3[40];

  //Mount DEC
  char keywords4[40];
  char description4[40];

  
  
	char datatype[40];

	status = 0;

	fits_create_file(&fptr, nameBuf, &status);

	printf("bpp = %d\n", bpp);
	if(bpp == 16)
		fits_create_img(fptr, USHORT_IMG, naxis, naxes, &status);
	else
		fits_create_img(fptr, BYTE_IMG,   naxis, naxes, &status);
  
  sprintf(ccd_px, "%.3f x %.3f",pixelWidthUM, pixelHeightUM);
	strcpy(keywords, "QHYCCD");
  strcpy(keywords1, "Target");
  strcpy(keywords2, "Pixel size");
  strcpy(keywords3, "Mount RA");
  strcpy(keywords4, "Mount DEC");
	strcpy(datatype, "TSTRING");
	strcpy(description, "Iamge capture camera");
  strcpy(target, "11:11:11 +22:22:22");

  strcpy(description1, "Mount's name");
  strcpy(description2, "Camera informations");
  strcpy(description3, "Mount's RA");
  strcpy(description4, "Mount's DEC");
	if(strcmp(datatype, "TSTRING") == 0)
	{
		fits_update_key(fptr, TSTRING, keywords, &camId, description, &status);
    fits_update_key(fptr, TSTRING, keywords2 , &ccd_px ,description2, &status);
    fits_update_key(fptr, TSTRING, keywords1 , &target ,description1, &status);
    fits_update_key(fptr, TSTRING, keywords3 , &RA ,description3, &status);
    fits_update_key(fptr, TSTRING, keywords4 , &DEC ,description4, &status);
	}
	else if(strcmp(datatype, "TINT"))
	{
		int valInt = 1;
		fits_update_key(fptr, TINT, keywords, &camId, description, &status);
    fits_update_key(fptr, TINT, keywords2 , &ccd_px ,description2, &status);
    fits_update_key(fptr, TINT, keywords1 , &target ,description1, &status);
    fits_update_key(fptr, TINT, keywords3 , &RA ,description3, &status);
    fits_update_key(fptr, TINT, keywords4 , &DEC ,description4, &status);
	}
	else if(strcmp(datatype, "TDOUBLE") == 0)
	{
		double valDouble = 1.0;
		fits_update_key(fptr, TDOUBLE, keywords, &camId, description, &status);
    fits_update_key(fptr, TDOUBLE, keywords2 , &ccd_px ,description2, &status);
    fits_update_key(fptr, TDOUBLE, keywords1 , &target ,description1, &status);
    fits_update_key(fptr, TDOUBLE, keywords3 , &RA ,description3, &status);
    fits_update_key(fptr, TDOUBLE, keywords4 , &DEC ,description4, &status);
	}
	else if(strcmp(datatype, "TLOGICAL") == 0)
	{
		fits_update_key(fptr, TLOGICAL, keywords, &camId, description, &status);
    fits_update_key(fptr, TLOGICAL, keywords2 , &ccd_px ,description2, &status);
    fits_update_key(fptr, TLOGICAL, keywords1 , &target ,description1, &status);
    fits_update_key(fptr, TLOGICAL, keywords3 , &RA ,description3, &status);
    fits_update_key(fptr, TLOGICAL, keywords4 , &DEC ,description4, &status);
	}

	nelements = naxes[0] * naxes[1];
	printf("nelements = %ld\n", nelements);

	if(bpp == 16)
		fits_write_img(fptr, TUSHORT, fpixel, nelements, &pImgData[0], &status);
	else
		fits_write_img(fptr, TBYTE, fpixel, nelements, &pImgData[0], &status);

	fits_close_file(fptr, &status);
	fits_report_error(stderr, status);
  }
  else
  {
    printf("GetQHYCCDSingleFrame failure, error: %d\n", retVal);
  }

  delete [] pImgData;

  i++;
}
  

  pt=fopen("qhy_num.txt","w");
	fprintf(pt,"%d",number);
	fclose(pt);
  retVal = CancelQHYCCDExposingAndReadout(pCamHandle);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("CancelQHYCCDExposingAndReadout success.\n");
  }
  else
  {
    printf("CancelQHYCCDExposingAndReadout failure, error: %d\n", retVal);
    return 1;
  }
  
  // close camera handle
  retVal = CloseQHYCCD(pCamHandle);
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("Close QHYCCD success.\n");
  }
  else
  {
    printf("Close QHYCCD failure, error: %d\n", retVal);
  }

  // release sdk resources
  retVal = ReleaseQHYCCDResource();
  if (QHYCCD_SUCCESS == retVal)
  {
    printf("SDK resources released.\n");
  }
  else
  {
    printf("Cannot release SDK resources, error %d.\n", retVal);
    return 1;
  }

  return 0;

}
