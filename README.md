# SX `Flashloans` ⚡️ - EOSIO Smart Contract

> Borrow any amount of liquidity instantly for near-zero fees & no collateral.

## Table of Content

- [Concept](#concept)
- [Examples](#examples)
- [ACTION `borrow`](#action-borrow)
- [ACTION `callback`](#action-callback)
- [STATIC `calculate_fee`](#static-calculate_fee)
- [TABLE `settings`](#action-settings)
- [TABLE `state`](#action-state)

## Concept

1. account calls `borrow` action from `flash.sx` with a desired `quantity`.
    - contract gets **initial** balance of asset.
    - contract sends quantity `to` account.
2. account recieves notifications via `on_notify` and/or `callback` of incoming transfer.
    - account is free to use received quantity for any purposes.
    - account returns loan back to contract.
3. contract gets **final** balance of asset
    - contract throws an error if **initial** balance is lower than **final** balance.

🙏 Inspired by [EOS Titan](http://eostitan.com) **[flashloans](https://github.com/CryptoMechanics/flashloans)** smart contract design.

## Examples

- [**basic.cpp**](/examples/basic.cpp)
- [**callback.cpp**](/examples/callback.cpp)

## ACTION `borrow`

Account requests to borrow quantity

- **authority**: `any`

### params

- `{name} receiver` - receiver of flash loan
- `{extended_asset} amount` - flash loan request amount
- `{string} [memo=""]` - (optional) transfer memo
- `{name} [notifier=""]` - (optional) notify accounts after transfer has been sent

### Example 1

```c++
const asset quantity = asset{10000, symbol{"EOS", 4}};
const name contract = "eosio.token"_n;

sx::flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
borrow.send( get_self(), extended_asset{quantity, contract}, "my memo", "notifyme" );
```

### Example 2

```bash
$ cleos push action flash.sx borrow '["myaccount", ["1.0000 EOS", "eosio.token"], "my memo", "notifyme"]' -p myaccount
```

## ACTION `callback`

Notifies recipient account via `callback` action after transfer has been sent from `borrow` action

- **authority**: `get_self()`

### params

- `{name} code` - flash loan contract
- `{name} receiver` - receiver of flash loan
- `{extended_asset} amount` - flash loan request amount
- `{asset} fee` - flash loan fee
- `{string} [memo=""]` - used for outgoing transfer
- `{name} [notifier=""]` - callback notifier account

### Example

```c++
[[eosio::on_notify("flash.sx::callback")]]
void callback( const name code, const name receiver, const extended_asset amount, const asset fee, const string memo, const name notifier )
{
    eosio::token::transfer_action transfer( amount.contract, { receiver, "active"_n });
    transfer.send( receiver, code, amount.quantity + fee, memo );
}
```

## STATIC `calculate_fee`

Calculate processing fee

### params

- `{name} code` - flash loan code
- `{asset} quantity` - quantity input

### example

```c++
const asset quantity = asset{10000, symbol{"EOS", 4}}; // 1.0000 EOS
const asset fee = sx::flash::calculate_fee( "flash.sx"_n, quantity );
// => 0.0002 EOS
```

## TABLE `settings`

- `{uint8_t} fee` - processing fee (bips 1/100 of 1%)

### example

```json
{
    "fee": 2
}
```

## TABLE `state`

- `{name} receiver` - receiver of flash loan
- `{name} contract` - contract name
- `{asset} balance` - balance amount

### example

```json
{
    "receiver": "myaccount",
    "contract": "eosio.token",
    "balance": "1.0000 EOS"
}
```
