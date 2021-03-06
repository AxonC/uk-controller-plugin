#pragma once

#include "curl/CurlRequest.h"

namespace UKControllerPlugin {
    namespace Api {

        /*
            Responsible for converting a set of input data into a cURL request
            that can sent to the API.
        */
        class ApiRequestBuilder
        {
            public:
                ApiRequestBuilder(std::string apiDomain, std::string apiKey);
                UKControllerPlugin::Curl::CurlRequest BuildAuthCheckRequest(void) const;
                UKControllerPlugin::Curl::CurlRequest BuildDependencyListRequest(void) const;
                UKControllerPlugin::Curl::CurlRequest BuildSquawkAssignmentCheckRequest(std::string callsign) const;
                UKControllerPlugin::Curl::CurlRequest BuildSquawkAssignmentDeletionRequest(std::string callsign) const;
                UKControllerPlugin::Curl::CurlRequest BuildLocalSquawkAssignmentRequest(
                    std::string callsign,
                    std::string unit,
                    std::string flightRules
                ) const;
                UKControllerPlugin::Curl::CurlRequest BuildGeneralSquawkAssignmentRequest(
                    std::string callsign,
                    std::string origin,
                    std::string destination
                ) const;
                UKControllerPlugin::Curl::CurlRequest BuildRemoteFileRequest(std::string uri) const;
                UKControllerPlugin::Curl::CurlRequest BuildVersionCheckRequest(std::string versionString) const;
                std::string GetApiDomain(void) const;
                std::string GetApiKey(void) const;

                // The type string to send in the payload if we want a general squawk
                const std::string generalSquawkAssignmentType = "general";

                // The type string to send in the payload if we want a local squawk
                const std::string localSquawkAssignmentType = "local";

            private:

                UKControllerPlugin::Curl::CurlRequest AddCommonHeaders(
                    UKControllerPlugin::Curl::CurlRequest request
                ) const;

                // The base URL of the API
                const std::string apiDomain;

                // Our API key
                const std::string apiKey;
        };
    }  // namespace Api
}  // namespace UKControllerPlugin
