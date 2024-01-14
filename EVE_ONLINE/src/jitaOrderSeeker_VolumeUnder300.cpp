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

    std::ifstream fin("../EVEONLINE_ItemIDs/allOrdersOnJitaIDS.txt");
    std::string line;

    std::ofstream file;
    file.open("../EVEONLINE_ItemIDs/validOrderAtJita_VolumeUnder300_Type_id.txt");

    // Set the request headers
    struct curl_slist *headers = NULL;
    std::string response_data;
    // headers = curl_slist_append(headers, "accept: application/json");
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // Set the request timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    int commaCounter = 0;
    std::string item_ids;
    // Starting to read item id's from txt and getting their prices from jita!!
    while (std::getline(fin, line))
    {
        response_data.clear();
        std::cout << "Line: " << line << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, ("https://esi.evetech.net/latest/universe/types/" + line + "/?datasource=tranquility&language=en").c_str());
        //std::cout << "https://esi.evetech.net/latest/universe/types/" + line + "/?datasource=tranquility&language=en" << std::endl;
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cout << "Error: " << curl_easy_strerror(res) << std::endl;
            continue;
        };

        //checking if response data is valid json
        //jitaOrderSeeker_VolumeUnder300: /usr/include/rapidjson/document.h:1681: const Ch* rapidjson::GenericValue<Encoding, Allocator>::GetString() const [with Encoding = rapidjson::UTF8<>; Allocator = rapidjson::MemoryPoolAllocator<>; rapidjson::GenericValue<Encoding, Allocator>::Ch = char]: Assertion `IsString()' failed.
        if (response_data[0] != '{')
        {
            std::cout << "Skipped because of json" << std::endl;
            continue;
        }

        // Verify if the response data is valid JSON
        Document item_info = nullptr;
        if (item_info.Parse(response_data.c_str()).HasParseError())
        {
            std::cout << "Invalid JSON" << std::endl;
            continue;
        }

        // Print the response data
        // file << response_data << std::endl;
        if (!item_info.HasMember("packaged_volume") || item_info["packaged_volume"].GetFloat() > MAX_item_info || item_info["packaged_volume"].GetFloat() == 0.0)
        {
            std::cout << item_info["name"].GetString() << " Volume:" << item_info["packaged_volume"].GetFloat() << std::endl;
            std::cout << "Skipped because of volume" << std::endl;
            continue;
        };
        //std::cout << item_info["name"].GetString() << " Volume:" << item_info["packaged_volume"].GetFloat() << std::endl;
        
        

        commaCounter++;
        item_ids += line + ",";
        if (commaCounter == 99 || fin.peek() == EOF)
        {
            response_data.clear();
            std::cout << "commaCounter: " << commaCounter << std::endl;
            std::cout << "item_ids: " << item_ids << std::endl;
            commaCounter = 0;
            //deleting last char at in the string item_ids
            item_ids.pop_back();
            // Create a string to hold the response data

            // Disable content decoding
            // curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);

            // getting the prices from jita
            // https://market.fuzzwork.co.uk/aggregates/?station=60003760&types=34,35,36,37,38,39,40
            curl_easy_setopt(curl, CURLOPT_URL, ("https://market.fuzzwork.co.uk/aggregates/?station=60003760&types=" + item_ids).c_str());
            std::cout << "https://market.fuzzwork.co.uk/aggregates/?station=60003760&types=" + item_ids << std::endl;
            item_ids="";

            // Perform the request and handle any errors
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                std::cout << "Error: " << curl_easy_strerror(res) << std::endl;
                continue;
            };

            // filtering scopes dedicated to money
            rapidjson::Document document;
            document.Parse(response_data.c_str());

            if (document.HasParseError())
            {
                std::cerr << "Failed to parse the JSON data." << std::endl;
                return 1;
            }

            if (document.IsObject())
            {
                for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it)
                {
                    const std::string &itemID = it->name.GetString();
                    const rapidjson::Value &item = it->value;

                    if (item.HasMember("sell") && item["sell"].IsObject())
                    {
                        const rapidjson::Value &sell = item["sell"];

                        if (sell.HasMember("min") && sell["min"].IsString())
                        {
                            const std::string &sellMin = sell["min"].GetString();

                            float min_price = stof(sellMin);

                            if (min_price == 0.0)
                            {
                                std::cout << "Skipped because of price" << std::endl;
                                continue;
                            }

                            file << itemID << std::endl;
                            std::cout << "Item ID: " << itemID << ", Sell Min: " << min_price << std::endl;
                        }else{
                            std::cout << "Item doesnt have min field? Item ID: "<< itemID << std::endl;
                        }
                    }else{
                        std::cout << "Item doesnt have sell field?"<< itemID << std::endl;
                    }
                }
            }
        }
    }

    // Print the response data
    // file << response_data << std::endl;

    // 1. Parse a XML string into DOM.
    // pugi::xml_document doc;
    // if (!doc.load_string(response_data.c_str()))
    // {
    //     std::cerr << "Failed to load XML file" << std::endl;
    //     return 1;
    // }

    // // Access exec_api element
    // pugi::xml_node min = doc.child("exec_api").child("marketstat").child("type").child("sell").child("min");
    // float min_price = min.text().as_float();
    // if(min_price == 0.0)
    // {
    //     std::cout << "Skipped because of price" << std::endl;
    //     continue;
    // }
    // std::cout << min.text().get() << '\n';

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

    // Clean up
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
