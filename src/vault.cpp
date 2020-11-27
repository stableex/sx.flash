#include <eosio.token/eosio.token.hpp>

/**
 * Notify contract when any token transfer notifiers relay contract
 */
[[eosio::on_notify("*::transfer")]]
void sx::flash::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // ignore outgoing transfers
    if ( to != get_self() ) return;

    // ignore funding while flash loan is active
    state_table _state( get_self(), get_self().value );
    if ( _state.exists() ) return;

    // incoming token contract
    const name contract = get_first_receiver();

    // table & index
    sx::flash::vault_table _vault( get_self(), get_self().value );
    auto _vault_by_supply = _vault.get_index<"bysupply"_n>();

    // iterators
    auto balance_itr = _vault.find( quantity.symbol.code().raw() );
    auto supply_itr = _vault_by_supply.find( quantity.symbol.code().raw() );

    // handle issuance (ex: EOS => SXEOS)
    if ( balance_itr != _vault.end() ) {
        // calculate issuance supply token by providing balance
        check( contract == balance_itr->balance.contract, "balance contract does not match" );
        const symbol_code id = balance_itr->balance.quantity.symbol.code();
        const extended_asset out = calculate_issue( id, quantity );

        // issue & transfer to sender
        issue( out, "issue" );
        transfer( get_self(), from, out, get_self().to_string() );

        // update internal balance & supply
        _vault.modify( balance_itr, get_self(), [&]( auto& row ) {
            row.balance.quantity += quantity;
            row.supply += out;
            row.last_updated = current_time_point();
        });

    // handle retire (ex: SXEOS => EOS)
    } else if ( supply_itr != _vault_by_supply.end() ) {
        // calculate redeem balance from retiring supply token
        check( contract == supply_itr->supply.contract, "supply contract does not match" );
        const symbol_code id = supply_itr->balance.quantity.symbol.code();
        const extended_asset out = calculate_retire( id, quantity );

        // retire & transfer to sender
        retire( { quantity, contract }, "retire" );
        transfer( get_self(), from, out, get_self().to_string() );

        // update internal balance & supply
        _vault_by_supply.modify( supply_itr, get_self(), [&]( auto& row ) {
            row.balance -= out;
            row.supply.quantity -= quantity;
            row.last_updated = current_time_point();
        });
    } else {
        check( false, "incoming transfer asset symbol not supported");
    }
}

[[eosio::action]]
void sx::flash::setvault( const extended_symbol deposit, const symbol_code id )
{
    require_auth( get_self() );
    sx::flash::vault_table _vault( get_self(), get_self().value );

    // ID must use same symbol precision as deposit
    const extended_symbol ext_id = {{ id, deposit.get_symbol().precision() }, TOKEN_CONTRACT };

    // deposit token must exists
    const asset supply = eosio::token::get_supply( deposit.get_contract(), deposit.get_symbol().code() );
    check( supply.amount > 0, "deposit has no supply");
    check( deposit.get_symbol() == supply.symbol, "symbol precision mismatch");

    // must have open balance
    // initializing vault must not contain any deposit balance
    const asset balance = eosio::token::get_balance( deposit.get_contract(), get_self(), deposit.get_symbol().code() );
    check( balance.amount <= 0, "must have zero balance");
    check( _vault.find( id.raw() ) == _vault.end(), "vault already created");

    // create vault
    create( ext_id );
    _vault.emplace( get_self(), [&]( auto& row ) {
        row.balance = { 0, deposit };
        row.supply = { 0, ext_id };
        row.last_updated = current_time_point();
    });
}

void sx::flash::set_balance( const extended_asset value )
{
    sx::flash::vault_table _vault( get_self(), get_self().value );
    const auto& vault = _vault.get( value.quantity.symbol.code().raw(), "value does not exists in vault" );

    _vault.modify( vault, get_self(), [&]( auto& row ) {
        row.balance = value;
        row.last_updated = current_time_point();
    });
}

extended_asset sx::flash::calculate_issue( const symbol_code id, const asset payment )
{
    sx::flash::vault_table _vault( get_self(), get_self().value );
    const auto vault = _vault.get( id.raw(), "id does not exists in vault" );
    const int64_t ratio = 10000;

    // initialize vault supply
    if ( vault.supply.quantity.amount == 0 ) return { payment.amount * ratio, vault.supply.get_extended_symbol() };

    // issue & redeem supply calculation
    // calculations based on fill REX order
    // https://github.com/EOSIO/eosio.contracts/blob/f6578c45c83ec60826e6a1eeb9ee71de85abe976/contracts/eosio.system/src/rex.cpp#L775-L779
    const int64_t S0 = vault.balance.quantity.amount; // vault
    const int64_t S1 = S0 + payment.amount; // payment
    const int64_t R0 = vault.supply.quantity.amount; // supply
    const int64_t R1 = (uint128_t(S1) * R0) / S0;

    return { R1 - R0, vault.supply.get_extended_symbol() };
}

extended_asset sx::flash::calculate_retire( const symbol_code id, const asset payment )
{
    sx::flash::vault_table _vault( get_self(), get_self().value );
    const auto vault = _vault.get( id.raw(), "id does not exists in vault" );

    // issue & redeem supply calculation
    // calculations based on add to REX pool
    // https://github.com/EOSIO/eosio.contracts/blob/f6578c45c83ec60826e6a1eeb9ee71de85abe976/contracts/eosio.system/src/rex.cpp#L772
    const int64_t S0 = vault.balance.quantity.amount;
    const int64_t R0 = vault.supply.quantity.amount;
    const int64_t p  = (uint128_t(payment.amount) * S0) / R0;

    return { p, vault.balance.get_extended_symbol() };
}
