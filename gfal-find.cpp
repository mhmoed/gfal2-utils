#include <stdexcept>
#include <iostream>
#include <stack>
#include <memory>
#include <algorithm>
#include <ctime>

#include <boost/program_options.hpp>

#include <fnmatch.h>

#include "gfal2.hpp"


// Constants

const int NUM_SECONDS_IN_SIX_MONTHS = 15552000;
const int DATETIME_BUFFER_SIZE = 20;
const int SIZE_FIELD_LENGTH = 12;

const std::string OLD_FORMAT = "%h %e  %Y";
const std::string NEW_FORMAT = "%h %e %R";


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


struct shell_pattern_filter:public display_filter
{
    shell_pattern_filter(const std::string &pattern):display_filter(), _pattern(pattern)
    {
    }

    virtual bool operator()(const gfal2::directory_entry &entry) const
    {
        return fnmatch(_pattern.c_str(), entry.name.c_str(), 0);
    }

    private:

        const std::string _pattern;
};


// Formatters

struct formatter
{
    virtual std::string operator()(const std::string &path, const gfal2::directory_entry &entry) = 0;
};


struct short_formatter:public formatter
{
    virtual std::string operator()(const std::string &path, const gfal2::directory_entry &entry)
    {
        return path;
    }
};


struct long_formatter:public formatter
{
    virtual std::string operator()(const std::string &path, const gfal2::directory_entry &entry)
    {
        std::string line;

        // Permissions

        if (gfal2::is_directory(entry))
            line = "d";
        else if (gfal2::is_symlink(entry))
            line = "l";
        else
            line = "-";


        auto mode = entry.status.st_mode;

        // User

        line += mode & S_IRUSR ? "r" : "-";
        line += mode & S_IWUSR ? "w" : "-";
        line += mode & S_IXUSR ? "x" : "-";

        // Group

        line += mode & S_IRGRP ? "r": "-";
        line += mode & S_IWGRP ? "w": "-";
        line += mode & S_IXGRP ? "x": "-";

        // World

        line += mode & S_IROTH ? "r": "-";
        line += mode & S_IWOTH ? "w": "-";
        line += mode & S_IXOTH ? "x": "-";

        line += " ";

        // Number of symbolic links

        line += boost::lexical_cast<std::string>(entry.status.st_nlink);
        line += "\t";

        // Group ID

        line += boost::lexical_cast<std::string>(entry.status.st_gid);
        line += "\t";

        // User ID

        line += boost::lexical_cast<std::string>(entry.status.st_uid);
        line += "\t";

        // Modification time

        auto current_time = std::time(nullptr);
        auto &mod_time = entry.status.st_mtime;

        char buffer[DATETIME_BUFFER_SIZE];

        const std::string &datetime_format = (current_time - mod_time) > NUM_SECONDS_IN_SIX_MONTHS ? OLD_FORMAT : NEW_FORMAT;
        
        strftime(buffer, 20, datetime_format.c_str(), localtime(&mod_time));
        
        line += buffer;
        line += "\t";

        // Size

        const std::string size_field = gfal2::is_directory(entry) ? "0" : boost::lexical_cast<std::string>(entry.status.st_size);
        line += std::string(std::max<long>(SIZE_FIELD_LENGTH - size_field.length(), 0), ' ');  // Pad with leading empty spaces to align
        line += size_field;
        line += "\t";

        // Path

        line += path;

        return std::string(line);
    }
};


void find(const std::string &root,
          const std::vector<std::unique_ptr<display_filter>> &display_filters,
          const std::unique_ptr<formatter> &formatter)
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
                cout << (*formatter)(path, entry) << endl;

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
        ("name", po::value<string>(), "match basename against shell pattern")
        ("long,l", "show entries in long listing format")
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

        // Entry type filter

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

        // Name filter

        if (vm.count("name"))
        {
            const auto &pattern = vm["name"].as<string>();
            display_filters.push_back(move(unique_ptr<shell_pattern_filter>(new shell_pattern_filter(pattern))));
        }

        const unique_ptr<formatter> format(move(unique_ptr<formatter>(vm.count("long") ? static_cast<formatter*>(new long_formatter) : static_cast<formatter*>(new short_formatter))));

        find(vm["url"].as<string>(), display_filters, format);
    }
    catch (const std::exception &exception)
    {
        cerr << "Terminating with exception:" << endl << exception.what() << endl;
    }

    return EXIT_SUCCESS;
}
