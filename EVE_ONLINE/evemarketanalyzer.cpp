#include "/usr/include/rapidjson/document.h"
#include "/usr/include/rapidjson/writer.h"
#include "/usr/include/rapidjson/stringbuffer.h"
#include <curl/curl.h>
#include <sstream>
#include <fstream>
#include <pugixml.hpp>
#include <chrono>
#include <iomanip>
#include <queue>
#include <iostream>
#include <algorithm>
#include <exception>
// 750-2000
#define MAX_item_order_history 300.0
#define BUDGET 500000000.0

std::string formatNumber(double number)
{
    // Convert number to a string with thousands separator
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << std::setprecision(2) << std::setw(12) << std::right << number;
    std::string numberStr = ss.str();

    // Replace the thousands separator with the appropriate character for the current locale
    char sep = std::use_facet<std::numpunct<char>>(ss.getloc()).thousands_sep();
    std::replace(numberStr.begin(), numberStr.end(), sep, ',');

    return numberStr;
}

struct Item
{
    int type_id;
    double profit_percentage;
    double profit_per_isk;
    double packet_Jita_Price;
    double mj_price;
    int packet_volume;
    double net_Profit;

    Item()
    {
        type_id = 0;
        profit_percentage = 0.0;
        profit_per_isk = 0.0;
        packet_Jita_Price = 0.0;
        mj_price = 0.0;
        int packet_volume = 0;
        double net_Profit = 0;
    }

    Item(Item &a)
    {
        type_id = a.type_id;
        profit_percentage = a.profit_percentage;
        profit_per_isk = a.profit_per_isk;
        packet_Jita_Price = a.packet_Jita_Price;
        mj_price = a.mj_price;
        packet_volume = a.packet_volume;
        net_Profit = a.net_Profit;
    }

    void print()
    {
        std::cout << std::left << std::setw(6) << this->type_id
                  << std::left << std::setw(22) << "| Packet MJ Sell Price: " << std::right << std::setw(14) << formatNumber(this->mj_price)
                  << std::left << std::setw(16) << "| Packet volume: " << std::right << std::setw(7) << this->packet_volume
                  << std::left << std::setw(20) << "| Packet Jita price: " << std::right << std::setw(14) << formatNumber(this->packet_Jita_Price)
                  << std::left << std::setw(20) << "| Profit percentage: " << std::right << std::setw(10) << this->profit_percentage
                  << std::left << std::setw(17) << "| Profit per isk: " << std::right << std::setw(10) << this->profit_per_isk
                  << std::left << std::setw(13) << "| Net Profit: " << std::right << std::setw(14) << formatNumber(this->net_Profit) << std::endl;
    }
};

struct Compare
{
    bool operator()(Item *a, Item *b)
    {
        return a->net_Profit < b->net_Profit;
    }
};

class BST
{
public:
    struct Node
    {
        Item val;
        Node *left;
        Node *right;
        Node(Item v)
        {
            val = v;
            left = nullptr;
            right = nullptr;
        };
    };

    Node *root;
    Compare cmp;
    int count;

    BST()
    {
        root = nullptr;
        count = 0;
    }

    void insert(Item val)
    {
        if (val.net_Profit < 10000000.0)
        {
            // std::cout << "Item " << val.type_id << " has net profit " << val.net_Profit << " which is less than 10M" << std::endl;
            return;
        }

        // Insertion logic using cmp function
        Node *newNode = new Node(val);

        if (!root)
        {
            root = newNode;
            return;
        }

        Node *curr = root;
        while (curr)
        {
            if (curr->val.net_Profit > val.net_Profit)
            {
                if (curr->left)
                    curr = curr->left;
                else
                {
                    curr->left = newNode;
                    break;
                }
            }
            else
            {
                if (curr->right)
                    curr = curr->right;
                else
                {
                    curr->right = newNode;
                    break;
                }
            }
        }
    };
    Item *search(double net_Profit)
    {
        Node *curr = root;

        while (curr)
        {
            if (curr->val.net_Profit == net_Profit)
            {
                return &(curr->val);
            }
            else if (curr->val.net_Profit > net_Profit)
            {
                curr = curr->left;
            }
            else
            {
                curr = curr->right;
            }
        }

        return nullptr;
    };
    std::queue<Item *> getTop50()
    {
        std::queue<Item *> maxHeap;

        inOrder(root, maxHeap);
        // maxHeap now contains top 50 Items

        return maxHeap;
    };
    void printAll()
    {
        printInOrder(root);
    };

private:
    void inOrder(Node *node, std::queue<Item *> &maxHeap)
    {
        if (!node)
            return;

        inOrder(node->left, maxHeap);

        if (maxHeap.size() < 50)
        {
            maxHeap.push(&node->val);
        }
        else
        {
            return;
        }

        inOrder(node->right, maxHeap);
    };
    void printInOrder(Node *node)
    {
        if (node == nullptr)
            return;

        printInOrder(node->left);
        node->val.print();
        printInOrder(node->right);
    };
};

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
    std::cout << "Program has started2! " << std::endl;
    BST bst;
    // opening file stream
    std::ofstream file;
    file.open("top50.txt");
    int type_id;
    try
    {
        // Initialize libcurl
        curl_global_init(CURL_GLOBAL_ALL);

        // Create a curl handle
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

        std::ifstream fin("validOrderAtJita_VolumeUnder300_Type_id.txt");
        std::string line;

        // Get the current date
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        auto currentDate = *std::localtime(&nowTime);

        // Set the request headers
        struct curl_slist *headers = NULL;
        // headers = curl_slist_append(headers, "accept: application/json");
        headers = curl_slist_append(headers, "Cache-Control: no-cache");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // TODO: Need to store successfull items in a BST

        // Starting to read item id's from txt and getting their mjPrices from jita!!
        int aaa=0;
        while (std::getline(fin, line))
        {
            aaa++;

            std::istringstream(line) >> type_id;
            // Create a string to hold the response data
            std::string response_data;
            // Disable content decoding
            // curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);

            // getting the mjPrice history of the item
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
            

            if (item_order_history.HasParseError())
            {
                switch (item_order_history.GetParseError())
                {
                case rapidjson::kParseErrorNone:
                    // this case should never happen, since we're checking for a parse error
                    std::cerr << "Unknown parsing error" << std::endl;
                    break;
                case rapidjson::kParseErrorDocumentEmpty:
                    std::cerr << "The JSON document is empty" << std::endl;
                    break;
                case rapidjson::kParseErrorDocumentRootNotSingular:
                    std::cerr << "The JSON document must contain a single root element" << std::endl;
                    break;
                // add more cases for other error codes as needed
                default:
                    std::cerr << "Unknown parsing error code: " << item_order_history.GetParseError() << std::endl;
                    break;
                }
                continue;
            }

            // check if the document is an array
            if (!item_order_history.IsArray()) {
                // handle error
                std::cerr << "Expected JSON array, but got something else" << std::endl;
                continue;
            }

            // Get the number of objects in the JSON array
            int numObjs = item_order_history.Size();
            if (numObjs < 6)
            {
                // std::cout << "Skipped because of no data" << std::endl;
                continue;
            }

            int recentVolume = 0;
            int tolarance = 0;
            bool validFlag = false;
            double mjPrice = -1;
            // Loop through the objects in the JSON array
            for (rapidjson::Value::ConstValueIterator itr = item_order_history.End() - 1; itr != std::prev(item_order_history.End(), 6); --itr)
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
                    if (mjPrice < (*itr)["lowest"].GetDouble())
                    {
                        mjPrice = (*itr)["lowest"].GetDouble();
                    }
                }
                else if (diff_days > 5 && tolarance < 2)
                {
                    recentVolume += (*itr)["volume"].GetInt();
                    tolarance++;
                    if (mjPrice < (*itr)["lowest"].GetDouble())
                    {
                        mjPrice = (*itr)["lowest"].GetDouble();
                    }
                }
                else
                {
                    validFlag = true;
                    break;
                }
            }
            if (validFlag)
            {
                continue;
            }
            // after that we have the recent volume of the item and we have mjPrice of it
            // file << type_id << " recentVolume: " << recentVolume << " mjPrice: " << mjPrice << std::endl;
            // Reset the Curl transfer handle and response data buffer for the next request
            // curl_easy_reset(curl);
            response_data.clear();
            // getting items volume from https://esi.evetech.net/latest/universe/types/<type_id>/?datasource=tranquility&language=en
            curl_easy_setopt(curl, CURLOPT_URL, ("https://api.evemarketer.com/ec/marketstat?typeid=" + std::to_string(type_id) + "&usesystem=30000142").c_str());
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                std::cout << "Error: " << curl_easy_strerror(res) << std::endl;
                continue;
            };

            // Print the response data
            // file << response_data << std::endl;

            // 1. Parse a XML string into DOM.
            pugi::xml_document doc;
            if (!doc.load_string(response_data.c_str()))
            {
                std::cerr << "Failed to load XML file" << std::endl;
                return 1;
            }

            // Access exec_api element
            pugi::xml_node min = doc.child("exec_api").child("marketstat").child("type").child("sell").child("min");
            double jitaPrice = min.text().as_double();
            if (jitaPrice == 0.0)
            {
                std::cout << "Skipped because of jitaPrice = 0" << std::endl;
                continue;
            }

            // file << item_order_history["name"].GetString() << " Volume:" << item_order_history["packaged_volume"].GetFloat() << " Single Jita sell mjPrice:" << min.text().get() << std::endl;
            // file << type_id << " MJ Price: " << mjPrice << " MJ Recent Volume: " << recentVolume << " Jita price: " << jitaPrice << std::endl;
            response_data.clear();

            // Starting PACKET ANALYSIS
            // one packet is 2 days volume of the item
            // to find 2 days volume we need to divide recentVolume by 5 and multiply by 2
            int packetVolume = recentVolume / 5 * 2;
            // now we need to calculate packets price and expected sell price
            double packetPrice = jitaPrice * packetVolume;
            if (packetPrice > BUDGET)
            {
                std::cout << "Skipped because of packetPrice > BUDGET" << std::endl;
                std::cout << ((float)aaa/13619)*100 << std::endl;
                continue;
            }
            // std::cout << std::fixed << std::setprecision(2);
            // std::cout<<packetPrice<<" "<<jitaPrice<<" "<<packetVolume<<std::endl;
            double expectedPackedSellPrice = mjPrice * packetVolume;
            // now we need to calculate profit
            double profit = expectedPackedSellPrice - packetPrice;
            // now we need to calculate profit percentage
            double profitPercentage = profit / packetPrice * 100;
            // since we have a limited amount of money we can invest and we want to distribute our budget into different items
            // items with higher profit percentage will be more profitable
            // so we need to calculate profit per isk
            double profitPerIsk = profit / packetPrice;
            Item item;
            item.type_id = type_id;
            item.packet_Jita_Price = packetPrice;
            item.mj_price = expectedPackedSellPrice;
            item.packet_volume = packetVolume;
            item.profit_per_isk = profitPerIsk;
            item.profit_percentage = profitPercentage;
            item.net_Profit = profit;
            bst.insert(item);
            
        }

        std::queue<Item *> a = bst.getTop50();

        while (!a.empty())
        {
            Item *item = a.front();
            a.pop();
            file << std::left << std::setw(6) << item->type_id
                 << std::left << std::setw(22) << "| Packet MJ Sell Price: " << std::right << std::setw(14) << formatNumber(item->mj_price)
                 << std::left << std::setw(16) << "| Packet volume: " << std::right << std::setw(7) << item->packet_volume
                 << std::left << std::setw(20) << "| Packet Jita price: " << std::right << std::setw(14) << formatNumber(item->packet_Jita_Price)
                 << std::left << std::setw(20) << "| Profit percentage: " << std::right << std::setw(10) << item->profit_percentage
                 << std::left << std::setw(17) << "| Profit per isk: " << std::right << std::setw(10) << item->profit_per_isk
                 << std::left << std::setw(13) << "| Net Profit: " << std::right << std::setw(14) << formatNumber(item->net_Profit) << std::endl;
            // file << item->type_id << " Packet MJ Sell Price: " << formatNumber(item->mj_price) << " Packet volume: " << item->packet_volume << " Packet Jita price: " << formatNumber(item->packet_Jita_Price ) << " Profit percentage: " << item->profit_percentage << " Profit per isk: " << item->profit_per_isk << " Net Profit: "<< formatNumber(item->net_Profit) << std::endl;
        }
        // bst.printAll();
        //  Clean up
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        return 0;
    }
    catch (const std::exception &e)
    {
        // Handle the exception and print an error message
        std::cerr << "An exception occurred: " << e.what() << std::endl
                  << "Last type_id: " << type_id << std::endl;
        std::queue<Item *> a = bst.getTop50();

        while (!a.empty())
        {
            Item *item = a.front();
            a.pop();
            file << std::left << std::setw(6) << item->type_id
                 << std::left << std::setw(22) << "| Packet MJ Sell Price: " << std::right << std::setw(14) << formatNumber(item->mj_price)
                 << std::left << std::setw(16) << "| Packet volume: " << std::right << std::setw(7) << item->packet_volume
                 << std::left << std::setw(20) << "| Packet Jita price: " << std::right << std::setw(14) << formatNumber(item->packet_Jita_Price)
                 << std::left << std::setw(20) << "| Profit percentage: " << std::right << std::setw(10) << item->profit_percentage
                 << std::left << std::setw(17) << "| Profit per isk: " << std::right << std::setw(10) << item->profit_per_isk
                 << std::left << std::setw(13) << "| Net Profit: " << std::right << std::setw(14) << formatNumber(item->net_Profit) << std::endl;
            // file << item->type_id << " Packet MJ Sell Price: " << formatNumber(item->mj_price) << " Packet volume: " << item->packet_volume << " Packet Jita price: " << formatNumber(item->packet_Jita_Price ) << " Profit percentage: " << item->profit_percentage << " Profit per isk: " << item->profit_per_isk << " Net Profit: "<< formatNumber(item->net_Profit) << std::endl;
        }
    }
}