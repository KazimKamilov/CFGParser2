#ifndef _CFG_PARSER_HPP_
#define _CFG_PARSER_HPP_

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>


/**
    \brief Config parser class.
    Config file syntax:
   >section<       >inheritance<              >attibutes<
    [name] : base_struct0, base_struct1 = attribute0, attribute1
    key = value
    string = "Some of something"
    multistr = "You can use\n
         multiline \n
        string too!"
    ;this is comment line!
    |And this is comment block
    which you can use as multiline|
*/
class CFGParser final
{
public:

    using ValueHash = std::unordered_map<std::string, std::string>;

    struct Section final
    {
        std::vector<std::string> inheritances;
        std::vector<std::string> attributes;
        ValueHash values;
    };

    using SectionDataHash = std::unordered_map<std::string, Section>;

private:
    SectionDataHash _section_data;

    std::function<void(const std::string&)> _msg_functor;

    static constexpr char comment_character {';'};
    static constexpr char comment_multiline {'|'};

    std::string _current_file {};
    std::string _base_path {};

    // Some dummies for return unexistance things
    const std::vector<std::string> _dummy {};
    const std::string _dummy_str {};

public:
    CFGParser() noexcept;
    CFGParser(const std::string& file_path) noexcept;
    ~CFGParser() noexcept = default;

    // We cannot copy config file or move it. Sorry)
    CFGParser(CFGParser const&) noexcept = delete;
    CFGParser(CFGParser const&&) noexcept = delete;
    CFGParser const& operator=(CFGParser const&) noexcept = delete;
    CFGParser const& operator=(CFGParser const&&) noexcept = delete;

    /**
        \brief Sets include base path.
    */
    void setBasePath(const std::string& path) { _base_path = path; }

    /**
        \brief You can use your own message functor.
    */
    template<typename F> void setMessageFunctor(F&& func) { _msg_functor = std::move(func); }

    /**
        \brief Load and parse config file.
    */
    void load(const std::string& file_path);
    void save(const std::string& file_path);
    void saveCurrent() { this->save(_current_file); }

    /**
        \brief Checking that the section have some attributes(string flags).
    */
    const bool hasAttribute(const std::string& section, const std::string& attribute) const noexcept;
    const bool hasAttributes(const std::string& section) const noexcept;
    const std::vector<std::string>& getAttributes(const std::string& section) const noexcept;

    /**
        \brief Checking is section exists.
    */
    const bool hasSection(const std::string& section) const noexcept;

    /**
        \brief Checking is key exist inside a section.
    */
    const bool hasKey(const std::string& section, const std::string& key) const noexcept;

    /**
        \brief Checking is section has inheritance from some other section.
    */
    const bool isInheritedFrom(const std::string& section, const std::string& base_section) const noexcept;
    const bool hasInheritances(const std::string& section) const noexcept;
    const std::vector<std::string>& getInheritances(const std::string& section) const noexcept;

    /**
        \brief Get string from config file.
    */
    const std::string& getString(const std::string& section, const std::string& key, const std::string& default_value = {}) const noexcept;

    /**
        \brief Parse value to desired type. Important! Do not set type as string!
    */
    template<typename T>
    inline const T get(const std::string& section, const std::string& key, const T& default_value = static_cast<T>(0)) const noexcept
    {
        const auto& str = this->getString(section, key);

        if (!str.empty())
            return makeValueFromString<T>(str);
        else
            return default_value;
    }

    template<typename T>
    inline void set(const std::string& section, const std::string& key, const T value) noexcept
    {
        if (const auto iter = _section_data.find(section);
            iter != _section_data.cend())
        {
            if (iter->second.values.find(key) != iter->second.values.cend())
            {
                iter->second.values[key] = std::to_string(value);
            }
            else
            {
                if (_msg_functor)
                    _msg_functor("Section \"" + section + "\" key \"" + key + "\" is not exist!");
            }
        }
        else
        {
            if (_msg_functor)
                _msg_functor("Section \"" + section + "\" is not exist!");
        }
    }

    /**
        \brief Get array value.
    */
    template<typename T>
    inline const std::vector<T> getArray(const std::string& section, const std::string& key) const noexcept
    {
        const auto& str = this->getString(section, key);

        if (!str.empty())
        {
            std::vector<T> result;
            std::string num_str;
            const size_t str_len = str.length();

            for (const auto character :  str)
            {
                if (character == ',')
                {
                    result.push_back(makeValueFromString<T>(num_str));
                    num_str.clear();
                }
                else
                {
                    num_str += character;
                }
            }

            // push last element
            result.push_back(makeValueFromString<T>(num_str));

            return result;
        }
        else
        {
            return {};
        }
    }

    /**
        \brief Returns section number.
    */
    const size_t getSectionCount() const noexcept { return _section_data.size(); }

    /**
        \brief Returns all cfg data reference.
    */
    const SectionDataHash& getSectionData() const noexcept { return _section_data; }

private:
    template<typename T>
    inline constexpr T makeValueFromString(const std::string& string_value) const
    {
        T value = static_cast<T>(0);

        if constexpr (std::is_same<T, bool>::value)
        {
            if (string_value == "true" || string_value == "on" || string_value == "yes")
                value = true;
        }
        else if constexpr (std::is_same<T, int>::value)
        {
            value = std::stoi(string_value);
        }
        else if constexpr (std::is_same<T, uint32_t>::value)
        {
            value = std::stoul(string_value);
        }
        else if constexpr (std::is_same<T, float>::value)
        {
            value = std::stof(string_value);
        }
        else if constexpr (std::is_same<T, double>::value)
        {
            value = std::stod(string_value);
        }
        else if constexpr (std::is_same<T, long double>::value)
        {
            value = std::stold(string_value);
        }
        else if constexpr (std::is_same<T, int64_t>::value)
        {
            value = std::stoll(string_value);
        }
        else if constexpr (std::is_same<T, uint64_t>::value)
        {
            value = std::stoull(string_value);
        }
        else
        {
            value = static_cast<T>(std::stoi(string_value));
        }

        return value;
    }

    const std::string& getValueFromInheritance(const Section& section_data, const std::string& key) const noexcept;

};

#endif
