#include "h264-file-reader.h"
#include <assert.h>
#include <string.h>
#include <algorithm>

#define H264_NAL(v)	(v & 0x1F)

enum { NAL_IDR = 5, NAL_SEI = 6, NAL_SPS = 7, NAL_PPS = 8 };
const unsigned int MAX_SIZE = 128;

H264FileReader::H264FileReader(const char* file)
:m_ptr(NULL), bm_ptr(NULL), m_capacity(0), read(0), write(0), oneFrame(NULL), oneFrameSize(0)
{
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
	if (!m_ptr)
	{
		m_capacity = 10 * 1024 * 1024;
		m_ptr = (uint8_t*)malloc(m_capacity);
		m_videos.resize(128);
		spspps2 = true;
	}
	rPos = wPos = frPos = fwPos = 0;

	m_vit = m_videos.begin();
}

H264FileReader::~H264FileReader()
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

bool H264FileReader::IsOpened() const
{
	return !m_videos.empty();
}

int H264FileReader::GetNextFrame(int64_t &dts, const uint8_t* &ptr, size_t &bytes)
{
	if(m_vit == m_videos.end())
		return -1; // file end

	ptr = m_vit->nalu;
	dts = m_vit->time;
	bytes = m_vit->bytes;

	++m_vit;
	m_videos.pop_back();
	return 0;
}
int H264FileReader::GetNextFrameEx(int64_t& dts, const uint8_t*& ptr, size_t& bytes)
{
	vframe_t* frame = front();
	if (frame)
	{
		ptr = frame->nalu;
		dts = frame->time;
		bytes = frame->bytes;
		pop();
	}
	return 0;
}
int H264FileReader::Seek(int64_t &dts)
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

static inline const uint8_t* search_start_code(const uint8_t* ptr, const uint8_t* end)
{
    for(const uint8_t *p = ptr; p + 3 < end; p++)
    {
        if(0x00 == p[0] && 0x00 == p[1] && (0x01 == p[2] || (0x00==p[2] && 0x01==p[3])))
            return p;
    }
	return end;
}

static inline int h264_nal_type(const unsigned char* ptr)
{
    int i = 2;
    assert(0x00 == ptr[0] && 0x00 == ptr[1]);
    if(0x00 == ptr[2])
        ++i;
    assert(0x01 == ptr[i]);
    return H264_NAL(ptr[i+1]);
}

static inline int h264_nal_new_access(const unsigned char* ptr, const uint8_t* end)
{
	int i = 2;
	if (end - ptr < 4)
		return 1;
	assert(0x00 == ptr[0] && 0x00 == ptr[1]);
	if (0x00 == ptr[2])
		++i;
	assert(0x01 == ptr[i]);
	int nal_unit_type = H264_NAL(ptr[i + 1]);
	if (nal_unit_type < 1 || nal_unit_type > 5)
		return 1;

	if (ptr + i + 2 > end)
		return 1;

	// Live555 H264or5VideoStreamParser::parse
	// The high-order bit of the byte after the "nal_unit_header" tells us whether it's
	// the start of a new 'access unit' (and thus the current NAL unit ends an 'access unit'):
	return (ptr[i + 2] & 0x80) != 0 ? 1 : 0;
}

int H264FileReader::Init()
{
    size_t count = 0;
    bool spspps = true;

	const uint8_t* end = m_ptr + m_capacity;
    const uint8_t* nalu = search_start_code(m_ptr, end);
	const uint8_t* p = nalu;

	while (p < end)
	{
        const unsigned char* pn = search_start_code(p + 4, end);
		size_t bytes = pn - nalu;

        int nal_unit_type = h264_nal_type(p);
		assert(0 != nal_unit_type);
        if(nal_unit_type <= 5 && h264_nal_new_access(pn, end))
        {
            if(m_sps.size() > 0) spspps = false; // don't need more sps/pps

			vframe_t frame;
			frame.nalu = (uint8_t*)nalu;
			frame.bytes = (long)bytes;
			frame.time = 40 * count++;
			frame.idr = 5 == nal_unit_type; // IDR-frame
			m_videos.push_back(frame);
			nalu = pn;
        }
        else if(NAL_SPS == nal_unit_type || NAL_PPS == nal_unit_type)
        {
            if(spspps)
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
int H264FileReader::Init2()
{
	size_t count = 0;

	const uint8_t* end = oneFrame + oneFrameSize;
	const uint8_t* nalu = search_start_code(oneFrame, end);
	const uint8_t* p = nalu;

	while (p < end)
	{
		const unsigned char* pn = search_start_code(p + 4, end);
		size_t bytes = pn - nalu;

		int nal_unit_type = h264_nal_type(p);
		assert(0 != nal_unit_type);
		if (nal_unit_type <= 5 && h264_nal_new_access(pn, end))
		{
			if (m_sps.size() > 0) spspps2 = false; // don't need more sps/pps

			vframe_t frame;
			frame.nalu = nalu;
			frame.bytes = (long)bytes;
			frame.time = 40 * count++;
			frame.idr = 5 == nal_unit_type; // IDR-frame
			//m_videos.push_back(frame);
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
		else if (NAL_SPS == nal_unit_type || NAL_PPS == nal_unit_type)
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

H264FileReader::vframe_t* H264FileReader::front()
{
	//printf("H264FileReader::front read:%d\n", read);
	if ((write - read + MAX_SIZE) % MAX_SIZE >= 1)
		return &m_videos[read];
	return NULL;
}

void H264FileReader::pop()
{
	if ((write - read + MAX_SIZE) % MAX_SIZE >= 1)
		read = (++read) % MAX_SIZE;
	else
		return;
}

bool H264FileReader::push(vframe_t frame)
{
	if (!dump() && !((write + 1) % MAX_SIZE == read))
	{
		//printf("H264FileReader::push write:%d\n", write);
		m_videos[write] = frame;
		write = (++write) % MAX_SIZE;
		return true;
	}
	else
	{
		//如果队列满了，则写入缓冲
		cacheQueue.push(frame);
	}
	return true;
}
bool H264FileReader::dump()
{
	//缓冲中还有数据
	while (!cacheQueue.empty())
	{
		if ((write + 1) % MAX_SIZE == read)
			return true;
		// 优先将缓冲中的数据写入到队列
		m_videos[write] = cacheQueue.front();
		write = ++write % MAX_SIZE;
		cacheQueue.pop();
	}
	return false;
}
void H264FileReader::Input(const uint8_t* data, size_t size)
{
	/*oneFrame = data;
	oneFrameSize = size;
	Init2();*/
	//return;
	if (m_capacity - wPos >= size)
	{
		int nal_unit_type = h264_nal_type(data);
		if (NAL_SPS == nal_unit_type || NAL_PPS == nal_unit_type)
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