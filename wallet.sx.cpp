#include <eosio.token/eosio.token.hpp>
#include "wallet.sx.hpp"

[[eosio::on_notify("*::transfer")]]
void walletSx::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    require_auth( from );

    // ignore no-incoming transfers
    if ( to != get_self() ) return;

    // add balance from sender
    const name contract = get_first_receiver();
    add_balance( from, contract, quantity, get_self() );

    // deposit log (notification purposes only)
    walletSx::deposit_action deposit( get_self(), { get_self(), "active"_n });
    deposit.send( from, contract, quantity );
}

[[eosio::action]]
void walletSx::withdraw( const name account, const name contract, const asset quantity )
{
    walletSx::transfer( account, account, contract, quantity, "withdraw" );
}

[[eosio::action]]
void walletSx::transfer( const name from, const name to, const name contract, const asset quantity, const optional<string> memo )
{
    // authority `from` or contract itself
    if ( !has_auth( get_self() ) ) require_auth( from );

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

void walletSx::sub_balance( const name account, const name contract, const asset quantity )
{
    walletSx::balances _balances( get_self(), contract.value );
    const symbol_code symcode = quantity.symbol.code();
    const auto & itr = _balances.get( account.value, "no account balance found" );
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
    walletSx::balances _balances( get_self(), contract.value );
    const symbol_code symcode = quantity.symbol.code();
    const auto itr = _balances.find( account.value);

    // create new balance entry
    if ( itr == _balances.end() ) {
        _balances.emplace( ram_payer, [&]( auto& row ) {
            row.account = account;
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
    token::accounts _accounts( contract, account.value );
    auto itr = _accounts.find( symcode.raw() );
    check( itr != _accounts.end(), account.to_string() + " account must have " + symcode.to_string() + " `open` balance" );
}