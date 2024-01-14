#include "../lib/rapidjson/document.h"
#include "../lib/rapidjson/writer.h"
#include "../lib/rapidjson/stringbuffer.h"
#include "../lib/SFML/Audio.hpp"
#include <curl/curl.h>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <queue>
#include <iostream>
#include <algorithm>

#define BUDGET 75000000.0
#define REQUIRED_RECENCT_VOLUME 30
using namespace rapidjson;


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
    double profit_per_isk;
    double jita_price;
    double packet_Jita_Price;
    double mj_price;
    double packet_mj_price;
    double recent_volume;
    int packet_volume;
    double net_Profit;

    Item()
    {
        type_id = 0;
        profit_per_isk = 0.0;
        jita_price = 0.0;        
        packet_Jita_Price = 0.0;
        mj_price = 0.0;
        packet_mj_price = 0.0;
        recent_volume = 0.0;
        packet_volume = 0;
        net_Profit = 0;
    }

    Item(Item &a)
    {
        type_id = a.type_id;
        profit_per_isk = a.profit_per_isk;
        jita_price = a.jita_price;
        packet_Jita_Price = a.packet_Jita_Price;
        mj_price = a.mj_price;
        packet_mj_price = a.packet_mj_price;
        recent_volume = a.recent_volume;
        packet_volume = a.packet_volume;
        net_Profit = a.net_Profit;
    }

    std::string getString()
    {
        std::stringstream ss;
        ss << std::left << std::setw(6)  << "| ID: " << std::right << std::setw(8)<<this->type_id
           << std::left << std::setw(16) << "| Packet volume: " << std::right << std::setw(7) << this->packet_volume
           << std::left << std::setw(20) << "| Jita price: " << std::right << std::setw(14) << formatNumber(this->jita_price)
           << std::left << std::setw(20) << "| Packet Jita price: " << std::right << std::setw(14) << formatNumber(this->packet_Jita_Price)
           << std::left << std::setw(22) << "| MJ Sell Price: " << std::right << std::setw(14) << formatNumber(this->mj_price)
           << std::left << std::setw(22) << "| Packet MJ Sell Price: " << std::right << std::setw(14) << formatNumber(this->packet_mj_price)
           << std::left << std::setw(17) << "| Profit per isk: " << std::right << std::setw(10) << this->profit_per_isk
           << std::left << std::setw(13) << "| Net Profit: " << std::right << std::setw(14) << formatNumber(this->net_Profit) << std::endl;

        return ss.str();
    }

    Item &operator=(const Item &other)
    {
        if (this != &other)
        {
            // Copy the data members of 'other' into this object
            this->type_id = other.type_id;
            this->profit_per_isk = other.profit_per_isk;
            this->jita_price = other.jita_price;
            this->packet_Jita_Price = other.packet_Jita_Price;
            this->mj_price = other.mj_price;
            this->packet_mj_price = other.packet_mj_price;
            this->recent_volume = other.recent_volume;
            this->packet_volume = other.packet_volume;
            this->net_Profit = other.net_Profit;
        }
        return *this;
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
        int height;
        Node(Item v)
        {
            val.operator=(v);
            left = nullptr;
            right = nullptr;
            height=1;
        };
    };

    Node *root;

    BST()
    {
        root = nullptr;
    }

    void insert(Item val)
    {
        root = insertNode(root, val);
    };

    Item *search(int type_id)
    {
        Node *curr = root;

        while (curr)
        {
            if (curr->val.type_id == type_id)
            {
                return &curr->val;
            }
            else if (curr->val.type_id > type_id)
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

private:
    int height(Node *node)
    {
        if (node == nullptr)
            return 0;
        return node->height;
    }

    int getBalance(Node *node)
    {
        if (node == nullptr)
            return 0;
        return height(node->left) - height(node->right);
    }

    Node *rightRotate(Node *y)
    {
        Node *x = y->left;
        Node *T2 = x->right;
        x->right = y;

        y->left = T2;

        y->height = std::max(height(y->left), height(y->right)) + 1;
        x->height = std::max(height(x->left), height(x->right)) + 1;

        return x;
    }

    Node *leftRotate(Node *x)
    {
        Node *y = x->right;
        Node *T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = std::max(height(x->left), height(x->right)) + 1;
        y->height = std::max(height(y->left), height(y->right)) + 1;

        return y;
    }

    Node *insertNode(Node *node, Item val)
    {
        if (node == nullptr)
        {
            Node *newNode = new Node(val);
            return newNode;
        }

        if (val.type_id < node->val.type_id)
            node->left = insertNode(node->left, val);
        else
            node->right = insertNode(node->right, val);

        node->height = std::max(height(node->left), height(node->right)) + 1;

        int balance = getBalance(node);

        if (balance > 1 && val.type_id < node->left->val.type_id)
            return rightRotate(node);

        if (balance < -1 && val.type_id > node->right->val.type_id)
            return leftRotate(node);

        if (balance > 1 && val.type_id > node->left->val.type_id)
        {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if (balance < -1 && val.type_id < node->right->val.type_id)
        {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }
    
};

// Set up a function to handle the response data
std::size_t write_callback(char *ptr, std::size_t size, std::size_t nmemb, void *userdata)
{
    std::string *response_data = static_cast<std::string *>(userdata);
    std::size_t total_size = size * nmemb;
    response_data->append(ptr, total_size);
    return total_size;
}

class LinkedList
{
    struct Node
{
    Item *item;
    Node *next;

    Node(Item *item) : item(item), next(nullptr) {}
};
public:
    LinkedList() : head(nullptr) {}

    ~LinkedList()
    {
        clear();
    }

    void insert(Item *item)
    {
        if (head == nullptr)
        {
            head = new Node(item);
        }
        else if (item->net_Profit > head->item->net_Profit)
        {
            Node *newNode = new Node(item);
            newNode->next = head;
            head = newNode;
        }
        else
        {
            Node *current = head;
            while (current->next != nullptr && item->net_Profit <= current->next->item->net_Profit)
            {
                current = current->next;
            }
            Node *newNode = new Node(item);
            newNode->next = current->next;
            current->next = newNode;
        }
    }

    void print()
    {
        Node *current = head;
        while (current != nullptr)
        {
            std::cout << current->item->net_Profit << " ";
            current = current->next;
        }
        std::cout << std::endl;
    }

    //get first 50 items
    std::queue<Item *> getTop50()
    {
        std::queue<Item *> a;
        Node *current = head;
        int counter = 0;
        while (current != nullptr && counter < 50)
        {
            a.push(current->item);
            current = current->next;
            counter++;
        }
        return a;
    }

    void clear()
    {
        Node *current = head;
        while (current != nullptr)
        {
            Node *next = current->next;
            delete current->item;
            delete current;
            current = next;
        }
        head = nullptr;
    }

private:
    Node *head;
};

int main()
{
    //adding them into a ordered linked list. Order is based on expected packet sell price
    //making this linked list
    
    std::cout << "Program has started! Budget: " << formatNumber(BUDGET) << std::endl;
    BST bst;
    LinkedList ll;
    // opening file stream
    std::ofstream file;
    file.open("../top50.txt");
    int type_id;

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);

    // Create a curl handle
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

    std::ifstream fin("../EVEONLINE_ItemIDs/validOrderAtJita_VolumeUnder300_Type_id.txt");
    std::string line;

    // Get the current date
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);

    // Set the request headers
    struct curl_slist *headers = NULL;
    // headers = curl_slist_append(headers, "accept: application/json");
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // TODO: Need to store successfull items in a BST

    // Starting to read item id's from txt and getting their mjPrices from jita!!

    int commaCounter = 0;
    std::stringstream ss;
    while (std::getline(fin, line))
    {
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
        if (!item_order_history.IsArray())
        {
            // handle error
            std::cerr << "Expected JSON array, but got something else" << std::endl;
            continue;
        }

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
        double mjPrice = -1;
        double mjPrice2 = std::numeric_limits<double>::max();
        // Loop through the objects in the JSON array
        // Looping through the last 6 entry
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
                }else if(mjPrice2 > (*itr)["highest"].GetDouble())
                {
                    mjPrice2 = (*itr)["highest"].GetDouble();
                }
            }
            else if (diff_days > 5 && tolarance < 2)
            {
                recentVolume += (*itr)["volume"].GetInt();
                tolarance++;
                if (mjPrice < (*itr)["lowest"].GetDouble())
                {
                    mjPrice = (*itr)["lowest"].GetDouble();
                }else if(mjPrice2 > (*itr)["highest"].GetDouble())
                {
                    mjPrice2 = (*itr)["highest"].GetDouble();
                }
            }
            else
            {
                validFlag = true;
                break;
            }
        }
        if (validFlag || recentVolume < REQUIRED_RECENCT_VOLUME)
        {
            std::cout << "Skipped because of no recent data or recent volume: " << recentVolume << std::endl;
            continue;
        }
        // now we have recent volume and mjPrice
        // we need to get the jita price of the item
        mjPrice=(mjPrice+mjPrice2)/2;
        Item *item = new Item();
        item->type_id = type_id;
        item->mj_price = mjPrice;
        item->recent_volume = recentVolume;
        bst.insert(*item);

        // after that we have the recent volume of the item and we have mjPrice of it
        // file << type_id << " recentVolume: " << recentVolume << " mjPrice: " << mjPrice << std::endl;
        // Reset the Curl transfer handle and response data buffer for the next request
        // curl_easy_reset(curl);
        response_data.clear();
        // getting items volume from https://esi.evetech.net/latest/universe/types/<type_id>/?datasource=tranquility&language=en

        commaCounter++;
        
        ss << line <<",";
        std::string str = ss.str();
        if (commaCounter == 99 || fin.peek() == EOF)
        {
            std::string c = str;
            std::getline(ss, c);
            //now removing , and creating 3 strings from the line
            std::string item_ids;
            ss=std::stringstream("");
            //now making item_ids = line
            item_ids = c;
            //removing last char from item_ids
            item_ids.pop_back();
            std::cout << "commaCounter: " << commaCounter << std::endl;
            std::cout << "item_ids: " << item_ids << std::endl;
            commaCounter = 0;
            // deleting last char at in the string item_ids
            // Create a string to hold the response data

            // Disable content decoding
            // curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 0);

            // getting the prices from jita
            // https://market.fuzzwork.co.uk/aggregates/?station=60003760&types=34,35,36,37,38,39,40
            curl_easy_setopt(curl, CURLOPT_URL, ("https://market.fuzzwork.co.uk/aggregates/?station=60003760&types=" + item_ids).c_str());
            std::cout << "https://market.fuzzwork.co.uk/aggregates/?station=60003760&types=" + item_ids << std::endl;
            item_ids = "";

            // Perform the request and handle any errors
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                std::cout << "Error: " << curl_easy_strerror(res) << std::endl;
                continue;
            };

            // Print the response data
            //std::cout << response_data << std::endl;

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
                    const rapidjson::Value &iteratedItem = it->value;
                    float jitaPrice;

                    if (iteratedItem.HasMember("sell") && iteratedItem["sell"].IsObject())
                    {
                        const rapidjson::Value &sell = iteratedItem["sell"];

                        if (sell.HasMember("min") && sell["min"].IsString())
                        {
                            const std::string &sellMin = sell["min"].GetString();

                            jitaPrice = stof(sellMin);

                            if (jitaPrice == 0.0)
                            {
                                std::cout << "Skipped because of price" << std::endl;
                                continue;
                            }

                            // file << item_order_history["name"].GetString() << " Volume:" << item_order_history["packaged_volume"].GetFloat() << " Single Jita sell mjPrice:" << min.text().get() << std::endl;
                            // file << type_id << " MJ Price: " << mjPrice << " MJ Recent Volume: " << recentVolume << " Jita price: " << jitaPrice << std::endl;
                            response_data.clear();
                        }
                        else
                        {
                            std::cout << "Item doesnt have min field? Item ID: " << itemID << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "Item doesnt have sell field?" << itemID << std::endl;
                    }

                    Item *item = bst.search(stoi(itemID));

                    // Starting PACKET ANALYSIS
                    // one packet is 2 days volume of the item
                    // to find 2 days volume we need to divide recentVolume by 5 and multiply by 2
                    int packetVolume = item->recent_volume / 5 * 2;
                    // now we need to calculate packets price and expected sell price
                    double packetPrice = jitaPrice * packetVolume;
                    if (packetPrice > BUDGET)
                    {
                        std::cout << "Skipped because of packetPrice > BUDGET" << std::endl;
                        continue;
                    }
                    // std::cout << std::fixed << std::setprecision(2);
                    //std::cout<<packetPrice<<" "<<jitaPrice<<" "<<packetVolume<<std::endl;
                    double expectedPackedSellPrice = item->mj_price * packetVolume;

                    // now we need to calculate profit
                    double profit = expectedPackedSellPrice - packetPrice;
                    // since we have a limited amount of money we can invest and we want to distribute our budget into different items
                    // items with higher profit percentage will be more profitable
                    // so we need to calculate profit per isk
                    double profitPerIsk = profit / packetPrice;
                    item->jita_price = jitaPrice;
                    item->packet_Jita_Price = packetPrice;
                    item->packet_mj_price = expectedPackedSellPrice;
                    item->packet_volume = packetVolume;
                    item->profit_per_isk = profitPerIsk;
                    item->net_Profit = profit;
                    //adding them into a ordered linked list. Order is based on expected packet sell price
                    ll.insert(item);
                    
                }
                
            }
        }
    }

    std::queue<Item *> a = ll.getTop50();

    while (!a.empty())
    {
        Item *item = a.front();
        a.pop();
        file << item->getString();
        // file << item->type_id << " Packet MJ Sell Price: " << formatNumber(item->mj_price) << " Packet volume: " << item->packet_volume << " Packet Jita price: " << formatNumber(item->packet_Jita_Price ) << " Profit percentage: " << item->profit_percentage << " Profit per isk: " << item->profit_per_isk << " Net Profit: "<< formatNumber(item->net_Profit) << std::endl;
    }
    
    // bst.printAll();
    //  Clean up
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    file.close();
    fin.close();
    //playing mp3 file inside resources folder ../res/notification.mp3
    sf::Music music;
    if (!music.openFromFile("../res/notification.mp3")) {
        // Handle error
        return -1;
    }

    music.play();
    // opening txt file in notepad and closing program
    //system("gedit ./top50.txt");
    return 0;
}