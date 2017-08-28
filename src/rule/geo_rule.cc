// MIT License

// Copyright (c) 2017 Zhuhao Wang

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "nekit/rule/geo_rule.h"

namespace nekit {
namespace rule {

namespace {
const std::string CountryIsoCodeCacheKey = "NECO";
}

GeoRule::GeoRule(
    utils::CountryIsoCode code, bool match,
    std::shared_ptr<transport::AdapterFactoryInterface> adapter_factory)
    : code_{code}, match_{match}, adapter_factory_{adapter_factory} {}

MatchResult GeoRule::Match(std::shared_ptr<utils::Session> session) {
  utils::CountryIsoCode code;

  auto iter = session->int_cache().find(CountryIsoCodeCacheKey);
  if (iter != session->int_cache().end()) {
    code = static_cast<utils::CountryIsoCode>(iter->second);
  } else {
    if (session->type() == utils::Session::Type::Address) {
      code = LookupAndCache(session, session->address());
    } else {
      if (session->domain()->isFailed()) {
        return MatchResult::NotMatch;
      }
      if (!session->domain()->isResolved()) {
        return MatchResult::ResolveNeeded;
      }
      code = LookupAndCache(session, session->domain()->addresses()->front());
    }
  }

  if ((code == code_) == match_) {
    return MatchResult::Match;
  } else {
    return MatchResult::NotMatch;
  }
}

std::unique_ptr<transport::AdapterInterface> GeoRule::GetAdapter(
    std::shared_ptr<utils::Session> session) {
  return adapter_factory_->Build(session);
}

utils::CountryIsoCode GeoRule::LookupAndCache(
    std::shared_ptr<utils::Session> session,
    const boost::asio::ip::address &address) {
  auto result = utils::Maxmind::Lookup(address);
  assert(!result.error());
  session->int_cache()[CountryIsoCodeCacheKey] =
      static_cast<int>(result.country_iso_code());
  return result.country_iso_code();
}
}  // namespace rule
}  // namespace nekit