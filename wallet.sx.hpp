#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using namespace std;

#include <optional>

namespace sx {

class [[eosio::contract("wallet.sx")]] wallet : public contract {

public:
    using contract::contract;

    /**
     * ## TABLE `balances`
     *
     * **scope:** `account`
     *
     * - `{name} contract` - token contract
     * - `{map<symbol_code, asset>} balances` - balances
     *
     * ### Example - cleos
     *
     * ```bash
     * $ cleos get table wallet.sx myaccount balances
     * ```
     *
     * ### Example - json
     *
     * ```json
     * {
     *     "contract": "eosio.token",
     *     "balances": [
     *         { "key": "EOS", "value": "1.0000 EOS" }
     *     ]
     * }
     * ```
     */
    struct [[eosio::table("balances")]] balances_row {
        name                            contract;
        map<symbol_code, asset>         balances;

        uint64_t primary_key() const { return contract.value; }
    };
    typedef eosio::multi_index< "balances"_n, balances_row > balances;

    /**
     * ## ACTION `withdraw`
     *
     * Request to withdraw quantity
     *
     * - **authority**: `account` or `get_self()`
     *
     * ### params
     *
     * - `{name} account` - account of wallet assets
     * - `{name} contract` - token contract (ex: "eosio.token")
     * - `{asset} quantity` - withdraw quantity amount (ex: "1.0000 EOS")
     *
     * ### Example - cleos
     *
     * ```bash
     * cleos push action wallet.sx withdraw '["myaccount", "eosio.token", "1.0000 EOS"]' -p myaccount
     * ```
     *
     * ### Example - smart contract
     *
     * ```c++
     * // input variables
     * const name account = "myaccount"_n;
     * const name contract = "eosio.token"_n;
     * const asset quantity = asset{ 10000, symbol{"EOS", 4} };
     *
     * // send transaction
     * sx::wallet::withdraw_action withdraw( "wallet.sx"_n, { account, "active"_n });
     * withdraw.send( account, contract, quantity );
     * ```
     */
    [[eosio::action]]
    void withdraw( const name account, const name contract, const asset quantity );

    /**
     * ## ACTION `open`
     *
     * Open contract & symbol balance for account
     *
     * - **authority**: `ram_payer`
     *
     * ### params
     *
     * - `{name} account` - account to open balance
     * - `{name} contract` - token contract (ex: "eosio.token")
     * - `{symbol_code} symcode` - symcode code (ex: "EOS")
     * - `{name} ram_payer` - authorized account to pay for RAM
     *
     * ### Example - cleos
     *
     * ```bash
     * cleos push action wallet.sx open '["myaccount", "eosio.token", "EOS", "myaccount"]' -p myaccount
     * ```
     *
     * ### Example - smart contract
     *
     * ```c++
     * const name account = "myaccount"_n;
     * const name contract = "eosio.token"_n;
     * const symbol_code symcode = symbol_code{"EOS"};
     * const name ram_payer = "myaccount";
     *
     * sx::wallet::open_action open( "wallet.sx"_n, { ram_payer, "active"_n });
     * open.send( account, contract, symcode, ram_payer );
     * ```
     */
    [[eosio::action]]
    void open( const name account, const name contract, const symbol_code symcode, const name ram_payer );

    /**
     * ## ACTION `close`
     *
     * Close contract & symbol balance for account
     *
     * - **authority**: `account`
     *
     * ### params
     *
     * - `{name} account` - account to close balance
     * - `{name} contract` - token contract (ex: "eosio.token")
     * - `{symbol_code} symcode` - symcode code (ex: "EOS")
     *
     * ### Example - cleos
     *
     * ```bash
     * cleos push action wallet.sx close '["myaccount", "eosio.token", "EOS"]' -p myaccount
     * ```
     *
     * ### Example - smart contract
     *
     * ```c++
     * const name account = "myaccount"_n;
     * const name contract = "eosio.token"_n;
     * const symbol_code symcode = symbol_code{"EOS"};
     *
     * sx::wallet::close_action close( "wallet.sx"_n, { account, "active"_n });
     * close.send( account, contract, symcode );
     * ```
     */
    [[eosio::action]]
    void close( const name account, const name contract, const symbol_code symcode );

    [[eosio::action]]
    void deposit( const name account, const name contract, const asset quantity );

    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const string memo );

    /**
     * ## STATIC `get_balance`
     *
     * Get balance of account
     *
     * ### params
     *
     * - `{name} code` - SX wallet contract account
     * - `{name} account` - account name
     * - `{name} contract` - token contract
     * - `{symbol_code} symcode` - symbol code
     *
     * ### example
     *
     * ```c++
     * const name account = "myaccount"_n;
     * const name contract = "eosio.token"_n;
     * const symbol_code symcode = symbol_code{"EOS"};
     *
     * const asset balance = sx::wallet::get_balance( "wallet.sx"_n, account, contract, symcode );
     * //=> "1.0000 EOS"
     * ```
     */
    static asset get_balance( const name code, const name account, const name contract, const symbol_code symcode )
    {
        sx::wallet::balances _balances( code, account.value );
        const auto balances = _balances.get( contract.value, "no account balance found" ).balances;
        const asset balance = balances.at( symcode );
        check( balance.symbol.code().raw(), "no balance found" );

        return balance;
    }

    // action wrappers
    using withdraw_action = eosio::action_wrapper<"withdraw"_n, &sx::wallet::withdraw>;
    using deposit_action = eosio::action_wrapper<"deposit"_n, &sx::wallet::deposit>;
    using open_action = eosio::action_wrapper<"open"_n, &sx::wallet::open>;
    using close_action = eosio::action_wrapper<"close"_n, &sx::wallet::close>;

private:
    void add_balance( const name account, const name contract, const asset quantity, const name ram_payer );
    void sub_balance( const name account, const name contract, const asset quantity );
    void check_open( const name account, const name contract, const symbol_code symcode );
    void check_open_internal( const name account, const name contract, const symbol_code symcode );
    void require_auth_or_self( const name account );
};

}