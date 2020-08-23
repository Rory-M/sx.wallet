#include <eosio.token/eosio.token.hpp>
#include "wallet.sx.hpp"

[[eosio::on_notify("*::transfer")]]
void walletSx::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    require_auth( from );

    // ignore no-incoming transfers
    if ( to != get_self() ) return;

    // allows to deposit to another account
    // default (no memo) deposits goes to sender (from)
    const name account = memo.length() ? name{memo} : from;
    const name contract = get_first_receiver();

    // for alternate account deposits, account must already have open balance
    if ( account != from ) check_open_internal( account, contract, quantity.symbol.code() );

    // update balance
    add_balance( account, contract, quantity, get_self() );

    // deposit log (notification purposes only)
    walletSx::deposit_action deposit( get_self(), { get_self(), "active"_n });
    deposit.send( account, contract, quantity );
}

[[eosio::action]]
void walletSx::withdraw( const name account, const name contract, const asset quantity )
{
    walletSx::transfer( account, account, contract, quantity, "withdraw" );
}

[[eosio::action]]
void walletSx::transfer( const name from, const name to, const name contract, const asset quantity, const optional<string> memo )
{
    require_auth_or_self( from );

    // deduct balance from internal balances
    sub_balance( from, contract, quantity );
    check_open( to, contract, quantity.symbol.code() );

    // return tokens to account
    token::transfer_action transfer( contract, { get_self(), "active"_n });
    transfer.send( get_self(), to, quantity, *memo );
}

[[eosio::action]]
void walletSx::deposit( const name account, const name contract, const asset quantity )
{
    require_auth( get_self() );
}

[[eosio::action]]
void walletSx::open( const name account, const name contract, const symbol_code symcode, const name ram_payer )
{
    require_auth( ram_payer );

    check( is_account( account ), "account does not exist" );

    // retrieve token precision directly from token contract
    const asset supply = token::get_supply( contract, symcode );
    walletSx::balances _balances( get_self(), account.value );
    const auto itr = _balances.find( contract.value );

    // create new balance entry
    if ( itr == _balances.end() ) {
        _balances.emplace( ram_payer, [&]( auto& row ) {
            row.contract = contract;
            row.balances[ symcode ] = asset{0, supply.symbol};
        });
    } else {
        // balance does not exists
        if ( itr->balances.find( symcode ) == itr->balances.end() ) {
            _balances.modify( itr, same_payer, [&]( auto& row ) {
                row.balances[ symcode ] = asset{0, supply.symbol};
            });
        }
    }
}

[[eosio::action]]
void walletSx::close( const name account, const name contract, const symbol_code symcode )
{
    require_auth_or_self( account );

    walletSx::balances _balances( get_self(), account.value );
    const auto & itr = _balances.get( contract.value, "no account to close" );

    // account exists but no balances exists
    if ( itr.balances.size() == 0 ) return _balances.erase( itr );

    // symbol exists in balances
    if ( itr.balances.find( symcode ) != itr.balances.end() ) {
        const asset balance = itr.balances.at( symcode );
        check( balance.amount == 0, symcode.to_string() + " balance must equal to 0");

        // only single balance exists, delete entire account table row
        if ( itr.balances.size() == 1 ) return _balances.erase( itr );

        // delete single balance
        _balances.modify( itr, same_payer, [&]( auto& row ) {
            row.balances.erase( symcode );
        });
    }
}

void walletSx::sub_balance( const name account, const name contract, const asset quantity )
{
    walletSx::balances _balances( get_self(), account.value );
    const symbol_code symcode = quantity.symbol.code();
    const auto & itr = _balances.get( contract.value, "no account balance found" );
    const asset balance = itr.balances.at( symcode );

    // validation
    check( balance.symbol.code().raw(), "no balance found" );
    check( balance.amount >= quantity.amount, "overdrawn balance" );

    // reduce balance
    _balances.modify( itr, same_payer, [&]( auto& row ) {
        row.balances[ symcode ] -= quantity;
    });
}

void walletSx::add_balance( const name account, const name contract, const asset quantity, const name ram_payer )
{
    walletSx::balances _balances( get_self(), account.value );
    const symbol_code symcode = quantity.symbol.code();
    const auto itr = _balances.find( contract.value );

    // create new balance entry
    if ( itr == _balances.end() ) {
        _balances.emplace( ram_payer, [&]( auto& row ) {
            row.contract = contract;
            row.balances[ symcode ] = quantity;
        });
    // modify balance entry
    } else {
        _balances.modify( itr, same_payer, [&]( auto& row ) {
            if ( !row.balances.at( symcode ).symbol.code() ) row.balances[ symcode ] = quantity;
            else row.balances[ symcode ] += quantity;
        });
    }
}

void walletSx::check_open( const name account, const name contract, const symbol_code symcode )
{
    check( is_account( account ), account.to_string() + " account does not exist");

    token::accounts _accounts( contract, account.value );
    auto itr = _accounts.find( symcode.raw() );
    check( itr != _accounts.end(), account.to_string() + " account must have " + symcode.to_string() + " `open` balance in " + contract.to_string());
}

void walletSx::check_open_internal( const name account, const name contract, const symbol_code symcode )
{
    check( is_account( account ), account.to_string() + " account does not exist");

    walletSx::balances _balances( get_self(), account.value );
    const string message = account.to_string() + " account must have " + symcode.to_string() + " `open` balance in " + get_self().to_string();
    const auto itr = _balances.find( contract.value );
    check( itr != _balances.end(), message );
    const asset balance = itr->balances.at( symcode );
    check( balance.symbol.code().raw(), message );
}

void walletSx::require_auth_or_self( const name account )
{
    if ( has_auth( get_self() ) ) return;
    require_auth( account );
}
