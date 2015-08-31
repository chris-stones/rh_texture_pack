
#include<ctype.h>
#include<string>
#include<string.h>
#include<assert.h>
#include<algorithm>

/****
 * not used for encryption.
 * If there is a collosion, we will know about it at build time.
 * All I care about here is speed and bug-free-ness, and this is too small to be buggy or overly slow!
 * 
 * This code was written by this guy -> http://research.microsoft.com/en-us/people/palarson/
 * I found it on Stack-overflow.
 * 
 * THANKS PAUL LARSON.
 **/
unsigned int hash( const char* s, unsigned int seed)
{
    unsigned hash = seed;
    while (*s)
    {
        hash = hash * 101  +  *s++;
    }
    return hash;
}

int tohashable(int c) {

    switch(c) {
    default:
        return tolower(c);
    case '/':
    case '\\':
        return '.';
    }
}

std::string get_game_resource_name(std::string hashString, const std::string &append, const char * resource_root) {

	// remove file extension - game does not care is origonal resource was .png/.jpeg/.dds/.bmp/.whatever
    int dot = hashString.find_last_of(".");
    assert(dot != std::string::npos );
    hashString = hashString.substr(0, dot);

    std::transform(hashString.begin(), hashString.end(), hashString.begin(), tohashable);

    // append an optional string. We need to do this when alpha components need to be stored separately to colour components
	// due to limitations in the final pixel format. For example, RGB / DXT1 / ETC1.
	hashString += append;

    hashString = std::string( hashString.begin()+strlen(resource_root), hashString.end() );
    if(hashString[0] == '.')
        hashString = std::string( hashString.begin()+1, hashString.end() );
    return hashString;
}
