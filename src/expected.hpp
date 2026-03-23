#ifndef CAESAR_EXPECTED_HPP
#define CAESAR_EXPECTED_HPP

#include <algorithm>
#include <variant>

template <typename E>
struct Unexpected {
  E error;
  explicit Unexpected(E e) : error(std::move(e)) {}
};

Unexpected(const char*) -> Unexpected<std::string>;

template <typename Result, typename Error>
class Expected {
  std::variant<Result, Error> m_data;

 public:
  // NOLINTBEGIN(google-explicit-constructor)
  Expected(const Result& val) : m_data(std::in_place_index<0>, val) {}
  Expected(Result&& val) : m_data(std::in_place_index<0>, std::move(val)) {}
  Expected(Unexpected<Error> err)
      : m_data(std::in_place_index<1>, std::move(err.error)) {}
  // NOLINTEND(google-explicit-constructor)

  [[nodiscard]] bool hasValue() const { return m_data.index() == 0; }
  explicit operator bool() const { return hasValue(); }

  Result& value() { return std::get<0>(m_data); }
  const Result& value() const { return std::get<0>(m_data); }

  Error& error() { return std::get<1>(m_data); }
  const Error& error() const { return std::get<1>(m_data); }

  Result& operator*() { return value(); }
  Result* operator->() { return &value(); }
};

#endif
