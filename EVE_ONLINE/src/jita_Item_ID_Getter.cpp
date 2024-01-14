#include "../lib/rapidjson/document.h"
#include "../lib/rapidjson/writer.h"
#include "../lib/rapidjson/stringbuffer.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <fstream>

#define MAX_item_info 300.0

using namespace rapidjson;

// Set up a function to handle the response data
std::size_t write_callback(char *ptr, std::size_t size, std::size_t nmemb, void *userdata)
{
    std::string *response_data = static_cast<std::string *>(userdata);
    std::size_t total_size = size * nmemb;
    response_data->append(ptr, total_size);
    return total_size;
}

int main()
{
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);

    // Create a curl handle
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

    std::ofstream file;
    file.open("../EVEONLINE_ItemIDs/allOrdersOnJitaIDS.txt");

    // Set the request headers
    struct curl_slist *headers = NULL;
    // headers = curl_slist_append(headers, "accept: application/json");
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    headers = curl_slist_append(headers, "accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string response_data;
    // Disable content decoding
    // curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);

    // opening a for loop to iterate page we are getting through api
    for (int i = 1; i <= 17; i++)
    {
        // checking "https://esi.evetech.net/latest/markets/10000002/types/?datasource=tranquility&page=" + std::to_string(i)
        // via printing
        std::cout << "https://esi.evetech.net/latest/markets/10000002/types/?datasource=tranquility&page=" + std::to_string(i) << std::endl;

        // getting the prices from jita
        // curl_easy_setopt(curl, CURLOPT_URL, ("https://esi.evetech.net/latest/universe/types/" + std::to_string(type_id) + "/?datasource=tranquility&language=en").c_str());
        curl_easy_setopt(curl, CURLOPT_URL, ("https://esi.evetech.net/latest/markets/10000002/types/?datasource=tranquility&page=" + std::to_string(i)).c_str());

        // Set the request timeout
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        // Perform the request and handle any errors
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cout << "Error: " << curl_easy_strerror(res) << std::endl;
            std::cout << "Error: " << res << std::endl;
        };

        // Starting to read item id's from txt and getting their prices from jita!!

        // Create a string to hold the response data

        // Print the response data

        // editing response data

        // Remove the brackets from the string
        response_data = response_data.substr(1, response_data.length() - 2);

        // changing all the  ,  to \n
        for (long unsigned int i = 0; i < response_data.length(); i++)
        {
            if (response_data[i] == ',')
            {
                response_data[i] = '\n';
            }
        }

        file << response_data << std::endl;

        // // Split the string into individual numbers
        // std::stringstream ss(response_data);
        // std::string num;

        // while (std::getline(ss, num, ','))
        // {
        //     file << num << std::endl;

        // }
        // Reset the Curl transfer handle and response data buffer for the next request
        // curl_easy_reset(curl);
        response_data.clear();
        // getting items volume from https://esi.evetech.net/latest/universe/types/<type_id>/?datasource=tranquility&language=en
    }

    // Clean up
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
