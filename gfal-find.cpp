#include <stdexcept>
#include <iostream>
#include <stack>
#include <memory>
#include <algorithm>

#include <boost/program_options.hpp>

#include "gfal2.hpp"


// Display filters

struct display_filter
{
    virtual bool operator()(const gfal2::directory_entry &entry) const = 0;
};


struct file_filter:public display_filter
{
    virtual bool operator()(const gfal2::directory_entry &entry) const
    {
        return gfal2::is_file(entry);
    }
};


struct directory_filter:public display_filter
{
    virtual bool operator()(const gfal2::directory_entry &entry) const
    {
        return gfal2::is_directory(entry);
    }
};


void find(const std::string &root, const std::vector<std::unique_ptr<display_filter>> &display_filters)
{
    using namespace std;
    using gfal2::DIRECTORY_SEPARATOR;

    stack<string> remaining({""});

    // List directories while there are still directories remaining

    gfal2::context context;

    while (not remaining.empty())
    {
        gfal2::directory_entries entries;
    
        const string relative_path = remaining.top();
        remaining.pop();

        // Process entries: print all, push directories to stack relative to current path

        for (const auto &entry : gfal2::list_directory(context, root + DIRECTORY_SEPARATOR + relative_path))
        {
            const auto path = relative_path.empty() ? entry.name : relative_path + DIRECTORY_SEPARATOR + entry.name;

            if (none_of(display_filters.cbegin(), display_filters.cend(), [&entry](auto &filter){ return (*filter)(entry); }))
                cout << path << endl;

            // Push to stack for further listing if directory

            if (gfal2::is_directory(entry))
                remaining.push(path);
        }
    }
}


int main(const int argc, char **argv)
{
	using namespace std;
    namespace po = boost::program_options;

    // Parse arguments

    po::options_description description("Search files in a directory tree");
    description.add_options()
        ("help,h", "show help message")
        ("type", po::value<string>(), "type of file (f or d)")
        ("url", po::value<string>(), "URL to search from");
    
    po::positional_options_description p;
    p.add("url", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        cerr << description << endl;
        return EXIT_SUCCESS;
    }

    if (not vm.count("url"))
    {
        cerr << description << endl;
        cerr << "Missing URL." << endl;
        return EXIT_SUCCESS;
    }

    // Run find

    try
    {
        // Set up display filters

        vector<unique_ptr<display_filter>> display_filters;

        if (vm.count("type"))
        {
            const auto &type = vm["type"].as<string>();
            if (type == "f")
                display_filters.push_back(move(unique_ptr<directory_filter>(new directory_filter)));
            else if (type == "d")
                display_filters.push_back(move(unique_ptr<file_filter>(new file_filter)));
            else
                throw invalid_argument(string("unknown entry type: ") + type);
        }

        find(vm["url"].as<string>(), display_filters);
    }
    catch (const std::exception &exception)
    {
        cerr << "Terminating with exception:" << endl << exception.what() << endl;
    }

    return EXIT_SUCCESS;
}
