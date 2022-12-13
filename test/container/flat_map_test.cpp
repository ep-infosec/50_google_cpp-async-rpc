/// \file
/// \brief Test for `flat_map` class.
///
/// \copyright
///   Copyright 2019 by Google LLC.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.

#include "arpc/container/flat_map.h"
#include <utility>
#include <vector>
#include "catch2/catch.hpp"

TEST_CASE("flat_map tests") {
  using map = arpc::flat_map<int, int>;
  using vector = std::vector<std::pair<int, int>>;

  map s1;
  map s2{{4, 1}, {3, 2}};
  map s3{{3, 2}, {4, 1}, {3, 3}};
  const map s4{{3, 2}, {4, 1}, {3, 3}};

  SECTION("empty true for empty map") { REQUIRE(s1.empty()); }

  SECTION("empty false for non-empty map") { REQUIRE_FALSE(s2.empty()); }

  SECTION("size returns the number of elements") {
    REQUIRE(s1.size() == 0);
    REQUIRE(s2.size() == 2);
    REQUIRE(s3.size() == 2);
  }

  SECTION("items returned in the expected order") {
    REQUIRE(vector(s1.begin(), s1.end()) == (vector{}));
    REQUIRE(vector(s2.begin(), s2.end()) == (vector{{3, 2}, {4, 1}}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{{3, 2}, {4, 1}}));
  }

  SECTION("find missing item returns end()") {
    REQUIRE(s1.find(1) == s1.end());
    REQUIRE(s2.find(1) == s2.end());
    REQUIRE(s3.find(1) == s3.end());
    REQUIRE(s4.find(1) == s4.end());
  }

  SECTION("find first item returns begin()") {
    REQUIRE(s2.find(3) == s2.begin());
    REQUIRE(s3.find(3) == s3.begin());
    REQUIRE(s4.find(3) == s4.begin());
  }

  SECTION("find existing item returns non-end and can be dereferenced") {
    REQUIRE(s2.find(4) != s2.end());
    REQUIRE(s3.find(4) != s3.end());
    REQUIRE(s4.find(4) != s4.end());

    REQUIRE(*(s2.find(4)) == std::make_pair(4, 1));
    REQUIRE(*(s3.find(4)) == std::make_pair(4, 1));
    REQUIRE(*(s4.find(4)) == std::make_pair(4, 1));
  }

  SECTION("inserting individual items works") {
    REQUIRE(s1.insert(std::make_pair(3, 3)).second);
    REQUIRE_FALSE(s2.insert(std::make_pair(4, 5)).second);
    auto p = std::make_pair(4, 5);
    REQUIRE_FALSE(s2.insert(p).second);
    REQUIRE(s3.insert(std::make_pair(5, 6)).second);

    REQUIRE(vector(s1.begin(), s1.end()) == (vector{{3, 3}}));
    REQUIRE(vector(s2.begin(), s2.end()) == (vector{{3, 2}, {4, 1}}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{{3, 2}, {4, 1}, {5, 6}}));
  }

  SECTION("emplacing individual items works") {
    REQUIRE(s1.emplace(3, 3).second);
    REQUIRE_FALSE(s2.emplace(4, 5).second);
    REQUIRE(s3.emplace(5, 6).second);

    REQUIRE(vector(s1.begin(), s1.end()) == (vector{{3, 3}}));
    REQUIRE(vector(s2.begin(), s2.end()) == (vector{{3, 2}, {4, 1}}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{{3, 2}, {4, 1}, {5, 6}}));
  }

  SECTION("inserting individual items with hints works") {
    s1.insert(s1.begin(), std::make_pair(3, 3));
    s2.insert(s2.end(), std::make_pair(4, 5));
    s3.insert(s3.begin(), std::make_pair(5, 6));

    REQUIRE(vector(s1.begin(), s1.end()) == (vector{{3, 3}}));
    REQUIRE(vector(s2.begin(), s2.end()) == (vector{{3, 2}, {4, 1}}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{{3, 2}, {4, 1}, {5, 6}}));
  }

  SECTION("inserting a range works") {
    s1.insert(s2.begin(), s2.end());

    REQUIRE(vector(s1.begin(), s1.end()) == (vector{{3, 2}, {4, 1}}));
  }

  SECTION("erasing individual values works") {
    REQUIRE(s1.erase(3) == 0);
    REQUIRE(s2.erase(4) == 1);
    REQUIRE(s3.erase(5) == 0);

    REQUIRE(vector(s1.begin(), s1.end()) == (vector{}));
    REQUIRE(vector(s2.begin(), s2.end()) == (vector{{3, 2}}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{{3, 2}, {4, 1}}));
  }

  SECTION("erasing individual items with iterators works") {
    s2.erase(s2.begin());
    s3.erase(s3.begin() + 1);

    REQUIRE(vector(s2.begin(), s2.end()) == (vector{{4, 1}}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{{3, 2}}));
  }

  SECTION("erasing a range works") {
    s2.erase(s2.begin(), s2.end());
    s3.erase(s3.begin(), s3.begin() + 1);

    REQUIRE(vector(s2.begin(), s2.end()) == (vector{}));
    REQUIRE(vector(s3.begin(), s3.end()) == (vector{{4, 1}}));
  }

  SECTION("lower_bound") {
    REQUIRE(s1.lower_bound(3) == s1.end());
    REQUIRE(s3.lower_bound(3) == s3.begin());
    REQUIRE(s3.lower_bound(4) == s3.begin() + 1);
    REQUIRE(s3.lower_bound(5) == s3.end());
    REQUIRE(s4.lower_bound(4) == s4.begin() + 1);
    REQUIRE(s4.lower_bound(5) == s4.end());
  }

  SECTION("upper_bound") {
    REQUIRE(s1.upper_bound(3) == s1.end());
    REQUIRE(s3.upper_bound(3) == s3.begin() + 1);
    REQUIRE(s3.upper_bound(4) == s3.end());
    REQUIRE(s4.upper_bound(3) == s4.begin() + 1);
    REQUIRE(s4.upper_bound(4) == s4.end());
  }

  SECTION("equal_range") {
    REQUIRE(s1.equal_range(3) == std::make_pair(s1.end(), s1.end()));
    REQUIRE(s3.equal_range(3) == std::make_pair(s3.begin(), s3.begin() + 1));
    REQUIRE(s3.equal_range(4) == std::make_pair(s3.begin() + 1, s3.end()));
    REQUIRE(s4.equal_range(3) == std::make_pair(s4.begin(), s4.begin() + 1));
    REQUIRE(s4.equal_range(4) == std::make_pair(s4.begin() + 1, s4.end()));
  }

  SECTION("count") {
    REQUIRE(s1.count(3) == 0);
    REQUIRE(s2.count(3) == 1);
    REQUIRE(s3.count(3) == 1);
  }

  SECTION("operator []") {
    REQUIRE(s1[3] == 0);
    REQUIRE(s2[3] == 2);
    REQUIRE(s3[4] == 1);
  }
}
