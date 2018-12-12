/*
 * main.c
 *
 *  Created on: Aug 26, 2014
 *      Author: jason
 */

#include <stdio.h>
#include "JpegRtpApi.h"

#define PIXFMT (YUV422)
#define Q     (50)
#define FRAMES (1000)
#define FPS (60)

// enum JPEGRTPSTATUS ReadYUVFile(const char*src, struct JpegEncBuffer *buf)
// {
	// int	    ret;
	// FILE	*fpr;
	// fpr = fopen(src, "r");
	// if (NULL == fpr) {
		// printf("can't open %s for read\n", src);
		// return FILEERROR;
	// }
	
	// ret = fread(buf->Ybuf,buf->width*buf->height, 1, fpr);
	// if(buf->pixfmt==YUV444) {
		// ret = fread(buf->Cbbuf,buf->width*buf->height, 1, fpr);
		// ret = fread(buf->Crbuf,buf->width*buf->height, 1, fpr);
	// } else if(buf->pixfmt==YUV422) {
		// ret = fread(buf->Cbbuf,buf->width*buf->height/2, 1, fpr);
		// ret = fread(buf->Crbuf,buf->width*buf->height/2, 1, fpr);		
	// } else if(buf->pixfmt==YUV420) {
		// ret = fread(buf->Cbbuf,buf->width*buf->height/4, 1, fpr);
		// ret = fread(buf->Crbuf,buf->width*buf->height/4, 1, fpr);		
	// }
// }
enum JPEGRTPSTATUS ReadYUVFile(FILE	*fpr, struct JpegEncBuffer *buf)
{
	int	    ret;
	
	
	ret = fread(buf->Ybuf,buf->width*buf->height, 1, fpr);
	if(buf->pixfmt==YUV444) {
		ret = fread(buf->Cbbuf,buf->width*buf->height, 1, fpr);
		ret = fread(buf->Crbuf,buf->width*buf->height, 1, fpr);
	} else if(buf->pixfmt==YUV422) {
		ret = fread(buf->Cbbuf,buf->width*buf->height/2, 1, fpr);
		ret = fread(buf->Crbuf,buf->width*buf->height/2, 1, fpr);		
	} else if(buf->pixfmt==YUV420) {
		ret = fread(buf->Cbbuf,buf->width*buf->height/4, 1, fpr);
		ret = fread(buf->Crbuf,buf->width*buf->height/4, 1, fpr);		
	}
}

enum JPEGRTPSTATUS YUVToJpeg(const char*src, const char *dst, unsigned int width, unsigned int height, enum PixFmt pixfmt, unsigned char QP)
{
	struct JPEG jpeg;
	struct JpegEncBuffer input;
	enum JPEGRTPSTATUS ret;
	

	ret=JpegEncBufferCreate(&input,QP,width,height,pixfmt);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegEncBufferCreate error: %d\n",ret);
		goto out;
	}
	FILE	*fpr;
	fpr = fopen(src, "r");
	if (NULL == fpr) {
		printf("can't open %s for read\n", src);
		return FILEERROR;
	}
	
	ReadYUVFile(fpr, &input);
	
	ret=JpegEncCode(&jpeg,&input);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JpegEncCodeError: %d\n",ret);
		goto out;
	}
	
	JpegInfoShow(&jpeg);
	
	ret=JpegInfoPutToFile(&jpeg,dst);
	if(ret!=JPEGRTPSTATUSOK)
	{
		printf("JJpegInfoPutToFile: %d\n",ret);
		goto out;
	}

out:
	JpegInfoDestroy(&jpeg);
	JpegEncBufferDestroy(&input);
	return ret;

}

int main(int argc, char **argv)
{
	if(argc<5)
		{
		printf("arg error \n");
		printf("src dst w h chroma Q \n");
		printf("usage: create a Jpeg from YUV file \n");
		return;
	} 
	enum PixFmt pixfmt = YUV444;
	unsigned char QP = 50;
	if( argc>5)
		pixfmt = atoi(argv[5]);
	if( argc>6)
		QP = atoi(argv[6]);
	
		
	YUVToJpeg(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]), pixfmt, QP);
}

// int main(int argc, char **argv)
// {
	// BMPToJpeg(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
// }




int main1(int argc, char **argv)
{
	
	if(argc<4)
		{
		printf("arg error \n");
		printf("src chroma w h Q \n");
		printf("usage: create a Jpeg from YUV file \n");
		return;
	} 
	enum PixFmt pixfmt = YUV422;
	unsigned char QP = 50;
	if( argc>4)
		pixfmt = atoi(argv[4]);
	if( argc>5)
		QP = atoi(argv[5]);
	
	const char*src = argv[1];
	int width =  atoi(argv[2]);
	int height = atoi(argv[3]);

	int i;
	
	char  bytes_8MB[20] = { 0x28, 0xA0, 0x02, 0x8A, 0x00, 
							0x28, 0xA0, 0x02, 0x8A, 0x00, 
							0x28, 0xA0, 0x02, 0x8A, 0x00, 
							0x28, 0xA0, 0x02, 0x8A, 0x00};
	int j;
	enum JPEGRTPSTATUS  ret;
	struct JPEG jpeg;
	struct JpegEncBuffer input;
	struct RTPJpegFrame jpegframe;
	struct BMP bmp;

	int sock;

	char file[1024];

	// sock=RTPConnetInit("255.255.255.255",39630);
	sock=RTPConnetInit("192.168.0.145",39630);
	if(sock<0)
	{
		printf("RTPConnetInit error!\n");
		return -1;
	}
	memset(&jpegframe, 0, sizeof(struct RTPJpegFrame));
	RTPHeadInit(&jpegframe.rtphead, 0, 0, 0);

	JpegEncInit();
	FILE	*fpr;
	fpr = fopen(src, "r");
	if (NULL == fpr) {
		printf("can't open %s for read\n", src);
		return FILEERROR;
	}
	
	for(i=1;i<FRAMES;i++)
	{
		// sprintf(file,"/root/capture/1 (%d).bmp",i);
		// printf("%s\n",file);

		// ret=BMPInfoGetFromFile(&bmp, file);
		// if(ret!=JPEGRTPSTATUSOK)
		// {
			// printf("BMPInfoGetFromFile Error: %d\n",ret);
			// goto out;
		// }
		
		
		ret=JpegEncBufferCreate(&input,Q, width,height,PIXFMT);
		if(ret!=JPEGRTPSTATUSOK)
		{
			printf("JpegEncBufferCreate Error: %d\n",ret);
			goto out;
		}

		// ReadYUVFile(src, &input);
		ReadYUVFile(fpr, &input);

		ret=JpegEncCode(&jpeg,&input);
		
		// jpeg.ScanSize = 0;
		// for (j=0; j<width*height/16/8/8; j++) {
			// memcpy(jpeg.ScanData + j*20, bytes_8MB, 20);
			// jpeg.ScanSize += 20; 
		// }
		
		
		if(ret!=JPEGRTPSTATUSOK)
		{
			printf("JpegEncCodeError: %d\n",ret);
			goto out;
		}
		ret=RTPJpegFrameInitFromJpeg(&jpeg, &jpegframe);
		if(ret!=JPEGRTPSTATUSOK)
		{
			printf("RTPJpegFrameInit error %d\n",ret);
			return -1;
		}
		// sleep(1);
		usleep(10000);
		RTPJpegFrameShow(&jpegframe);
		RTPJpegFrameSend(&jpegframe, 90000/FPS);
		// JpegInfoDestroy(&jpeg);
		// JpegEncBufferDestroy(&input);
		// BMPInfoDestroy(&bmp);
	}

out:
	JpegInfoDestroy(&jpeg);
	JpegEncBufferDestroy(&input);
	return 0;
}

// int main1(int argc, char **argv)
// {

	// int i;

	// enum JPEGRTPSTATUS  ret;
	// struct JPEG jpeg;
	// struct JpegEncBuffer input;
	// struct RTPJpegFrame jpegframe;
	// struct BMP bmp;

	// int sock;

	// char file[1024];

	// sock=RTPConnetInit("192.168.100.100",6666);
	// if(sock<0)
	// {
		// printf("RTPConnetInit error!\n");
		// return -1;
	// }
	// memset(&jpegframe, 0, sizeof(struct RTPJpegFrame));
	// RTPHeadInit(&jpegframe.rtphead, 0, 0, 0);

	// JpegEncInit();
	// for(i=1;i<FRAMES;i++)
	// {
		// sprintf(file,"/root/capture/1 (%d).bmp",i);
		// printf("%s\n",file);

		// ret=BMPInfoGetFromFile(&bmp, file);
		// if(ret!=JPEGRTPSTATUSOK)
		// {
			// printf("BMPInfoGetFromFile Error: %d\n",ret);
			// goto out;
		// }

		// ret=JpegEncBufferCreate(&input,Q,bmp.width,bmp.height,PIXFMT);
		// if(ret!=JPEGRTPSTATUSOK)
		// {
			// printf("JpegEncBufferCreate Error: %d\n",ret);
			// goto out;
		// }

		// switch(input.pixfmt)
		// {
		// case YUV444:
			// RGB24toYUV444(bmp.RGB, &input);break;
		// case YUV422:
			// RGB24toYUV422(bmp.RGB, &input);break;
		// case YUV420:
			// RGB24toYUV420(bmp.RGB, &input);break;
		// default:goto out;break;
		// }


		// ret=JpegEncCode(&jpeg,&input);
		// if(ret!=JPEGRTPSTATUSOK)
		// {
			// printf("JpegEncCodeError: %d\n",ret);
			// goto out;
		// }
		// ret=RTPJpegFrameInitFromJpeg(&jpeg, &jpegframe);
		// if(ret!=JPEGRTPSTATUSOK)
		// {
			// printf("RTPJpegFrameInit error %d\n",ret);
			// return -1;
		// }

		// RTPJpegFrameSend(&jpegframe, 90000/FPS);
		// JpegInfoDestroy(&jpeg);
		// JpegEncBufferDestroy(&input);
		// BMPInfoDestroy(&bmp);
	// }

// out:
	// JpegInfoDestroy(&jpeg);
	// JpegEncBufferDestroy(&input);
	// return 0;
// }






