


class MediaIn
{
public:
    int OnLiveStream(uint8_t* data, size_t size) = 0;
    int OnVodStream(uint8_t* data, size_t size) = 0;
};