#include "JsonStream.h"
#include "../../tools/KMP.h"

#define VSK_DATAHEARD 0x000001F0 
#define FREAMENDCODE  0x000001ED
#define VSK_DATAHEARDEX 0x000001FE


#define VKDATA2HEARDLEN 4
#define CODECTYPELEN 2
#define FRAMETYPELEN 2
#define GAPMSLEN     2
#define FRAMERATELEN 2
#define FRAMELENLEN  4

#define VIDEOWIDTHLEN 4
#define VIDEOHEIGHTLEN 4
#define TIMESTAMPLEN   8

#define AUDIOCHLLEN 1
#define AUDIORATELEN 4

#define VKDATA2ENDLEN 4
static unsigned char VISKINGFILEHEARD_2[VKDATA2HEARDLEN + 1] = { 0x00, 0x00, 0x01, 0xF0 };
static unsigned char VISKINGFILEHEARD_3[VKDATA2HEARDLEN + 1] = { 0x00, 0x00, 0x01, 0xFE };
static unsigned char UCHFREAMENDCODE[VKDATA2ENDLEN + 1] = { 0x00, 0x00, 0x01, 0xED };
static unsigned char VISKINGFILEHEARD[3] = { 0x00, 0x00, 0x01 };
void JsonStream::DataHeard::Clear()
{
	memset(this, 0, sizeof(DataHeard));
}

bool JsonStream::DataHeard::IsVideo()
{
	switch (codect)
	{
	case PT_AC3:
	case PT_PCMA:
	case PT_AAC:
	case PT_PCMU:
	case PT_G726_16:
	case PT_G726_24:
	case PT_G726_32:
	case PT_G726_40:
		return false;
	default:
		return true;
	}
}

bool JsonStream::DataHeard::IsPointCloud()
{
	return codect >= PT_VSK_POINT_CLOUD_XYZIT && codect <= PT_VSK_POINT_CLOUD_MAX;
}

bool JsonStream::DataHeard::IsKeyFram()
{
	switch (codect)
	{
	case PT_H264:
		if (framet == H264E_NALU_SPS || framet == H264E_NALU_PPS || framet == H264E_NALU_ISLICE)
		{
			return true;
		}
		else
		{
			return false;
		}
	case PT_H265:
		if (framet == H265_NALU_VPS || framet == H265_NALU_SPS || framet == H265_NALU_PPS || framet == H265_NALU_ISLICE || framet == H265_NALU_ISLICE_MAX)
		{
			return true;
		}
		else
		{
			return false;
		}
	case PT_AC3:
	case PT_PCMA:
	case PT_AAC:
	case PT_PCMU:
	case PT_G726_16:
	case PT_G726_24:
	case PT_G726_32:
	case PT_G726_40:
		return false;
	default:
		return true;
	}
}

bool JsonStream::DataHeard::IsPFram()
{
	switch (codect)
	{
	case PT_H264:
		if (framet == H264E_NALU_PSLICE)
		{
			return true;
		}
		break;
	case PT_H265:
		if (framet >= H265_NALU_PSLICE && framet <= H265_NALU_PSLICE_MAX)
		{
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}

bool JsonStream::DataHeard::IsKeyFramStart()
{
	switch (codect)
	{
	case PT_H264:
		if (framet == H264E_NALU_SPS)
		{
			return true; // sps,pps, i
		}
		break;
	case PT_H265:
		if (framet == H265_NALU_VPS || framet == H265_NALU_SPS)
		{
			return true;  // vps, sps,pps, i
		}
		break;
	case PT_VSK_POINT_CLOUD_XYZIT:
	case PT_VSK_POINT_CLOUD_DRACO:
		return true;
		break;
	default:
		return false;
	}

	return false;
}

unsigned int JsonStream::DataHeard::GetHeaderLen()
{
	return 0;// FrameHeardLen(codect, framet);
}

unsigned int JsonStream::DataHeard::GetFrameBufSize()
{
	return 0;// GetHeaderLen() + framelen + VKDATA2ENDLEN;
}

JsonStream::JsonStream(const char* devId, const char* streamId) :MediaStream(devId, streamId), mFrame(512 * 1024), lastTime(0), curTime(0), nFrameNum(0), frameRate(0)
{
}
JsonStream::~JsonStream()
{

}
time_t JsonStream::LastFrameTime()
{
	return curTime;
}

void CALLBACK JsonStream::VskX86NvrRtPreDataCb(unsigned int PlayHandle, uint8_t* pBuffer, unsigned int BufferSize, unsigned int DateType, time_t systime, unsigned int TimeSpace, void *pUser)
{
	JsonStream* pThis = (JsonStream*)pUser;
	if (pThis)
	{
		pThis->OnVskJsonStream(pBuffer, (size_t)BufferSize);
	}
}
void CALLBACK JsonStream::DataPlayCallBack(unsigned int PlayHandle, unsigned int DateType, uint8_t* pBuffer, unsigned int BufferSize, void* pUser)
{
	JsonStream* pThis = (JsonStream*)pUser;
	if (pThis)
	{
		pThis->OnVskJsonStream(pBuffer, (size_t)BufferSize);
	}
}
void CALLBACK JsonStream::PlayBackEndCb(unsigned int pbhandle, int errorcode, void* puser)
{

}
void JsonStream::OnVskJsonStream(uint8_t* data, size_t size)
{
	curTime = time(0);
	if (lastTime == 0)
	{
		lastTime = time(0);
	}
	if (curTime - lastTime > 5)
	{
		int interval = (std::chrono::duration_cast<std::chrono::milliseconds>(latestTime - firstTime)).count();
		if (getFrameRate() == 0)
			frameRate = nFrameNum * 1000 / (interval);
		else
			frameRate = getFrameRate();
		//printf("recved json stream data frame rate:%u, frame num:%ju, interval:%d, last:%ld,cur:%ld\n", frameRate, nFrameNum, interval, lastTime, curTime);
		lastTime = curTime;
	}
	DataHeard xDataHeard;
	xDataHeard.Clear();

	unsigned int skipsize = 0;
	int ret = PraseFrameHeard(data, size, &xDataHeard, &skipsize);
	if (ret == _VKDATA_OK && (int)xDataHeard.framelen > 0)
	{
		/*VideoNoViskHeadFrameInfo xStream;
		memset(&xStream, 0, sizeof(xStream));
		xStream.nEncodeType = xDataHeard.codect;
		xStream.nFrameType = xDataHeard.framet;
		xStream.nGapMs = xDataHeard.gapms;
		xStream.nFrameRate = xDataHeard.framerate;*/
		if (IskeyFrame(xDataHeard) == 1)	// sps, i帧才能获取时间, pps没有时间戳
		{
			/*xStream.nWidth = xDataHeard.KeyFramE.vwidth;
			xStream.nHeight = xDataHeard.KeyFramE.vheight;
			xStream.nTimeStamp = (uint32)xDataHeard.KeyFramE.timestamp;*/
		}
		else if (!xDataHeard.IsVideo())
		{
			/*xStream.nAudioChs = xDataHeard.audio.chl;
			xStream.nSamplingRate = xDataHeard.audio.frate;*/
		}

		unsigned int heardlen = FrameHeardLen(xDataHeard.codect, xDataHeard.framet);

		if (xDataHeard.codect == PT_H264)
		{
			if (xDataHeard.framet == H264E_NALU_SPS || xDataHeard.framet == H264E_NALU_PPS)
			{
				mFrame.writeBuf((char*)(data + skipsize + heardlen), xDataHeard.framelen);
			}
			else
			{
				if (nFrameNum == 0)
				{
					firstTime = std::chrono::steady_clock::now();
				}
				nFrameNum++;
				latestTime = std::chrono::steady_clock::now();
				int gap = 0;
				if (frameRate == 0)
				{
					if (xDataHeard.framerate > 0)
					{
						gap = 1000 / xDataHeard.framerate;
					}
					else
					{
						gap = xDataHeard.gapms > 0 ? xDataHeard.gapms : 40;
					}
				}
				else
				{
					gap = 1000 / frameRate;
				}
				
				if (xDataHeard.framet == H264E_NALU_ISLICE)
				{
					//printf("json idr:%ld, gapms:%d rate:%d, utc:%ju\n", time(0), gap, frameRate, xDataHeard.KeyFramE.timestamp);
					if (mFrame.freeSize() < xDataHeard.framelen)
					{
						mFrame.resetBuf(xDataHeard.framelen * 2);
					}
					mFrame.writeBuf((char*)(data + skipsize + heardlen), xDataHeard.framelen);
					OnMediaStream(switchFromToGB(xDataHeard.codect), (uint8_t*)mFrame.data(), mFrame.dataSize(), gap);
					mFrame.clear();
				}
				else
				{
					OnMediaStream(switchFromToGB(xDataHeard.codect), data + skipsize + heardlen, xDataHeard.framelen, gap);
				}
			}
		}
		else if (xDataHeard.codect == PT_H265)
		{
			if (xDataHeard.framet == H265_NALU_VPS || xDataHeard.framet == H265_NALU_SPS || xDataHeard.framet == H265_NALU_PPS)
			{
				mFrame.writeBuf((char*)(data + skipsize + heardlen), xDataHeard.framelen);
			}
			else
			{
				if (nFrameNum == 0)
				{
					firstTime = std::chrono::steady_clock::now();
				}
				nFrameNum++;
				latestTime = std::chrono::steady_clock::now();
				int gap = 0;
				if (frameRate == 0)
				{
					if (xDataHeard.framerate > 0)
					{
						gap = 1000 / xDataHeard.framerate;
					}
					else
					{
						gap = xDataHeard.gapms > 0 ? xDataHeard.gapms : 40;
					}
				}
				else
				{
					gap = 1000 / frameRate;
				}
				if (xDataHeard.framet == H265_NALU_ISLICE || xDataHeard.framet == H265_NALU_ISLICE_MAX)
				{
					if (mFrame.freeSize() < xDataHeard.framelen)
					{
						mFrame.resetBuf(xDataHeard.framelen * 2);
					}
					mFrame.writeBuf((char*)(data + skipsize + heardlen), xDataHeard.framelen);
					OnMediaStream(switchFromToGB(xDataHeard.codect), (uint8_t*)mFrame.data(), mFrame.dataSize(), gap);
					mFrame.clear();
				}
				else
				{
					OnMediaStream(switchFromToGB(xDataHeard.codect), data + skipsize + heardlen, xDataHeard.framelen, gap);
				}
			}
		}
		else
		{

		}
		/*xStream.pBuf = pBuffer + skipsize + heardlen;
		xStream.nFrameLen = xDataHeard.framelen;

		xStream.nUserData = pLinkData->mnUserData;
		xStream.pUser = pLinkData->mpUser;
		xStream.nPlayHandle = PlayHandle;*/

		//OnMediaStream(switchFromToGB(xDataHeard.codect), data + skipsize + heardlen, xDataHeard.framelen, xDataHeard.gapms > 0 ? xDataHeard.gapms : 40);
	}
	else
	{
		printf("parse vsk frame header failed %02x %02x %02x %02x\r\n", data[0], data[1], data[2], data[3]);
	}
}
int JsonStream::PraseFrameHeard(unsigned char* pbuf, unsigned int buflen, DataHeard* pheard/*out*/, unsigned int* skipsize/*out*/)
{
	int temnstartpos = MatchDataHeard(pbuf, buflen); //KMPMatch(pbuf, buflen, VISKINGFILEHEARD_2, VKDATA2HEARDLEN);//
	if (temnstartpos >= 0)
	{
		*skipsize = temnstartpos;
		int ret = ReadBasicHeardFormBuf(pbuf + temnstartpos, buflen - temnstartpos, pheard);
		if (ret != 0)
		{
			return ret;
		}
		unsigned headlen = basicHeardLen();
		unsigned char* pheardstartbuf = pbuf + temnstartpos;

		switch (pheard->codect) //一般SPS和pps I帧 组成一个IDR帧
		{
		case PT_H264:
		{
			if (pheard->framet == H264E_NALU_SPS) //sps 里面才有帧的 高和宽
			{
				//加数据宽 高
				//return basicHeardLen() + 4 + 4;
				if (temnstartpos + headlen + VIDEOWIDTHLEN + VIDEOHEIGHTLEN > buflen)
				{
					return _VKDATA_NOTENOUGH;//数据 不够
				}
				memcpy(&pheard->KeyFramE.vwidth, pheardstartbuf + headlen, VIDEOWIDTHLEN);
				headlen += VIDEOWIDTHLEN;

				memcpy(&pheard->KeyFramE.vheight, pheardstartbuf + headlen, VIDEOHEIGHTLEN);
				headlen += VIDEOHEIGHTLEN;


				memcpy(&pheard->KeyFramE.timestamp, pheardstartbuf + headlen, TIMESTAMPLEN);
				headlen += TIMESTAMPLEN;
				//*havereadsize = temnstartpos + headlen;
				return _VKDATA_OK;
			}
			else if (pheard->framet == H264E_NALU_ISLICE)
			{
				//加数据宽 高 utc 时间
				if (temnstartpos + headlen + VIDEOWIDTHLEN + VIDEOHEIGHTLEN + TIMESTAMPLEN > buflen)
				{
					return _VKDATA_NOTENOUGH;//数据 不够
				}
				memcpy(&pheard->KeyFramE.vwidth, pheardstartbuf + headlen, VIDEOWIDTHLEN);
				headlen += VIDEOWIDTHLEN;

				memcpy(&pheard->KeyFramE.vheight, pheardstartbuf + headlen, VIDEOHEIGHTLEN);
				headlen += VIDEOHEIGHTLEN;

				memcpy(&pheard->KeyFramE.timestamp, pheardstartbuf + headlen, TIMESTAMPLEN);
				headlen += TIMESTAMPLEN;

				//*havereadsize = temnstartpos + headlen;
				return _VKDATA_OK;
			}
			else
			{
				//*havereadsize = temnstartpos + headlen;
				return _VKDATA_OK;
			}
			break;
		}
		case PT_H265:
		{
			if (pheard->framet == H265_NALU_VPS || pheard->framet == H265_NALU_SPS)
			{
				if (temnstartpos + headlen + VIDEOWIDTHLEN + VIDEOHEIGHTLEN > buflen)
				{
					return _VKDATA_NOTENOUGH;//数据 不够
				}
				memcpy(&pheard->KeyFramE.vwidth, pheardstartbuf + headlen, VIDEOWIDTHLEN);
				headlen += VIDEOWIDTHLEN;

				memcpy(&pheard->KeyFramE.vheight, pheardstartbuf + headlen, VIDEOHEIGHTLEN);
				headlen += VIDEOHEIGHTLEN;

				memcpy(&pheard->KeyFramE.timestamp, pheardstartbuf + headlen, TIMESTAMPLEN);
				headlen += TIMESTAMPLEN;
				//*havereadsize = temnstartpos + headlen;
				return _VKDATA_OK;
			}
			else if (pheard->framet == H265_NALU_ISLICE || pheard->framet == H265_NALU_ISLICE_MAX)
			{
				//加数据宽 高 utc 时间
				if (temnstartpos + headlen + VIDEOWIDTHLEN + VIDEOHEIGHTLEN + TIMESTAMPLEN > buflen)
				{
					return _VKDATA_NOTENOUGH;//数据 不够
				}
				memcpy(&pheard->KeyFramE.vwidth, pheardstartbuf + headlen, VIDEOWIDTHLEN);
				headlen += VIDEOWIDTHLEN;

				memcpy(&pheard->KeyFramE.vheight, pheardstartbuf + headlen, VIDEOHEIGHTLEN);
				headlen += VIDEOHEIGHTLEN;

				memcpy(&pheard->KeyFramE.timestamp, pheardstartbuf + headlen, TIMESTAMPLEN);
				headlen += TIMESTAMPLEN;

				//*havereadsize = temnstartpos + headlen;
				return _VKDATA_OK;
			}
			else
			{
				//*havereadsize = temnstartpos + headlen;
				return _VKDATA_OK;
			}
			break;
		}
		case PT_AC3:
		case PT_PCMA:
		case PT_AAC:
		case PT_PCMU:
		case PT_G726_16:
		case PT_G726_24:
		case PT_G726_32:
		case PT_G726_40:
			//声道和采样率
			if (temnstartpos + headlen + AUDIOCHLLEN + AUDIORATELEN > buflen)
			{
				return _VKDATA_NOTENOUGH;//数据 不够
			}
			memcpy(&pheard->audio.chl, pheardstartbuf + headlen, AUDIOCHLLEN);
			headlen += AUDIOCHLLEN;

			memcpy(&pheard->audio.frate, pheardstartbuf + headlen, AUDIORATELEN);
			headlen += AUDIORATELEN;

			//*havereadsize = temnstartpos + headlen;
			return _VKDATA_OK;
			//break;
		default:
			return _VKDATA_OK;
		}
	}
	else
	{
		*skipsize = buflen;
	}
	return 0;// _VKDATA_LOSTDATA;
}
int JsonStream::IskeyFrame(DataHeard& heard)
{
	switch (heard.codect) //一般SPS和pps I帧 组成一个IDR帧
	{
	case PT_H264:
	{
		if (heard.framet == H264E_NALU_SPS || heard.framet == H264E_NALU_ISLICE)
		{
			return 1;
		}
	}
	break;
	case PT_H265:
	{
		if (heard.framet == H265_NALU_VPS || heard.framet == H265_NALU_SPS ||
			heard.framet == H265_NALU_ISLICE || heard.framet == H265_NALU_ISLICE_MAX)
		{
			return 1;
		}
	}
	break;
	case PT_VSK_POINT_CLOUD_XYZIT:
	case PT_VSK_POINT_CLOUD_DRACO:
		return 1;
	default:
		break;
	}
	return 0;
}
//根据帧类型判断 帧heard 长度,数据buf 里面的长度 不是 sizeof(DataHeard)
unsigned int JsonStream::FrameHeardLen(unsigned short codectype, unsigned short frametype)
{
	switch (codectype) //一般SPS和pps I帧 组成一个IDR帧
	{
	case PT_H264:
	{
		if (frametype == H264E_NALU_SPS) //sps 里面才有帧的 高和宽
		{
			//加数据宽 高
			return basicHeardLen() + VIDEOWIDTHLEN + VIDEOHEIGHTLEN + TIMESTAMPLEN;
		}
		else if (frametype == H264E_NALU_ISLICE)
		{
			//加数据宽 高 utc 时间
			return basicHeardLen() + VIDEOWIDTHLEN + VIDEOHEIGHTLEN + TIMESTAMPLEN;
		}
		else
		{
			return basicHeardLen();
		}
		break;
	}
	case PT_H265:
	{
		if (frametype == H265_NALU_VPS || frametype == H265_NALU_SPS)
		{
			return basicHeardLen() + VIDEOWIDTHLEN + VIDEOHEIGHTLEN + TIMESTAMPLEN;
		}
		else if (frametype == H265_NALU_ISLICE || frametype == H265_NALU_ISLICE_MAX)
		{
			//加数据宽 高 utc 时间
			return basicHeardLen() + VIDEOWIDTHLEN + VIDEOHEIGHTLEN + TIMESTAMPLEN;
		}
		else
		{
			return basicHeardLen();
		}
		break;
	}
	case PT_AC3:
	case PT_PCMA:
	case PT_AAC:
	case PT_PCMU:
	case PT_G726_16:
	case PT_G726_24:
	case PT_G726_32:
	case PT_G726_40:
		//声道和采样率
		return basicHeardLen() + AUDIOCHLLEN + AUDIORATELEN;
		//break;
	case PT_VSK_POINT_CLOUD_XYZIT:
	case PT_VSK_POINT_CLOUD_DRACO:
		return -1;// gnPointCloudFrameHeaderSize;
	default:
		return basicHeardLen();
	}
	// 修改返回了-1的bug
	return 0;
}
unsigned int  JsonStream::basicHeardLen()
{
	//heard start code 4 Byte
		//codect 2	Byte
		//framet 2	Byte
		//gapms  2	Byte
		//framerate 2 Byte
		//framelen	4 Byte

		//videowidth 4Byte			framet SPS have I slice have 
		//videoheight  4Byte	   framet SPS have I slice have 
		//timestamp    I slice have  

	return VKDATA2HEARDLEN + CODECTYPELEN + FRAMETYPELEN + GAPMSLEN + FRAMERATELEN + FRAMELENLEN;
}
int JsonStream::MatchDataHeard(unsigned char* pbuf, unsigned int dwSize)
{
	int temp = KMPMatch(pbuf, dwSize, VISKINGFILEHEARD_3, VKDATA2HEARDLEN);
	if (temp >= 0) return temp;
	temp = KMPMatch(pbuf, dwSize, VISKINGFILEHEARD_2, VKDATA2HEARDLEN);
	return temp;
}
int JsonStream::ReadBasicHeardFormBuf(unsigned char* pbuf, unsigned int pbufsize, DataHeard* pheard)
{
	if (pbufsize < VKDATA2HEARDLEN)
	{
		return _VKDATA_NOTENOUGH;
	}
	unsigned int  readsize = VKDATA2HEARDLEN;
	if (readsize + CODECTYPELEN > pbufsize)
	{
		return _VKDATA_NOTENOUGH;
	}
	memcpy(&pheard->codect, pbuf + readsize, CODECTYPELEN);
	readsize += CODECTYPELEN;

	if (readsize + FRAMETYPELEN > pbufsize)
	{
		return _VKDATA_NOTENOUGH;
	}
	memcpy(&pheard->framet, pbuf + readsize, FRAMETYPELEN);
	readsize += FRAMETYPELEN;

	if (readsize + GAPMSLEN > pbufsize)
	{
		return _VKDATA_NOTENOUGH;
	}
	memcpy(&pheard->gapms, pbuf + readsize, GAPMSLEN);
	readsize += GAPMSLEN;

	if (readsize + FRAMERATELEN > pbufsize)
	{
		return _VKDATA_NOTENOUGH;
	}
	memcpy(&pheard->framerate, pbuf + readsize, FRAMERATELEN);
	readsize += FRAMERATELEN;

	if (readsize + FRAMELENLEN > pbufsize)
	{
		return _VKDATA_NOTENOUGH;
	}
	memcpy(&pheard->framelen, pbuf + readsize, FRAMELENLEN);
	readsize += FRAMELENLEN;
	return _VKDATA_OK;

}
STREAM_CODEC JsonStream::switchFromToGB(int type)
{
	switch(type)
	{
	case PT_H264:
		return GB_CODEC_H264;
	case PT_H265:
		return GB_CODEC_H265;
	case PT_G711A:
	case PT_G711U:
	case PT_PCMA:
	case PT_PCMU:
	case PT_AAC:
	default:
		return GB_CODEC_UNKNOWN;
	}
}