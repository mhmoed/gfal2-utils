#ifndef INC_GFAL2_HPP
#define INC_GFAL2_HPP

#include <vector>
#include <string>
#include <sys/stat.h>

#include <boost/noncopyable.hpp>

#include <gfal_api.h>


namespace gfal2
{
    struct directory_entry
    {
        std::string name;
        struct stat status;
    };
    
    
    typedef std::vector<directory_entry> directory_entries;
    
    
    bool is_file(const struct directory_entry &entry);
    
    
    bool is_directory(const struct directory_entry &entry);
    
    
    bool is_symlink(const struct directory_entry &entry);

    
    class context:private boost::noncopyable
    {
        public:
            
            context();
            
            ~context();

            gfal2_context_t &handle();       
        
        private:

            gfal2_context_t ctx;
    };


    directory_entries list_directory(context &context, const std::string &url);

 
    namespace detail
    {
        class directory:private boost::noncopyable
        {
            public:

                directory(context &_ctx, const std::string &url);

                ~directory();

                DIR &handle();

            private:

                context &ctx;

                DIR *dir_handle;
        };

        void verify_error(const std::string &message, const GError* const error);

        struct stat stat(context &ctx, const std::string &url);
    };
};

#endif
