#include <stdexcept>
#include <iostream>
#include <stack>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "gfal2.hpp"


void find(const boost::filesystem::path &root, bool report_files, bool report_directories)
{
    using namespace std;
    using namespace boost::filesystem;

    stack<path> remaining;
    remaining.push(path(""));

    // List directories while there are still directories remaining

    gfal2::context context;

    while (not remaining.empty())
    {
        gfal2::directory_entries entries;
    
        const auto relative_path = remaining.top();
        remaining.pop();

        // Process entries: print all, push directories to stack relative to current path

        const auto p = root / relative_path;

        for (const auto &entry : gfal2::list_directory(context, p.string()))
        {
            if ((gfal2::is_file(entry) and report_files) or (gfal2::is_directory(entry) and report_directories))
                cout << (relative_path / entry.name).string() << endl;

            // Push to stack for further listing if directory

            if (gfal2::is_directory(entry))
                remaining.push(relative_path / entry.name);
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
        const boost::filesystem::path root(vm["url"].as<string>());
        if (not vm.count("type"))
            find(root, true, true);
        else
        {
            const auto &type = vm["type"].as<string>();
            if (type == "f")
                find(root, true, false);
            else if (type == "d")
                find(root, false, true);
            else
                throw invalid_argument(string("unknown type: ") + type);
        }
    }
    catch (const std::exception &exception)
    {
        cerr << "Terminating with exception:" << endl << exception.what() << endl;
    }

    return EXIT_SUCCESS;
}
