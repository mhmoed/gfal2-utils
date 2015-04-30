#include "gfal2.hpp"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>


namespace gfal2
{
    // TODO: overload with path

    bool is_file(const struct stat &s)
    {
        return S_ISREG(s.st_mode);
    }

    
    // TODO: overload with path
    
    bool is_directory(const struct stat &s)
    {
        return S_ISDIR(s.st_mode);
    }


    // TODO: overload with path
    
    bool is_symlink(const struct stat &s)
    {
        return S_ISLNK(s.st_mode);
    }


    context::context()
    {
        GError *error = NULL;
        ctx = gfal2_context_new(&error);
        verify_error("verify_error creating new context", error);
    }


    context::~context()
    {
        gfal2_context_free(ctx);
    }


    directory_entries context::list_directory(const std::string &url)
    {
        using namespace boost::filesystem;

        directory_entries entries;

        DIR* const handle = open_directory(url);
        GError *error = NULL;

        struct dirent *ent = gfal2_readdir(ctx, handle, &error);
        while (ent)
        {
            directory_entry entry;
            entry.name = ent -> d_name;
            entry.status = stat((path(url) / entry.name).string());
            entries.push_back(entry);

            ent = gfal2_readdir(ctx, handle, &error);
        }

        close_directory(handle);
        return entries;
    }


    struct stat context::stat(const std::string &url)
    {
        struct stat fstat;
        GError *error = NULL;
        gfal2_stat(ctx, url.c_str(), &fstat, &error);
        verify_error(std::string("error getting stats for URL ") + url, error);
        return fstat;
    }


    void context::verify_error(const std::string &message, const GError* const error)
    {
        if (error)
            throw std::runtime_error(message + " (code = " + boost::lexical_cast<std::string>(error -> code) + ", message = " + error -> message + ")");
    }

    
    DIR* context::open_directory(const std::string &url)
    {
        GError *error = NULL;
        DIR* handle = gfal2_opendir(ctx, url.c_str(), &error);
        verify_error(std::string("error opening directory ") + url, error);
        return handle;
    }


    void context::close_directory(DIR* const handle)
    {
        GError *error = NULL;
        gfal2_closedir(ctx, handle, &error);
        verify_error("error closing directory", error);
    }
}
