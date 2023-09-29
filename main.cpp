#include <thread>
#include <mutex>
#include <iostream>
#include <fstream>
#include <chrono>
#include <map>
#include <algorithm>
#include <iomanip>

#include "mtq.h"

// Vocabulary aka std::map<std::string, std::size_t>
using vocabulary = std::map<std::string, std::size_t>;

// Number of top frequent words to print
constexpr static size_t top_words_count = 10;

// Toggle number of threads to create
int toggle_threads_number (int argc)
{
    // Number of logical processors
	int temp = static_cast<int>(std::thread::hardware_concurrency());

    // If number of files < number of logical processors
    // Number of threads = number of files
    return (argc - 1 < temp) ? argc - 1 : temp;
}

// Make std::string lowercase
std::string tolower(std::string str) {
    for(int i = 0; i < str.length(); ++i)
    {
        str[i] = static_cast<char>(std::tolower(str[i]));
    }
    return str;
};

// Count words in stream
void count_words(std::istream& input, vocabulary& voc)
{
    // For each word (space is separator)
    std::for_each(
        std::istream_iterator<std::string>(input),
        std::istream_iterator<std::string>(),

        // Lambda function for counting words
        [&voc](const std::string &s)
        {
            // Increment current (in lower register) word's counter
            ++voc[tolower(s)];
        }
    );  
}

// Print
void print_frequent_words(std::ostream& output, vocabulary& voc, const size_t count)
{
    // New vector filled with constant iterators to vocabulary aka std::map
    std::vector<vocabulary::const_iterator> words;

    // Push back all words iterators
    words.reserve(voc.size());

    for (auto it = std::cbegin(voc); it != std::cend(voc); ++it) {
        words.push_back(it);
    }

    // TODO :
    // Implement checking for std::begin(words) + count (possibly could be out of bounds)
    // For very little amount of words

    // Partial sort words by number
    std::partial_sort(
        std::begin(words), std::begin(words) + count, std::end(words),

        [](auto lhs, auto &rhs)
        {
            return lhs->second > rhs->second;
        }
    );

    // Print sorted words with numbers
    std::for_each(
        std::begin(words), std::begin(words) + count,

        [&output](const vocabulary::const_iterator &pair)
        {
            output << std::setw(4) << pair->second << " " << pair->first << '\n';
        }
    );
}

int main(int argc, const char* argv[])
{
    // Check arguments list
    // If empty then show message and return EXIT_FAILURE
    if (argc < 2) {
        std::cout << "Usage: topk_words [FILES...]" << std::endl;
        return EXIT_FAILURE;
    }

    // Get start time_point with std::chrono
    auto start = std::chrono::high_resolution_clock::now();

    // Number of threads
    const int threads_number = toggle_threads_number(argc);

    // MT-safe queue of filenames
    mtq<std::string> books_to_counter;

    // Vocabularies
    std::vector<vocabulary> vocabularies;
    vocabularies.reserve(threads_number);

    // Threads
    std::vector<std::thread> threads;
    threads.reserve(threads_number);

    // Fill queue
    for(int i = 1; i < argc; ++i)
    {
        books_to_counter.push(argv[i]);
    }
    books_to_counter.stop();

    // Starting word counting threads
    for(int i = 0; i < threads_number; ++i)
    {
        // Add vocabulary for current thread
        vocabularies.emplace_back();

        // Start new thread
        threads.emplace_back(std::thread(
            [&, i]
            {
                // Filename var
                std::string file;

                // Stream var
                std::ifstream ifs;

                // Task for thread to count words until queue is empty
                while(books_to_counter.pop(file))
                {
                    // Open current file
                    ifs.open(file);

                    // Check weather file is open
                    if(!ifs.is_open())
                    {
                        std::cout << "Error! Cannot open " << file << std::endl;
                        throw;
                    }

                    // Count words in current file and update vocabulary
                    count_words(ifs, vocabularies[i]);

                    // Close file
                    ifs.close();
                }
            }
        ));
    }

    // Join all threads
    for(int i = 0; i < threads_number; ++i)
    {
        threads[i].join();
    }

    // TODO :
    // Map merging is very slow, have to find alterantive way

    // Merge all vocabularies into one
    for(int i = 1; i < vocabularies.size(); ++i)
    {
        for(auto& j : vocabularies[i])
        {
            if (auto search = vocabularies[0].find(j.first); search != vocabularies[0].end())
            {
                search->second += j.second;
            }
            else
            {
                vocabularies[0].insert(j);
            }
        }
    }

    // Print results
    print_frequent_words(std::cout, vocabularies[0], top_words_count);

    // Get end time_point with std::chrono
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate spent time with std::chrono
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Print spent time in microseconds
    std::cout << "Elapsed time is " << elapsed_ms.count() << " us\n";

    return EXIT_SUCCESS;
}