#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>


using namespace eosio;
using namespace std;

class [[eosio::contract]] wallet : public contract {
public:
    using contract::contract;

    [[eosio::action]]
    void id( const string user ) {
        print( to_string( string_to_uint64( user ) ) + "\n" );
    }

    [[eosio::action]]
    void deweb( const string user ) {
        print( to_string( string_to_id( user ) ) );
    }

    uint64_t string_to_uint64( const string str ) {
        return checksum256_to_uint64( eosio::sha256( str.c_str(), str.size() ) );
    }

    uint64_t string_to_id(std::string s) {
        const char *str = s.c_str();
        uint64_t hash = 5381;
        int c;
        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c;
        }
        return hash;
    }

    uint64_t checksum256_to_uint64( const checksum256 hash )
    {
        auto arr = hash.extract_as_byte_array();

    	uint64_t serial = ((uint64_t)arr.data()[0] << 56) +
            ((uint64_t)arr.data()[4] << 48) +
            ((uint64_t)arr.data()[8] << 40) +
            ((uint64_t)arr.data()[12] << 32) +
            ((uint64_t)arr.data()[16] << 24) +
            ((uint64_t)arr.data()[20] << 16) +
            ((uint64_t)arr.data()[24] << 8) +
            (uint64_t)arr.data()[28];

        return serial;
    }
};