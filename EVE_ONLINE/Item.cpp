#include <iostream>
#include "Item.h"
struct Item
{
    int type_id;
    double profit_percentage;
    double profit_per_isk;
    double packet_Jita_Price;
    double mj_price;
    int packet_volume;

    Item(){
        type_id = 0;
        profit_percentage = 0.0;
        profit_per_isk = 0.0;
        packet_Jita_Price = 0.0;
        mj_price = 0.0;
        int packet_volume=0;
    }

    Item(Item& a){
        type_id = a.type_id;
        profit_percentage = a.profit_percentage;
        profit_per_isk = a.profit_per_isk;
        packet_Jita_Price = a.packet_Jita_Price;
        mj_price = a.mj_price;
        packet_volume = a.packet_volume;
    }
    
    void print(){
        std::cout << "type_id: " << type_id;
        std::cout << " profit_percentage: " << profit_percentage;
        std::cout << " profit_per_isk: " << profit_per_isk;
        std::cout << " packet_Jita_Price: " << packet_Jita_Price;
        std::cout << " packet_volume: " << packet_volume;
        std::cout << " mj_price: " << mj_price << std::endl;
    }
};