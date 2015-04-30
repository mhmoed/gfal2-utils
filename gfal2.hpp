#ifndef INC_GFAL2_HPP
#define INC_GFAL2_HPP

#include <vector>
#include <string>
#include <sys/stat.h>

#include <gfal_api.h>


namespace gfal2
{
    struct directory_entry
    {
        std::string name;
        struct stat status;
    };
    
    
    typedef std::vector<directory_entry> directory_entries;
    
    
    bool is_file(const struct stat &s);
    
    
    bool is_directory(const struct stat &s);
    
    
    bool is_symlink(const struct stat &s);
    
    
    class context
    {
        public:
            
            context();
            
                
            ~context();
        
            
            directory_entries list_directory(const std::string &url);
            
            
            struct stat stat(const std::string &url);
        
        private:
            
            gfal2_context_t ctx;
            
            
            void verify_error(const std::string &message, const GError* const error);
            
            
            DIR* open_directory(const std::string &url);
            
            
            void close_directory(DIR* const handle);
        };
}

#endif
