// rapidjson/example/simpledom/simpledom.cpp`
#include "/usr/include/rapidjson/document.h"
#include "/usr/include/rapidjson/writer.h"
#include "/usr/include/rapidjson/stringbuffer.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <pugixml.hpp>
#include <chrono>
#include <iomanip>

#define MAX_item_order_history 300.0
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

    std::ifstream fin("validFlagOrderAtJita_VolumeUnder300_Type_id.txt");
    std::string line;

    std::ofstream file;
    file.open("top50.txt");

    // Get the current date
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    auto currentDate = *std::localtime(&nowTime);

    // Set the request headers
    struct curl_slist *headers = NULL;
    // headers = curl_slist_append(headers, "accept: application/json");
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Starting to read item id's from txt and getting their prices from jita!!
    while (std::getline(fin, line))
    {
        int type_id;
        std::istringstream(line) >> type_id;
        // Create a string to hold the response data
        std::string response_data;
        // Disable content decoding
        // curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);

        // getting the prices from jita
        curl_easy_setopt(curl, CURLOPT_URL, ("https://esi.evetech.net/latest/markets/10000066/history/?datasource=tranquility&type_id=" + std::to_string(type_id)).c_str());

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
            continue;
        };

        // Print the response data
        // file << response_data << std::endl;

        Document item_order_history;
        item_order_history.Parse(response_data.c_str());

        // Get the number of objects in the JSON array
        int numObjs = item_order_history.Size();
        if (numObjs < 6)
        {
            std::cout << "Skipped because of no data" << std::endl;
            continue;
        }

        int recentVolume = 0;
        int tolarance = 0;
        bool validFlag = false;
        int price=-1;
        // Loop through the objects in the JSON array
        for (Value::ConstValueIterator itr = item_order_history.End(); itr != item_order_history[numObjs - 6]; --itr)
        {
            // Get the date string from the object
            const char *dateString = (*itr)["date"].GetString();
            // Convert the date string to a std::tm object
            std::tm objDateTime = {};
            std::istringstream ss(dateString);
            ss >> std::get_time(&objDateTime, "%Y-%m-%d");
            // Calculate the difference in days
            auto currentTP = std::chrono::time_point_cast<std::chrono::hours>(std::chrono::system_clock::from_time_t(nowTime));
            auto objTP = std::chrono::time_point_cast<std::chrono::hours>(std::chrono::system_clock::from_time_t(std::mktime(&objDateTime)));
            auto diff = std::chrono::duration_cast<std::chrono::hours>(currentTP - objTP);
            int diff_days = diff.count() / 24; // Check if the difference is within 5 days
            if (diff_days <= 5)
            {
                // Get the volume
                recentVolume += (*itr)["volume"].GetInt();
                if(price < (*itr)["lowest"].GetInt()){
                    price = (*itr)["lowest"].GetInt();
                }

            }
            else if (diff_days > 5 && tolarance < 2)
            {
                recentVolume += (*itr)["volume"].GetInt();
                tolarance++;
                if(price < (*itr)["lowest"].GetInt()){
                    price = (*itr)["lowest"].GetInt();
                }
            }
            else
            {
                validFlag = true;
                break;
            }
        }
        if (validFlag){
            continue;
        }
        //after that we have the recent volume of the item and we have price of it
        


        // 2. Modify it by DOM.
        // Value &s = d["players"];
        // s.SetInt(s.GetInt() + 1);

        // 3. Stringify the DOM
        // StringBuffer buffer;
        // Writer<StringBuffer> writer(buffer);
        // d.Accept(writer);

        // Output {"project":"rapidjson","stars":11}
        // std::cout << buffer.GetString() << std::endl;

        // Reset the Curl transfer handle and response data buffer for the next request
        // curl_easy_reset(curl);
        response_data.clear();
        // getting items volume from https://esi.evetech.net/latest/universe/types/<type_id>/?datasource=tranquility&language=en
        curl_easy_setopt(curl, CURLOPT_URL, ("https://esi.evetech.net/latest/universe/types/" + std::to_string(type_id) + "/?datasource=tranquility&language=en").c_str());
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cout << "Error: " << curl_easy_strerror(res) << std::endl;
            continue;
        };

        // Print the response data
        // file << response_data << std::endl;

        Document item_order_history;
        item_order_history.Parse(response_data.c_str());
        if (!item_order_history.HasMember("packaged_volume") || item_order_history["packaged_volume"].GetFloat() > MAX_item_order_history || item_order_history["packaged_volume"].GetFloat() == 0.0)
        {
            std::cout << "Skipped because of volume" << std::endl;
            continue;
        };
        // file << item_order_history["name"].GetString() << " Volume:" << item_order_history["packaged_volume"].GetFloat() << " Single Jita sell price:" << min.text().get() << std::endl;
        file << type_id << std::endl;
        response_data.clear();
    }

    // Clean up
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
