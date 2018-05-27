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
};  // Users

EOSIO_ABI(Users, (create)(remove)(setemail)(setname)(setdob)(getuser))

}  // namespace lumeos
