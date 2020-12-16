#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <regex>
#include <queue>
#include <vector>
#include <chrono>
#include <set>

std::mutex locker;
std::mutex fin_locker;

void working_with_files(std::queue<std::string>& links, int& links_count, std::set<std::string>& names_of_files)
{
    std::ifstream input;
    std::string str, str1, new_link;
    std::regex mask("<a href=\"file://.*?>");
    std::regex mask2("[0-9]*.html");
    std::smatch matched_links, matched_links2;

    while (!links.empty())
    {
        locker.lock();
        if (links.empty())
        {
            locker.unlock();
            break;
        }
        str = links.front();
        links.pop();
        links_count++;
        locker.unlock();
        while (std::regex_search(str, matched_links, mask))
        {
            str1 = matched_links[0];
            str = matched_links.suffix().str();
            bool search = std::regex_search(str1, matched_links2, mask2);
            str1 = matched_links2.str();
            if (names_of_files.find(str1) == names_of_files.end())
            {
                fin_locker.lock();
                input.open("test_data/" + str1);
                while(input)
                {
                    std::getline(input, new_link);
                }
                input.close();
                fin_locker.unlock();
                links.push(new_link);
                names_of_files.insert(str1);
            }
        }
    }
} 

int main()
{
    auto begin = std::chrono::steady_clock::now();
    std::ifstream input;
    std::ofstream output;
    std::string first_link, str;
    int x, links_count = 0;
    std::queue<std::string> links;
    std::vector<std::thread> threads;
    std::set<std::string> names_of_files;

    input.open("input.txt");
    input >> first_link >> x;
    input.close();

    first_link = first_link.substr(7);
    input.open(first_link);
    while(input)
    {
        std::getline(input, str);
    }
    input.close();
    names_of_files.insert(first_link.substr(10));
    links.push(str);
    for (int i = 0; i < x; i++)
    {
        threads.emplace_back(working_with_files, std::ref(links), std::ref(links_count), std::ref(names_of_files));
    }
    for (int i = 0; i < x; i++)
        threads[i].join();
    auto end = std::chrono::steady_clock::now();
    std::cout << links_count << " " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    return 0;
}

//Самое эффективное время выполнения программы при 4 потоках ~13 сек