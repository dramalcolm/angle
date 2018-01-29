//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanPipelineCachePerf:
//   Performance benchmark for the Vulkan Pipeline cache.

#include "ANGLEPerfTest.h"

#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "random_utils.h"

using namespace rx;

namespace
{
class VulkanPipelineCachePerfTest : public ANGLEPerfTest
{
  public:
    VulkanPipelineCachePerfTest();
    ~VulkanPipelineCachePerfTest();

    void SetUp() override;
    void step() override;

    PipelineCache mCache;
    angle::RNG mRNG;

    std::vector<vk::PipelineDesc> mCacheHits;
    std::vector<vk::PipelineDesc> mCacheMisses;
    size_t mMissIndex = 0;

  private:
    void randomizeDesc(vk::PipelineDesc *desc);
};

VulkanPipelineCachePerfTest::VulkanPipelineCachePerfTest()
    : ANGLEPerfTest("VulkanPipelineCachePerf", "")
{
}

VulkanPipelineCachePerfTest::~VulkanPipelineCachePerfTest()
{
    mCache.destroy(VK_NULL_HANDLE);
}

void VulkanPipelineCachePerfTest::SetUp()
{
    // Insert a number of random pipeline states.
    for (int pipelineCount = 0; pipelineCount < 100; ++pipelineCount)
    {
        vk::Pipeline pipeline;
        vk::PipelineDesc desc;
        randomizeDesc(&desc);

        if (pipelineCount < 10)
        {
            mCacheHits.push_back(desc);
        }
        mCache.populate(desc, std::move(pipeline));
    }

    for (int missCount = 0; missCount < 10000; ++missCount)
    {
        vk::PipelineDesc desc;
        randomizeDesc(&desc);
        mCacheMisses.push_back(desc);
    }
}

void VulkanPipelineCachePerfTest::randomizeDesc(vk::PipelineDesc *desc)
{
    std::vector<uint8_t> bytes(sizeof(vk::PipelineDesc));
    FillVectorWithRandomUBytes(&mRNG, &bytes);
    memcpy(desc, bytes.data(), sizeof(vk::PipelineDesc));
}

void VulkanPipelineCachePerfTest::step()
{
    vk::RenderPass rp;
    vk::PipelineLayout pl;
    vk::ShaderModule sm;
    vk::PipelineAndSerial *result = nullptr;

    for (int iteration = 0; iteration < 100; ++iteration)
    {
        for (const auto &hit : mCacheHits)
        {
            (void)mCache.getPipeline(VK_NULL_HANDLE, rp, pl, sm, sm, hit, &result);
        }
    }

    for (int missCount = 0; missCount < 20 && mMissIndex < mCacheMisses.size();
         ++missCount, ++mMissIndex)
    {
        const auto &miss = mCacheMisses[mMissIndex];
        (void)mCache.getPipeline(VK_NULL_HANDLE, rp, pl, sm, sm, miss, &result);
    }
}

}  // anonymous namespace

TEST_F(VulkanPipelineCachePerfTest, Run)
{
    run();
}