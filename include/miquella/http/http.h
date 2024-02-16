#pragma once

#include <miquella/core/sceneFactory.h>

//#include <cpr/cpr.h>

//#include <nlohmann/json.hpp>
//using json = nlohmann::json;

namespace miquella 
{

namespace http 
{

std::tuple<long, std::string> uploadJobToRemoteController(
                                const std::string& serverURL,
                                int port,
                                const std::string& filePath, 
                                const std::string& jobID,
                                size_t lastSample);

std::tuple<long, std::string> uploadJobToLocalController(
                                const std::string& filePath, 
                                const std::string& jobID,
                                size_t lastSample);

std::tuple<long, std::string> requestJob(
                                const std::string& serverURL,
                                int port);

std::tuple<long, std::string> submitJob(
                                const std::string& serverURL,
                                int port,
                                miquella::core::SceneID id,
                                int nSamples,
                                int freqOutput);

std::tuple<long, std::string> requestLastLocalSample(
                                const std::string& serverURL,
                                int port,
                                const std::string& jobID);

std::tuple<long, std::string, std::map<std::string, std::string>> requestLastRemoteSample(
                                const std::string& serverURL,
                                int port,
                                const std::string& jobID);

std::tuple<long, std::string> requestListJobs(
                                const std::string& serverURL,
                                int port);

std::tuple<long, std::string> requestCancelJob(
                                const std::string& serverURL,
                                int port,
                                const std::string& jobID);

std::tuple<long, std::string> requestRemoveJob(
                                const std::string& serverURL,
                                int port,
                                const std::string& jobID);
} // http

} // miquella 