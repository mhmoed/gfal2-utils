#include "gfal2.hpp"

#include <boost/lexical_cast.hpp>


namespace gfal2
{
    using namespace detail;


    bool is_file(const struct directory_entry &entry)
    {
        return S_ISREG(entry.status.st_mode);
    }


    bool is_directory(const struct directory_entry &entry)
    {
        return S_ISDIR(entry.status.st_mode);
    }


    bool is_symlink(const struct directory_entry &entry)
    {
        return S_ISLNK(entry.status.st_mode);
    }


    context::context():boost::noncopyable()
    {
        auto function = std::bind(gfal2_context_new, std::placeholders::_1);
        ctx = checked<gfal2_context_t>(function, "error creating new context");
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
        directory_entries entries;
        directory directory(ctx, url);
        
        GError *error = NULL;
        while (struct dirent *ent = gfal2_readdir(ctx.handle(), &directory.handle(), &error))
        {
            verify_error(std::string("error reading directory ") + url, error);

            directory_entry entry;
            entry.name = ent -> d_name;
            entry.status = stat(ctx, url + DIRECTORY_SEPARATOR + entry.name);

            entries.push_back(entry);
        }        

        return entries;
    }


    // Implementation. Don't use directly.

    namespace detail
    {
        directory::directory(context &_ctx, const std::string &url):boost::noncopyable(), ctx(_ctx)
        {
            auto function = std::bind(gfal2_opendir, ctx.handle(), url.c_str(), std::placeholders::_1);
            dir_handle = checked<DIR*>(function, "error opening directory");
        }


        directory::~directory()
        {
            auto function = std::bind(gfal2_closedir, ctx.handle(), dir_handle, std::placeholders::_1);
            checked<void>(function, "error closing directory");
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


        template<> void checked<void>(std::function<void(GError**)> function, const std::string &message)
        {
            GError *error = NULL;
            function(&error);
            verify_error(message, error);
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
