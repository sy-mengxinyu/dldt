// Copyright (C) 2018-2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

/**
 * @brief A header file that provides interface for network reader that is used to build networks from a given IR
 * @file ie_icnn_net_reader.h
 */
#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>

#include <ie_blob.h>
#include "cnn_network_impl.hpp"
#include <xml_parse_utils.h>
#include <details/caseless.hpp>

#include <ngraph/type/element_type.hpp>
#include <ngraph/op/parameter.hpp>
#include <ngraph/function.hpp>
#include <ngraph/node.hpp>

namespace InferenceEngine {

class IParser {
public:
    using Ptr = std::shared_ptr<IParser>;
    virtual ~IParser() = default;
    virtual std::shared_ptr<ngraph::Function> parse(const pugi::xml_node &root, const Blob::CPtr& weights) = 0;
};

class IRParser {
public:
    explicit IRParser(size_t version);
    std::shared_ptr<ngraph::Function> parse(const pugi::xml_node &root, const Blob::CPtr& weights);
    virtual ~IRParser() = default;

private:
    IParser::Ptr parser;
};

class V10Parser : public IParser {
public:
    std::shared_ptr<ngraph::Function> parse(const pugi::xml_node& root, const Blob::CPtr& weights) override;

private:
    struct GenericLayerParams {
        struct LayerPortData {
            size_t        portId;
            Precision     precision;
            SizeVector    dims;
        };
        size_t layerId;
        std::string name;
        std::string type;
        Precision precision;
        std::vector<LayerPortData> inputPorts;
        std::vector<LayerPortData> outputPorts;

        size_t getRealInputPortId(size_t id) {
            size_t real_id = 0;
            for (auto & it : inputPorts) {
                if (it.portId == id) {
                    return real_id;
                }
                ++real_id;
            }
            THROW_IE_EXCEPTION << "Can not find input port with id " << id << " in layer " << name;
        }

        size_t getRealOutputPortId(size_t id) {
            size_t real_id = 0;
            for (auto & it : outputPorts) {
                if (it.portId == id) {
                    return real_id;
                }
                ++real_id;
            }
            THROW_IE_EXCEPTION << "Can not find output port with id " << id << " in layer " << name;
        }
    };

    class LayerBaseCreator {
    private:
        std::string type;

    protected:
        explicit LayerBaseCreator(const std::string& type) : type(type) {}
        std::string getType() { return type; }
        template<class T>
        std::vector<T> getParameters(const pugi::xml_node& node, const std::string& name) {
            std::vector<T> result;
            std::string param = XMLParseUtils::GetStrAttr(node, name.c_str());
            std::stringstream ss(param);
            std::string field;
            while (getline(ss, field, ',' )) {
                std::stringstream fs(field);
                T value;
                fs >> value;
                result.emplace_back(value);
            }
            return result;
        }

        template<class T>
        std::vector<T> getParameters(const pugi::xml_node& node, const std::string& name, const std::vector<T>& def) {
            std::vector<T> result;
            std::string param = XMLParseUtils::GetStrAttr(node, name.c_str(), "");
            if (param.empty())
                return def;
            std::stringstream ss(param);
            std::string field;
            while (getline(ss, field, ',' )) {
                std::stringstream fs(field);
                T value;
                fs >> value;
                result.emplace_back(value);
            }
            return result;
        }

        void checkParameters(const GenericLayerParams& params, int numInputs, int numOutputs) {
            if (numInputs >= 0 && params.inputPorts.size() != numInputs) {
                THROW_IE_EXCEPTION << params.type << " layer " << params.name << " with id: " << params.layerId
                    << " has incorrect number of input ports!";
            }
            for (size_t i = 0; i < params.inputPorts.size(); i++) {
                for (const auto& dim : params.inputPorts[i].dims) {
                    if (!dim)
                        THROW_IE_EXCEPTION << params.type << " layer " << params.name << " with id: " << params.layerId
                            << " has incorrect dimensions in the input port" << i << "!";
                }
            }
            if (numOutputs >= 0 && params.outputPorts.size() != numOutputs) {
                THROW_IE_EXCEPTION << params.type << " layer " << params.name << " with id: " << params.layerId
                    << " has incorrect number of output ports!";
            }
            for (size_t i = 0; i < params.outputPorts.size(); i++) {
                for (const auto& dim : params.outputPorts[i].dims) {
                    if (!dim)
                        THROW_IE_EXCEPTION << params.type << " layer " << params.name << " with id: " << params.layerId
                            << " has incorrect dimensions in the output port" << i << "!";
                }
            }
        }

    public:
        virtual ~LayerBaseCreator() {}
        virtual std::shared_ptr<ngraph::Node> createLayer(const ngraph::OutputVector & inputs,
                                                          const pugi::xml_node& node, const Blob::CPtr& weights,
                                                          const GenericLayerParams& layerParsePrms) = 0;

        bool shouldCreate(const std::string& nodeType) const;

        std::shared_ptr<ngraph::Node> createOptionalParameter(const GenericLayerParams::LayerPortData& port);
    };

    template<class T>
    class LayerCreator : public LayerBaseCreator{
    public:
        explicit LayerCreator(const std::string& type) : LayerBaseCreator(type) {}
        std::shared_ptr<ngraph::Node> createLayer(const ngraph::OutputVector & inputs,
                                                  const pugi::xml_node& node, const Blob::CPtr& weights,
                                                  const GenericLayerParams& layerParsePrms) override;
    };

    std::shared_ptr<ngraph::Node> createNode(const ngraph::OutputVector & inputs,
                                             const pugi::xml_node& node, const Blob::CPtr& weights,
                                             const GenericLayerParams& params);

    GenericLayerParams parseGenericParams(const pugi::xml_node& node);

    std::map<std::string, DataPtr> portsToData;
    std::map<std::string, GenericLayerParams> layersParseInfo;
};

}  // namespace InferenceEngine

