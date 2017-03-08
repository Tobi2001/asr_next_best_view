/**

Copyright (c) 2016, Aumann Florian, Borella Jocelyn, Heller Florian, Meißner Pascal, Schleicher Ralf, Stöckle Patrick, Stroh Daniel, Trautmann Jeremias, Walter Milena, Wittenbeck Valerij
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. The use is explicitly not permitted to any application which deliberately try to kill or do harm to any living creature.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "next_best_view/filter/sample_point/HypothesisClusterSpaceSampleFilter.hpp"

namespace next_best_view {

    HypothesisClusterSpaceSampleFilter::HypothesisClusterSpaceSampleFilter(const ClusterExtractionPtr &ce, float offset) : mClusterExtraction(ce), mOffset(offset) { }

    HypothesisClusterSpaceSampleFilter::~HypothesisClusterSpaceSampleFilter() { }

    void HypothesisClusterSpaceSampleFilter::doFiltering(IndicesPtr &resultIndicesPtr) {
        // if indices is not set we will use all points
        SamplePointCloudPtr samplesPtr = getInputPointCloud();
        IndicesPtr samplesIndicesPtr = getInputPointIndices();

        // get clusters and expand them by an offset (fcp)
        BoundingBoxPtrsPtr clustersPtr = mClusterExtraction->getClusters();
        BoundingBoxPtrsPtr expandedClustersPtr(new BoundingBoxPtrs());
        for (BoundingBoxPtr &clusterPtr : *clustersPtr) {
            BoundingBoxPtr bbPtr = BoundingBoxPtr(new BoundingBox(clusterPtr->minPos, clusterPtr->maxPos));
            bbPtr->expand(SimpleVector3(mOffset, mOffset, 0));
            bbPtr->ignoreAxis(2);
            expandedClustersPtr->push_back(bbPtr);
        }

        // go through all samples
        for (int idx : *samplesIndicesPtr) {
            // if sample is in a cluster, add it to result/ret vec
            for (BoundingBoxPtr &cluster : *expandedClustersPtr) {
                if (cluster->contains(samplesPtr->at(idx).getSimpleVector3())) {
                    resultIndicesPtr->push_back(idx);
                    break;
                }
            }
        }
    }
}