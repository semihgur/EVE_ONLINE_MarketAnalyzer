#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>

int main() {
    // Get the current date
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    auto currentDate = *std::localtime(&nowTime);
    //printing current date
    std::cout << "Current date: " << std::put_time(&currentDate, "%Y-%m-%d") << '\n';

    // Get the date string from the object
    const char* dateString = "2023-05-20";
    // Convert the date string to a std::tm object
    std::tm objDateTime = {};
    std::istringstream ss(dateString);
    ss >> std::get_time(&objDateTime, "%Y-%m-%d");

    // Calculate the difference in days
    auto currentTP = std::chrono::time_point_cast<std::chrono::hours>(std::chrono::system_clock::from_time_t(nowTime));
    auto objTP = std::chrono::time_point_cast<std::chrono::hours>(std::chrono::system_clock::from_time_t(std::mktime(&objDateTime)));
    auto diff = std::chrono::duration_cast<std::chrono::hours>(currentTP - objTP);
    int diff_days = diff.count() / 24;

    std::cout << diff_days << " days difference" << std::endl;

    // Check if the difference is within 5 days
    if (diff_days <= 5) {
        // Object date is within 5 days from today
        std::cout << "Object date " << dateString << " is within 5 days from today\n";
    } else {
        std::cout << "Object date " << dateString << " is not within 5 days from today\n";
    }
    return 0;
}