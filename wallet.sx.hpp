#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using namespace std;

#include <optional>

class [[eosio::contract("wallet.sx")]] walletSx : public contract {

public:
    using contract::contract;

    /**
     * ## TABLE `balances`
     *
     * **scope:** `contract`
     *
     * - `{name} account` - account name
     * - `{map<symbol_code, asset>} balances` - balances
     *
     * ### Example - cleos
     *
     * ```bash
     * $ cleos get table wallet.sx eosio.token balances --lower myaccount --upper myaccount
     * ```
     *
     * ### Example - json
     *
     * ```json
     * {
     *     "account": "myaccount",
     *     "balances": [
     *         { "key": "EOS", "value": "1.0000 EOS" }
     *     ]
     * }
     * ```
     */
    struct [[eosio::table("balances")]] balances_row {
        name                            account;
        map<symbol_code, asset>         balances;

        uint64_t primary_key() const { return account.value; }
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
     * walletSx::withdraw_action withdraw( "wallet.sx"_n, { account, "active"_n });
     * withdraw.send( account, contract, quantity );
     * ```
     */
    [[eosio::action]]
    void withdraw( const name account, const name contract, const asset quantity );

    /**
     * ## ACTION `transfer`
     *
     * Transfer assets to an account
     *
     * - **authority**: `from` or `get_self()`
     *
     * ### params
     *
     * - `{name} from` - authorized sender account
     * - `{name} to` - receiver account
     * - `{name} contract` - token contract (ex: "eosio.token")
     * - `{asset} quantity` - transfer quantity amount (ex: "1.0000 EOS")
     * - `{string} [memo=""]` - memo used on transfer
     *
     * ### Example - cleos
     *
     * ```bash
     * cleos push action wallet.sx transfer '["myaccount", "toaccount", "eosio.token", "1.0000 EOS", "memo"]' -p myaccount
     * ```
     *
     * ### Example - smart contract
     *
     * ```c++
     * // input variables
     * const name from = "myaccount"_n;
     * const name to = "toaccount"_n;
     * const name contract = "eosio.token"_n;
     * const asset quantity = asset{ 10000, symbol{"EOS", 4} };
     * const string memo = "my memo";
     *
     * // send transaction
     * walletSx::transfer_action transfer( "wallet.sx"_n, { from, "active"_n });
     * transfer.send( from, to, contract, quantity, memo );
     * ```
     */
    [[eosio::action]]
    void transfer( const name from, const name to, const name contract, const asset quantity, const optional<string> memo );

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
     * const asset balance = walletSx::get_balance( "wallet.sx"_n, account, contract, symcode );
     * //=> "1.0000 EOS"
     * ```
     */
    static asset get_balance( const name code, const name account, const name contract, const symbol_code symcode )
    {
        walletSx::balances _balances( code, contract.value );
        const auto balances = _balances.get( account.value, "no account balance found" ).balances;
        const asset balance = balances.at( symcode );
        check( balance.symbol.code().raw(), "no balance found" );

        return balance;
    }

    // action wrappers
    using withdraw_action = eosio::action_wrapper<"withdraw"_n, &walletSx::withdraw>;
    using transfer_action = eosio::action_wrapper<"transfer"_n, &walletSx::transfer>;
    using deposit_action = eosio::action_wrapper<"deposit"_n, &walletSx::deposit>;

private:
    void add_balance( const name account, const name contract, const asset quantity, const name ram_payer );
    void sub_balance( const name account, const name contract, const asset quantity );
    void check_open( const name account, const name contract, const symbol_code symcode );
};
