#include "h265-file-reader.h"
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <malloc.h>
#include "videoNalu.hpp"


H265FileReader::H265FileReader()
	:m_ptr(NULL), bm_ptr(NULL), m_capacity(0), m_readIdx(0), m_writeIdx(0), rPos(0), wPos(0), frPos(0), fwPos(0), oneFrame(NULL), oneFrameSize(0), spspps2(true), m_duration(0)
{
	m_videos.resize(H265_FRAME_QUEUE_SIZE);
	m_vit = m_videos.begin();
}

H265FileReader::H265FileReader(const char* file)
:m_ptr(NULL), bm_ptr(NULL), m_capacity(0), m_readIdx(0), m_writeIdx(0), rPos(0), wPos(0), frPos(0), fwPos(0), oneFrame(NULL), oneFrameSize(0), spspps2(true), m_duration(0)
{
	m_videos.resize(H265_FRAME_QUEUE_SIZE);

	FILE* fp = fopen(file, "rb");
    if(fp)
    {
		fseek(fp, 0, SEEK_END);
		m_capacity = ftell(fp);
		fseek(fp, 0, SEEK_SET);

        m_ptr = (uint8_t*)malloc(m_capacity);
		fread(m_ptr, 1, m_capacity, fp);
		fclose(fp);

		Init();
    }

	m_vit = m_videos.begin();
}

H265FileReader::~H265FileReader()
{    
	if (m_ptr)
	{
		assert(m_capacity > 0);
		free(m_ptr);
	}
	if (bm_ptr)
	{
		free(bm_ptr);
	}
}

bool H265FileReader::IsOpened() const
{
	return !m_videos.empty();
}

int H265FileReader::GetNextFrame(int64_t &dts, const uint8_t* &ptr, size_t &bytes)
{
	if(m_vit == m_videos.end())
		return -1; // file end

	ptr = m_vit->nalu;
	dts = m_vit->time;
	bytes = m_vit->bytes;

	++m_vit;
	return 0;
}

int H265FileReader::GetNextFrameEx(int64_t& dts, const uint8_t*& ptr, size_t& bytes)
{
	vframe_t* frame = front();
	if (!frame)
		return -1;

	ptr = frame->nalu;
	dts = frame->time;
	bytes = frame->bytes;
	pop();

	return 0;
}

H265FileReader::vframe_t* H265FileReader::front()
{
	if ((m_writeIdx - m_readIdx + H265_FRAME_QUEUE_SIZE) % H265_FRAME_QUEUE_SIZE >= 1)
		return &m_videos[m_readIdx];
	return NULL;
}

void H265FileReader::pop()
{
	if ((m_writeIdx - m_readIdx + H265_FRAME_QUEUE_SIZE) % H265_FRAME_QUEUE_SIZE >= 1)
		m_readIdx = (++m_readIdx) % H265_FRAME_QUEUE_SIZE;
	else
		return;
}

bool H265FileReader::push(vframe_t frame)
{
	if (!dump() && !((m_writeIdx + 1) % H265_FRAME_QUEUE_SIZE == m_readIdx))
	{
		m_videos[m_writeIdx] = frame;
		m_writeIdx = (++m_writeIdx) % H265_FRAME_QUEUE_SIZE;
		return true;
	}
	else
	{
		cacheQueue.push(frame);
	}
	return true;
}

bool H265FileReader::dump()
{
	while (!cacheQueue.empty())
	{
		if ((m_writeIdx + 1) % H265_FRAME_QUEUE_SIZE == m_readIdx)
			return true;
		m_videos[m_writeIdx] = cacheQueue.front();
		m_writeIdx = ++m_writeIdx % H265_FRAME_QUEUE_SIZE;
		cacheQueue.pop();
	}
	return false;
}

int H265FileReader::Seek(int64_t &dts)
{
	vframe_t frame;
	frame.time = dts;

	vframes_t::iterator it;
	it = std::lower_bound(m_videos.begin(), m_videos.end(), frame);
	if(it == m_videos.end())
		return -1;

	while(it != m_videos.begin())
	{
		if(it->idr)
		{
			m_vit = it;
			return 0;
		}
		--it;
	}
	return 0;
}

int H265FileReader::Init()
{
    size_t count = 0;
    bool vpsspspps = true;

	const uint8_t* end = m_ptr + m_capacity;
    const uint8_t* nalu = search_start_code(m_ptr, end);
	const uint8_t* p = nalu;

	while (p < end)
	{
        const unsigned char* pn = search_start_code(p + 4, end);
		size_t bytes = pn - nalu;

        int nal_unit_type = h265_nal_type(p);
		assert(0 <= nal_unit_type);

        if(NAL_VPS == nal_unit_type || NAL_SPS == nal_unit_type || NAL_PPS_5 == nal_unit_type)
        {
            if(vpsspspps)
            {
                size_t n = 0x01 == p[2] ? 3 : 4;
				std::pair<const uint8_t*, size_t> pr;
				pr.first = p + n;
				pr.second = bytes;
				m_sps.push_back(pr);
            }
        }
		
        {
            if(m_sps.size() > 0) vpsspspps = false;

			vframe_t frame;
			frame.nalu = nalu;
			frame.bytes = bytes;
			frame.time = 40 * count++;
			frame.idr = (NAL_IDR_N_LP == nal_unit_type || NAL_IDR_W_RADL == nal_unit_type);
			push(frame);
			nalu = pn;
        }

        p = pn;
    }

    m_duration = 40 * count;
    return 0;
}

int H265FileReader::Init2()
{
	size_t count = 0;

	const uint8_t* end = oneFrame + oneFrameSize;
	const uint8_t* nalu = search_start_code(oneFrame, end);
	const uint8_t* p = nalu;

	while (p < end)
	{
		const unsigned char* pn = search_start_code(p + 4, end);
		size_t bytes = pn - nalu;

		int nal_unit_type = h265_nal_type(p);
		assert(0 <= nal_unit_type);

		if (nal_unit_type <= 31 && h264_nal_new_access(pn, end))
		{
			if (m_sps.size() > 0) spspps2 = false;

			vframe_t frame;
			frame.nalu = nalu;
			frame.bytes = (long)bytes;
			frame.time = 40 * count++;
			frame.idr = (NAL_IDR_N_LP == nal_unit_type || NAL_IDR_W_RADL == nal_unit_type);
			push(frame);
			if (fwPos)
			{
				frPos += bytes;
				if (frPos == fwPos)
				{
					fwPos = frPos = 0;
				}
			}
			else
			{
				rPos += bytes;
			}
			nalu = pn;
		}
		else if (NAL_VPS == nal_unit_type || NAL_SPS == nal_unit_type || NAL_PPS_5 == nal_unit_type)
		{
			if (spspps2)
			{
				size_t n = 0x01 == p[2] ? 3 : 4;
				std::pair<const uint8_t*, size_t> pr;
				pr.first = p + n;
				pr.second = bytes;
				m_sps.push_back(pr);
			}
		}

		p = pn;
	}
	m_duration = 40 * count;
	return 0;
}

void H265FileReader::Input(const uint8_t* data, size_t size)
{
	if (!m_capacity)
	{
		m_capacity = 1024 * 1024;
		m_ptr = (uint8_t*)malloc(m_capacity);
		bm_ptr = (uint8_t*)malloc(m_capacity);
	}

	if (m_capacity - wPos >= size)
	{
		int nal_unit_type = h265_nal_type(data);
		if (NAL_VPS == nal_unit_type || NAL_SPS == nal_unit_type || NAL_PPS_5 == nal_unit_type)
		{
			memcpy(m_ptr + wPos, data, size);
			wPos += size;
			return;
		}
		else
		{
			memcpy(m_ptr + wPos, data, size);
			wPos += size;
		}
		if (fwPos)
		{
			oneFrame = bm_ptr + frPos;
			oneFrameSize = fwPos - frPos;
		}
		else
		{
			oneFrame = m_ptr + rPos;
			oneFrameSize = wPos - rPos;
		}
		Init2();
	}
	else
	{
		frPos = rPos;
		fwPos = wPos;
		wPos = rPos = 0;
		uint8_t* tmpe = bm_ptr;
		bm_ptr = m_ptr;
		if (tmpe)
		{
			tmpe = (uint8_t*)malloc(m_capacity);
			m_ptr = tmpe;
		}
		Input(data, size);
	}
}
