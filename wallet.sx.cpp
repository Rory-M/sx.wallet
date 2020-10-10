#include <eosio.token/eosio.token.hpp>
#include "wallet.sx.hpp"

[[eosio::on_notify("*::transfer")]]
void sx::wallet::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    require_auth( from );

    // ignore no-incoming transfers
    if ( to != get_self() ) return;

    // update balance
    const name contract = get_first_receiver();
    add_balance( from, contract, quantity, get_self() );

    // // (OPTIONAL) account must already have open balance (prevents exploiting RAM)
    // check_open_internal( account, contract, quantity.symbol.code() );

    // deposit log (notification purposes only)
    sx::wallet::deposit_action deposit( get_self(), { get_self(), "active"_n });
    deposit.send( from, contract, quantity );
}

[[eosio::action]]
void sx::wallet::withdraw( const name account, const name contract, const asset quantity )
{
    require_auth( account );

    // deduct balance from internal balances
    sub_balance( account, contract, quantity );

    // // (OPTIONAL) account must already have open balance (prevents exploiting RAM)
    // check_open( account, contract, quantity.symbol.code() );

    // return tokens to account
    token::transfer_action transfer( contract, { get_self(), "active"_n });
    transfer.send( get_self(), account, quantity, "withdraw" );
}

[[eosio::action]]
void sx::wallet::deposit( const name account, const name contract, const asset quantity )
{
    require_auth( get_self() );
}

[[eosio::action]]
void sx::wallet::open( const name account, const name contract, const symbol_code symcode, const name ram_payer )
{
    require_auth( ram_payer );

    check( is_account( account ), "account does not exist" );

    // retrieve token precision directly from token contract
    const asset supply = token::get_supply( contract, symcode );
    sx::wallet::balances _balances( get_self(), account.value );
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
void sx::wallet::close( const name account, const name contract, const symbol_code symcode )
{
    require_auth_or_self( account );

    sx::wallet::balances _balances( get_self(), account.value );
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

void sx::wallet::sub_balance( const name account, const name contract, const asset quantity )
{
    sx::wallet::balances _balances( get_self(), account.value );
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

void sx::wallet::add_balance( const name account, const name contract, const asset quantity, const name ram_payer )
{
    sx::wallet::balances _balances( get_self(), account.value );
    const symbol_code symcode = quantity.symbol.code();
    const auto itr = _balances.find( contract.value );

    // create new balance entry
    if ( itr == _balances.end() ) {
        _balances.emplace( ram_payer, [&]( auto& row ) {
            row.contract = contract;
            row.balances[ symcode ] = quantity;
        });
    // modify existing balance entry
    } else {
        // create new field
        if ( !itr->balances.at( symcode ).symbol.code() ) {
            _balances.modify( itr, ram_payer, [&]( auto& row ) {
                row.balances[ symcode ] = quantity;
            });
        // modify existing field
        } else {
            _balances.modify( itr, same_payer, [&]( auto& row ) {
                row.balances[ symcode ] += quantity;
            });
        }
    }
}

void sx::wallet::check_open( const name account, const name contract, const symbol_code symcode )
{
    check( is_account( account ), account.to_string() + " account does not exist");

    // disable checks if authority is self
    if ( has_auth( get_self() )) return;

    // make sure receiver has open balance
    token::accounts _accounts( contract, account.value );
    auto itr = _accounts.find( symcode.raw() );
    check( itr != _accounts.end(), account.to_string() + " account must have " + symcode.to_string() + " `open` balance in " + contract.to_string());
}

void sx::wallet::check_open_internal( const name account, const name contract, const symbol_code symcode )
{
    check( is_account( account ), account.to_string() + " account does not exist");

    // disable checks if authority is self
    if ( has_auth( get_self() )) return;

    // make sure receiver has open balance internally
    sx::wallet::balances _balances( get_self(), account.value );
    const string message = account.to_string() + " account must have " + symcode.to_string() + " `open` balance in " + get_self().to_string();
    const auto itr = _balances.find( contract.value );
    check( itr != _balances.end(), message );
    const asset balance = itr->balances.at( symcode );
    check( balance.symbol.code().raw(), message );
}

void sx::wallet::require_auth_or_self( const name account )
{
    if ( has_auth( get_self() ) ) return;
    require_auth( account );
}
