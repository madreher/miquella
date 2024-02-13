#pragma once

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;


#define CONTROLLER_UPDATE_REMOTE_JOB "/updateRemoteJobExec"
#define CONTROLLER_UPDATE_LOCAL_JOB "/updateLocalJobExec"
#define CONTROLLER_REQUEST_JOB "/requestJob"

namespace miquella 
{

namespace http 
{

std::tuple<long, std::string> uploadJobToRemoteController(
                                const std::string& serverURL,
                                int port,
                                const std::string& filePath, 
                                const std::string& jobID,
                                size_t lastSample)
{
    std::string url = serverURL + ":" + std::to_string(port) + CONTROLLER_UPDATE_REMOTE_JOB;

    // IMPORTANT: the part name "file" must match the parameter name in the 
    // controller function!
    cpr::Response r = cpr::Post(cpr::Url{url},
                cpr::Multipart{
                    {"file", cpr::File{filePath}},
                    {"jobID", jobID},
                    {"lastSample", std::to_string(lastSample)}
                    });

    return {r.status_code, r.text};
}

std::tuple<long, std::string> uploadJobToLocalController(
                                const std::string& filePath, 
                                const std::string& jobID,
                                size_t lastSample)
{
    std::string url = std::string("http://localhost:8000") + CONTROLLER_UPDATE_LOCAL_JOB;

    cpr::Response r = cpr::Post(cpr::Url{url},
    cpr::Parameters{
        {"jobID", jobID},
        {"filePath", filePath},
        {"lastSample", std::to_string(lastSample)}
        });  
    
    return {r.status_code, r.text};
}

std::tuple<long, std::string> requestJob(
                                const std::string& serverURL,
                                int port)
{
    std::string url = serverURL + ":" + std::to_string(port) + CONTROLLER_REQUEST_JOB;

    cpr::Response r = cpr::Post(cpr::Url{url});

    return {r.status_code, r.text};
}                                

} // http

} // miquella 