# SX `Flashloans` ⚡️ - EOSIO Smart Contract

🙏Inspired by [EOS Titan](http://eostitan.com) **[flashloans](https://github.com/CryptoMechanics/flashloans)** smart contract design.

> Borrow any amount of liquidity instantly for zero fees & no collateral.

## Table of Content

- [Concept](#concept)
- [Examples](#examples)
- [Build](#build)
- [ACTION `borrow`](#action-borrow)
- [ACTION `checkbalance`](#action-checkbalance)
- [ACTION `callback`](#action-callback)

## Concept

1. account calls `borrow` action from `flash.sx` with a desired `quantity`.
    - contract gets **initial** balance of asset.
    - contract sends quantity `to` account.
2. account recieves notifications via `on_notify` and/or `callback` of incoming transfer.
    - account is free to use received quantity for any purposes.
    - account returns loan back to contract.
3. contract gets **final** balance of asset
    - contract throws an error if **initial** balance is lower than **final** balance.

## Examples

- [**basic.cpp**](/examples/basic.sx.cpp) ([example transaction](https://eosq.app/tx/6cf07056153122ea994770ff1205adf10b23597cdeee9f85d34e844e4ede8f4e))
- [**callback.cpp**](/examples/callback.sx.cpp) ([example transaction](https://eosq.app/tx/8c4e0186ed34af344d30b89b55f3e2a93fdef26fde9f9675e33e3ce0fbaf58cc))

## Build

```bash
$ eosio-cpp flash.sx.cpp -o flash.sx.wasm
```

## ACTION `borrow`

Account requests to borrow quantity

- **authority**: `any`

### params

- `{name} to` - receiver of flash loan
- `{name} contract` - token contract account
- `{asset} quantity` - flash loan request amount
- `{string} [memo=""]` - (optional) transfer memo
- `{name} [notifier=""]` - (optional) notify accounts after transfer has been sent

### Example 1

```c++
const asset quantity = asset{10000, symbol{"EOS", 4}};
flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
borrow.send( get_self(), "eosio.token"_n, quantity, "my memo", "notifyme" );
```

### Example 2

```bash
$ cleos push action flash.sx borrow '["myaccount", "eosio.token", "1.0000 EOS", "my memo", "notifyme"]' -p myaccount
```

## ACTION `savebalance`

Save existing balance of account

- **authority**: `any`

### params

- `{name} account` - account to save balance from
- `{name} contract` - token contract account
- `{symbol_code} symcode` - symbol code

### Example 1

```c++
const name account = "myaccount"_n;
const name contract = "eosio.token"_n;
const symbol_code symcode = symbol_code{"EOS"};

flash::savebalance_action savebalance( "flash.sx"_n, { account, "active"_n });
savebalance.send( account, contract, symcode );
```

### Example 2

```bash
$ cleos push action flash.sx checkbalance '["myaccount", "eosio.token", "EOS"] -p myaccount
```

## ACTION `checkbalance`

Throws error if account does not have equal or above previously saved balance

- **authority**: `any`

### params

- `{name} account` - account to check if minimum balance is available
- `{name} contract` - token contract account
- `{symbol_code} symcode` - symbol code
- `{quantity} [addition=null]` - (optional) additional quantity to saved balance

### Example 1

```c++
const name account = "myaccount"_n;
const name contract = "eosio.token"_n;
const symbol_code symcode = symbol_code{"EOS"};
const asset addition = asset{0, symbol{"EOS", 4}};

flash::checkbalance_action checkbalance( "flash.sx"_n, { account, "active"_n });
checkbalance.send( account, contract, symcode, addition );
```

### Example 2

```bash
$ cleos push action flash.sx checkbalance '["myaccount", "eosio.token", "EOS", "0.0000 EOS"] -p myaccount
```

## ACTION `callback`

Notifies recipient account via `callback` action after transfer has been sent from `borrow` action

- **authority**: `get_self()`

### params

- `{name} to` - receiver of flash loan
- `{name} contract` - token contract account
- `{asset} quantity` - flash loan request amount
- `{string} memo` - used for outgoing transfer
- `{name} recipient` - callback recipient

### Example

```c++
[[eosio::on_notify("flash.sx::callback")]]
void callback( const name to, const name contract, asset quantity, const string memo, const name recipient )
{
    token::transfer_action transfer( contract, { get_self(), "active"_n });
    transfer.send( get_self(), "flash.sx"_n, quantity, memo );
}
```