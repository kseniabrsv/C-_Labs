#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <regex>
#include <queue>
#include <vector>
#include <chrono>
#include <set>
#include <atomic>

std::mutex locker;
std::mutex fin_locker;
std::atomic<int> count_of_working(0);
std::atomic<int> links_count(0);

void working_with_files(std::queue<std::string>& links, std::set<std::string>& names_of_files)
{
    std::ifstream input;
    std::string str, str1, str_from_file;
    std::regex mask("<a href=\"file://.*?>");
    std::regex mask2("[0-9]*.html");
    std::smatch matched_links, matched_links2;

    while (true)
    {
        while (links.empty())
        {
            if (count_of_working == 0)
                return;
        }
        std::unique_lock<std::mutex> lock1(locker);
        if (links.empty())
        {
            break;
        }
        count_of_working++;
        str = links.front();
        links.pop();
        lock1.unlock();
        input.open("test_data/" + str);
        links_count++;
        while(input)
        {
            std::getline(input, str_from_file);
            while (std::regex_search(str_from_file, matched_links, mask))
            {
                str1 = matched_links[0];
                str_from_file = matched_links.suffix().str();
                bool search = std::regex_search(str1, matched_links2, mask2);
                str1 = matched_links2.str();
                std::unique_lock<std::mutex> lock2(fin_locker);
                if (names_of_files.find(str1) == names_of_files.end())
                {
                    links.push(str1);
                    names_of_files.insert(str1);
                }
            }
        }
        input.close();
        count_of_working--;
    }
} 

int main()
{
    auto begin = std::chrono::steady_clock::now();
    std::ifstream input;
    std::ofstream output;
    std::string first_link, str;
    int x;
    std::queue<std::string> links;
    std::vector<std::thread> threads;
    std::set<std::string> names_of_files;

    input.open("input.txt");
    input >> first_link >> x;
    input.close();

    first_link = first_link.substr(7);
    links.push(first_link.substr(10));
    names_of_files.insert(first_link.substr(10));
    for (int i = 0; i < x; i++)
    {
        threads.emplace_back(working_with_files, std::ref(links), std::ref(names_of_files));
    }
    for (int i = 0; i < x; i++)
        threads[i].join();
    auto end = std::chrono::steady_clock::now();
    std::cout << links_count << " " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    return 0;
}

//Самое эффективное время выполнения программы при 13 работающих потоках ~3,012 сек
