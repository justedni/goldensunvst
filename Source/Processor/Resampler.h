#pragma once

#include <vector>

#include "Types.h"

namespace GSVST {

/* 
 * res_data_fetch_cb fetches samplesRequired samples to fetchBuffer
 * so that the buffer can provide exactly samplesRequired samples
 *
 * returns false in case of 'end of stream'
 */
typedef bool (*res_data_fetch_cb)(std::vector<sample>& fetchBuffer, size_t samplesRequired, void *cbdata);
typedef std::vector<sample> StereoBuffer;


class Resampler {
public:
    // return value false by Process signals the "end of stream"
    virtual bool Process(sample* outData, size_t numBlocks, float phaseInc, res_data_fetch_cb cbPtr, void *cbdata) = 0;
    virtual void Reset() = 0;
    virtual ~Resampler();

protected:
    StereoBuffer fetchBuffer;
    float phase;
};

class LinearResampler : public Resampler {
public:
    LinearResampler();
    ~LinearResampler() override;
    bool Process(sample* outData, size_t numBlocks, float phaseInc, res_data_fetch_cb cbPtr, void *cbdata) override;
    void Reset() override;
};

class BlepResampler : public Resampler {
public:
    BlepResampler();
    ~BlepResampler() override;
    bool Process(sample* outData, size_t numBlocks, float phaseInc, res_data_fetch_cb cbPtr, void* cbdata) override;
    void Reset() override;
private:
    static float fast_Si(float t);
};


}
