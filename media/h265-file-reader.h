#ifndef _h265_file_reader_h_
#define _h265_file_reader_h_

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <queue>
#include <stdint.h>

#define H265_FRAME_QUEUE_SIZE 300

class H265FileReader
{
public:
	H265FileReader();
	H265FileReader(const char* file);
	~H265FileReader();

	bool IsOpened() const;
	void Input(const uint8_t* data, size_t size);

	struct vframe_t
	{
		const uint8_t* nalu;
		int64_t time;
		long bytes;
		bool idr;

		bool operator < (const struct vframe_t &v) const
		{
			return time < v.time;
		}
	};

public:
    const std::list<std::pair<const uint8_t*, size_t> > GetParameterSets() const { return m_sps; }
	int GetDuration(int64_t& duration) const { duration = m_duration; return 0; }
    int GetNextFrame(int64_t &dts, const uint8_t* &ptr, size_t &bytes);
	int GetNextFrameEx(int64_t& dts, const uint8_t*& ptr, size_t& bytes);
	int Seek(int64_t &dts);

	struct vframe_t* front();
	void pop();
	bool push(vframe_t);

private:
	int Init();
	int Init2();

private:
    typedef std::vector<vframe_t> vframes_t;
    vframes_t m_videos;
	vframes_t::iterator m_vit;
	size_t m_readIdx, m_writeIdx;
	std::queue<vframe_t> cacheQueue;

	bool spspps2;
    std::list<std::pair<const uint8_t*, size_t> > m_sps;
	int64_t m_duration;

    uint8_t *m_ptr, *bm_ptr;
    size_t m_capacity;
	size_t rPos, wPos, frPos, fwPos;

	const uint8_t* oneFrame;
	size_t oneFrameSize;

	bool dump();
};

#endif /* !_h265_file_reader_h_ */
