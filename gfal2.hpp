#ifndef INC_GFAL2_HPP
#define INC_GFAL2_HPP

#include <vector>
#include <string>
#include <functional>
#include <sys/stat.h>

#include <boost/noncopyable.hpp>

#include <gfal_api.h>


namespace gfal2
{
    static const char DIRECTORY_SEPARATOR = '/';


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


        template <typename R> R checked(std::function<R(GError**)> function, const std::string &message)
        {
            GError *error = NULL;
            const R value = function(&error);
            verify_error(message, error);
            return value;
        }


        template<> void checked<void>(std::function<void(GError**)> function, const std::string &message);


        struct stat stat(context &ctx, const std::string &url);
    };
};

#endif
