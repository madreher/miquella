#include <miquella/http/http.h>

#include <cpr/cpr.h>

// Actual type is std::string_view, not std::string. std::string was 
// supposed to be working for c++20 but doesn't seem to be supported 
// universally
constexpr auto CONTROLLER_UPDATE_REMOTE_JOB          = "/updateRemoteJobExec";
constexpr auto CONTROLLER_UPDATE_LOCAL_JOB           = "/updateLocalJobExec";
constexpr auto CONTROLLER_REQUEST_JOB                = "/requestJob";
constexpr auto CONTROLLER_SUBMIT_JOB                 = "/submitJob";
constexpr auto CONTROLLER_REQUEST_LAST_LOCAL_SAMPLE  = "/requestLastLocalSample";
constexpr auto CONTROLLER_REQUEST_LAST_REMOTE_SAMPLE = "/requestLastRemoteSample";
constexpr auto CONTROLLER_REQUEST_LIST_JOB           = "/requestListAllJobs";
constexpr auto CONTROLLER_CANCEL_JOB                 = "/cancelJob";
constexpr auto CONTROLLER_REMOVE_JOB                 = "/removeJob";

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

std::tuple<long, std::string> submitJob(
                                const std::string& serverURL,
                                int port,
                                miquella::core::SceneID id,
                                int nSamples,
                                int freqOutput)
{
    std::string url = serverURL + ":" + std::to_string(port) + CONTROLLER_SUBMIT_JOB;
    cpr::Response r = cpr::Post(cpr::Url{url},
        cpr::Parameters{
            {"sceneID", std::to_string(static_cast<uint8_t>(id))},
            {"nSamples", std::to_string(nSamples)},
            {"freqOutput", std::to_string(freqOutput)}
    });

    return {r.status_code, r.text};
}

std::tuple<long, std::string> requestLastLocalSample(
                                const std::string& serverURL,
                                int port,
                                const std::string& jobID)
{
    // Create an HTTP request.
    std::string url = serverURL + ":" + std::to_string(port) + CONTROLLER_REQUEST_LAST_LOCAL_SAMPLE;
    cpr::Response r = cpr::Get(cpr::Url{url},
        cpr::Parameters{{"jobID", jobID}});

    return {r.status_code, r.text};
}

std::tuple<long, std::string, std::map<std::string, std::string>> requestLastRemoteSample(
                                const std::string& serverURL,
                                int port,
                                const std::string& jobID)
{
    // Create an HTTP request.
    std::string url = serverURL + ":" + std::to_string(port) + CONTROLLER_REQUEST_LAST_REMOTE_SAMPLE;
    cpr::Response r = cpr::Get(cpr::Url{url},
        cpr::Parameters{{"jobID", jobID}});

    // Copy the header to a regular map to avoid having the caller depend on cpr 
    // Necessary because the cpr header map uses a custom comparator that the caller 
    // would need to include. The comparator for now is deemed not important, removing it 
    // for simplicity
    std::map<std::string, std::string> header;
    for(auto & [k,v] : r.header)
        header[k] = v;

    return {r.status_code, r.text, header};
}

std::tuple<long, std::string> requestListJobs(
                                const std::string& serverURL,
                                int port)
{
    // Create an HTTP request.
    std::string url = serverURL + ":" + std::to_string(port) + CONTROLLER_REQUEST_LIST_JOB;
    cpr::Response r = cpr::Get(cpr::Url{url});

    return {r.status_code, r.text};
}

std::tuple<long, std::string> requestCancelJob(
                                const std::string& serverURL,
                                int port,
                                const std::string& jobID)
{
    // Create an HTTP request.
    std::string url = serverURL + ":" + std::to_string(port) + CONTROLLER_CANCEL_JOB;
    cpr::Response r = cpr::Post(cpr::Url{url},
        cpr::Parameters{{"jobID", jobID}});

    return {r.status_code, r.text};
}

std::tuple<long, std::string> requestRemoveJob(
                                const std::string& serverURL,
                                int port,
                                const std::string& jobID)
{
    // Create an HTTP request.
    std::string url = serverURL + ":" + std::to_string(port) + CONTROLLER_REMOVE_JOB;
    cpr::Response r = cpr::Post(cpr::Url{url},
        cpr::Parameters{{"jobID", jobID}});

    return {r.status_code, r.text};
}

} // http

} // miquella 