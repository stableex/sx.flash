#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("flash.sx")]] flash : public contract {

public:
    using contract::contract;

    /**
     * ## ACTION `borrow`
     *
     * Account requests to borrow quantity
     *
     * - **authority**: `any`
     *
     * ### params
     *
     * - `{name} to` - receiver of flash loan
     * - `{name} contract` - token contract account
     * - `{asset} quantity` - flash loan request amount
     * - `{string} [memo=""]` - (optional) transfer memo
     * - `{name} [notifier=""]` - (optional) notify accounts after transfer has been sent
     *
     * ### Example 1
     *
     * ```c++
     * const asset quantity = asset{10000, symbol{"EOS", 4}};
     * flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
     * borrow.send( get_self(), "eosio.token"_n, quantity, "my memo", "notifyme" );
     * ```
     *
     * ### Example 2
     *
     * ```bash
     * $ cleos push action flash.sx borrow '["myaccount", "eosio.token", "1.0000 EOS", "my memo", "notifyme"]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void borrow( const name to, const name contract, const asset quantity, const optional<string> memo, const optional<name> notifier );

    /**
     * ## ACTION `checkbalance`
     *
     * Throws error if account does not have equal or above desired asset quantity
     *
     * - **authority**: `any`
     *
     * ### params
     *
     * - `{name} account` - account to check if minimum balance is available
     * - `{name} contract` - token contract account
     * - `{asset} quantity` - minimum required balance amount
     *
     * ### returns
     *
     * - `{asset}` - current balance of account
     *
     * ### Example 1
     *
     * ```c++
     * const name account = "myaccount"_n;
     * const name contract = "eosio.token"_n;
     * const asset quantity = asset{10000, symbol{"EOS", 4}};
     *
     * flash::checkbalance_action checkbalance( "flash.sx"_n, { account, "active"_n });
     * checkbalance.send( account, contract, quantity );
     * ```
     *
     * ### Example 2
     *
     * ```bash
     * $ cleos push action flash.sx checkbalance '["myaccount", "eosio.token", "1.0000 EOS"] -p myaccount
     * ```
     */
    [[eosio::action]]
    asset checkbalance( const name account, const name contract, const asset quantity );

    // action wrappers
    using borrow_action = eosio::action_wrapper<"borrow"_n, &flash::borrow>;
    using checkbalance_action = eosio::action_wrapper<"checkbalance"_n, &flash::checkbalance>;

private:
    void check_open_balance( const name contract, const name owner, const symbol_code symcode );
};
