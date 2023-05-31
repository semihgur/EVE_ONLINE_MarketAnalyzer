#include "/usr/include/rapidjson/document.h"
#include "/usr/include/rapidjson/writer.h"
#include "/usr/include/rapidjson/stringbuffer.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <pugixml.hpp>

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

    std::ifstream fin("item_id.txt");
    std::string line;

    std::ofstream file;
    file.open("valid_type_ids.txt");

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
        curl_easy_setopt(curl, CURLOPT_URL, ("https://api.evemarketer.com/ec/marketstat?typeid=" + std::to_string(type_id) + "&usesystem=30000142").c_str());

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
        //file << response_data << std::endl;

        // 1. Parse a XML string into DOM.
        pugi::xml_document doc;
        if (!doc.load_string(response_data.c_str()))
        {
            std::cerr << "Failed to load XML file" << std::endl;
            return 1;
        }

        // Access exec_api element
        pugi::xml_node min = doc.child("exec_api").child("marketstat").child("type").child("sell").child("min");
        float min_price = min.text().as_float();
        if(min_price == 0.0)
        {
            std::cout << "Skipped because of price" << std::endl;
            continue;
        }
        //std::cout << min.text().get() << '\n';

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
        //file << response_data << std::endl;

        Document item_info;
        item_info.Parse(response_data.c_str());
        if (!item_info.HasMember("packaged_volume") || item_info["packaged_volume"].GetFloat() > MAX_item_info || item_info["packaged_volume"].GetFloat() == 0.0)
        {
            std::cout << "Skipped because of volume" << std::endl;
            continue;
        };
        //file << item_info["name"].GetString() << " Volume:" << item_info["packaged_volume"].GetFloat() << " Single Jita sell price:" << min.text().get() << std::endl;
        file << type_id << std::endl;
        response_data.clear();
    }

    // Clean up
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
