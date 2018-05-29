//
//  Copyright (c) 2018, Respective Authors all rights reserved.
//
//  The MIT License
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to
//  deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//  sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//  IN THE SOFTWARE.
//

#include <users.hpp>

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

namespace lumeos {

// TODO: Add validations etc. cants use most of the boost here

struct Users : public eosio::contract {
    using userIndex = eosio::multi_index<N(user), user>;

   public:
    Users(account_name self) : contract(self) {}

    //@abi action
    void create(
        eosio::name const accountName, std::string const& name,
        std::string const& email,
        std::string const& addressStr,  // "street:city:country:postal_code"
        uint32_t dateOfBirth) {
        require_auth(accountName);

        userIndex users(_self, _self);
        eosio_assert(users.find(accountName) == users.end(),
                     "User already exists.");

        users.emplace(accountName, [&](auto& user) {
            user.m_accountName = accountName;
            user.m_name = name;
            user.m_email = email;
            user.m_dateOfBirth = dateOfBirth;
            user.m_address = address(addressStr);
        });
    }

    //@abi action
    void remove(eosio::name const accountName, std::string const& feedback) {
        // feedback for us to know why they deleting their account
        require_auth(accountName);
        userIndex users(_self, _self);
        auto accountItr = users.find(accountName);
        eosio_assert(accountItr != users.end(), "User not found.");

        users.erase(accountItr);
        eosio::print(std::string("erased: " + accountName.to_string() +
                                 " with reason: " + feedback)
                         .c_str());
    }

    // @abi action
    void setemail(eosio::name const accountName, std::string const& email) {
        // email validation should be done on front end,
        // i am restricted what I can use here.
        // but maybe some basic checks to make sure?
        require_auth(accountName);

        userIndex users(_self, _self);

        auto accountItr = users.find(accountName);
        eosio_assert(accountItr != users.end(), "User not found.");

        users.modify(accountItr, accountName,
                     [&](auto& user) { user.m_email = email; });
    }

    // @abi action
    void setname(eosio::name const accountName, std::string const& name) {
        require_auth(accountName);

        userIndex users(_self, _self);

        auto accountItr = users.find(accountName);
        eosio_assert(
            accountItr != users.end(),
            std::string("User not found: " + accountName.to_string()).c_str());

        users.modify(accountItr, accountName,
                     [&](auto& user) { user.m_name = name; });
    }

    // @abi action
    void setdob(eosio::name const accountName, uint32_t YYYYMMDD) {
        require_auth(accountName);

        userIndex users(_self, _self);

        auto accountItr = users.find(accountName);
        eosio_assert(
            accountItr != users.end(),
            std::string("User not found: " + accountName.to_string()).c_str());

        users.modify(accountItr, accountName,
                     [&](auto& user) { user.m_dateOfBirth = YYYYMMDD; });
    }

    // @abi action
    void getuser(eosio::name const accountName) {
        userIndex users(_self, _self);

        eosio_assert(users.find(accountName) != users.end(), "User not found");
        auto const& currentUser = users.get(accountName);
        eosio::print(static_cast<std::string>(currentUser).c_str());
    }

    auto getUserItr(eosio::name const accountName, userIndex const& users) {
        auto result = users.find(accountName);
        eosio_assert(
            result != users.end(),
            std::string("User not found: " + accountName.to_string()).c_str());

        return result;
    }

    struct friendship {
        eosio::name m_accountName;
        std::set<eosio::name> m_friends;

        operator std::string() const {
            return m_accountName.to_string() + " has " + std::to_string(m_friends.size()) + " friends.";
        }

        uint64_t primary_key() const { return m_accountName; }

        EOSLIB_SERIALIZE(friendship, (m_accountName)(m_friends))
    };

    using FriendsList = eosio::multi_index<N(friendship), friendship>;

    void validateUser(eosio::name const accountName) {
        userIndex users(_self, _self);
        eosio_assert(
            users.find(accountName) != users.end(),
            std::string("User not found: " + accountName.to_string()).c_str());
    }

    void makeFriends(eosio::name const firstAccountName,
                     eosio::name const secondAccountName) {
       FriendsList friendList( _self, _self);
       auto mainItr = friendList.find( firstAccountName );
       if( mainItr == friendList.end() ) {
          friendList.emplace(_self, [&]( auto& person ){
            person.m_friends.insert(secondAccountName);
          });
       } else {
          friendList.modify(mainItr, 0, [&]( auto& person ) {
            person.m_friends.insert(secondAccountName);
          });
       }
    }

    // @abi action
    void updateflist(eosio::name const firstAccountName,
                      eosio::name const secondAccountName, bool becomingFriends) {
        eosio_assert( firstAccountName != secondAccountName, "Cannot self friend" );
        require_auth( _self );

        validateUser(firstAccountName);
        validateUser(secondAccountName);

        if (becomingFriends) {
            eosio::print("make friends");
            makeFriends(firstAccountName, secondAccountName);
        } else {
            eosio::print("unfriend");
            // unfriend(firstAccountName, secondAccountName)
        }
    }
};  // Users

EOSIO_ABI(Users, (create)(remove)(setemail)(setname)(setdob)(getuser)(updateflist))

}  // namespace lumeos
