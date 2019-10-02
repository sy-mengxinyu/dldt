// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "ie_built_in_impl.hpp"
#include <map>
#include <memory>
#include <string>
#include <algorithm>
#include <vector>
#include <shape_infer/const_infer/ie_strided_slice_const_infer.hpp>

namespace InferenceEngine {
namespace ShapeInfer {

/**
 *@brief Implementation of Shape inference for StridedSlice layer
 */
class StridedSliceShapeProp : public BuiltInShapeInferImpl {
public:
    explicit StridedSliceShapeProp(const std::string& type) : BuiltInShapeInferImpl(type) {}

    void inferShapesImpl(const std::vector<Blob::CPtr>& inBlobs,
                         const std::map<std::string, std::string>& params,
                         const std::map<std::string, Blob::Ptr>& blobs,
                         std::vector<SizeVector>& outShapes) override {
        StridedSliceHelper helper(inBlobs, params);
        outShapes.push_back(helper.getOutputShape());
    }
};

}  // namespace ShapeInfer
}  // namespace InferenceEngine

