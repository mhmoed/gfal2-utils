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


    context::context():boost::noncopyable()
    {
        GError *error = NULL;
        ctx = gfal2_context_new(&error);
        detail::verify_error("verify_error creating new context", error);
    }


    context::~context()
    {
        gfal2_context_free(ctx);
    }


    gfal2_context_t &context::handle()
    {
        return ctx;
    }


    directory_entries list_directory(context &ctx, const std::string &url)
    {
        using boost::filesystem::path;

        directory_entries entries;
        detail::directory directory(ctx, url);
        
        GError *error = NULL;
        while (struct dirent *ent = gfal2_readdir(ctx.handle(), &directory.handle(), &error))
        {
            detail::verify_error(std::string("error reading directory ") + url, error);

            directory_entry entry;
            entry.name = ent -> d_name;
            entry.status = detail::stat(ctx, (path(url) / entry.name).string());

            entries.push_back(entry);
        }        

        return entries;
    }


    // Implementation. Don't use directly.

    namespace detail
    {
        directory::directory(context &_ctx, const std::string &url):boost::noncopyable(), ctx(_ctx)
        {
            GError *error = NULL;
            dir_handle = gfal2_opendir(ctx.handle(), url.c_str(), &error);
            verify_error(std::string("error opening directory ") + url, error);
        }


        directory::~directory()
        {
            GError *error = NULL;
            gfal2_closedir(ctx.handle(), dir_handle, &error);
            verify_error("error closing directory", error);
        }


        DIR &directory::handle()
        {
            return *dir_handle;
        }


        void verify_error(const std::string &message, const GError* const error)
        {
            if (error)
                throw std::runtime_error(message + " (code = " + boost::lexical_cast<std::string>(error -> code) + ", message = " + error -> message + ")");
        }


        struct stat stat(context &ctx, const std::string &url)
        {
            struct stat fstat;
            GError *error = NULL;
            gfal2_stat(ctx.handle(), url.c_str(), &fstat, &error);
            verify_error(std::string("error getting stats for URL ") + url, error);
            return fstat;
        }
    };
};
